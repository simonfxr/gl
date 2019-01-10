#include "glt/ShaderCompiler.hpp"

#include "data/range.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/GLSLPreprocessor.hpp"
#include "glt/utils.hpp"
#include "sys/fs.hpp"
#include "sys/io/Stream.hpp"
#include "sys/measure.hpp"

#include <algorithm>
#include <functional>
#include <queue>
#include <sstream>
#include <stack>
#include <string_view>
#include <utility>
#include <variant>

template<>
struct LogTraits<glt::ShaderCompilerQueue>
{
    static err::LogDestination getDestination(
      const glt::ShaderCompilerQueue &scq)
    {
        return err::LogDestination(err::Info,
                                   const_cast<glt::ShaderCompilerQueue &>(scq)
                                     .shaderCompiler()
                                     .shaderManager()
                                     .out());
    }
};

#define COMPILER_ERR_MSG(comp, ec, msg) LOG_RAISE(comp, ec, ::err::Error, msg)

namespace glt {

namespace {
struct CompileJob
{
    std::shared_ptr<ShaderSource> source;
    std::function<std::shared_ptr<ShaderObject>()> exec;
};
} // namespace

using QueuedJobs = std::unordered_set<ShaderSourceKey>;

using ShaderSourceFilePath = std::string; // absolute path

using CompileJobs = std::queue<CompileJob>;

namespace {
struct StringSource
{
    const std::string source;
};
} // namespace

namespace {
struct FileSource
{
    const std::string path;
};
} // namespace

namespace {
struct StringShaderObject
{
    const StringSource &source;
};
} // namespace

namespace {
struct FileShaderObject
{
    const FileSource &source;
    sys::fs::FileTime mtime;
};
} // namespace

struct ShaderSource::Data
{
    const ShaderSourceKey key;
    const ShaderManager::ShaderType type;
    const std::variant<StringSource, FileSource> source;

    static std::unique_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> &,
      const StringSource &,
      ShaderCompilerQueue &);

    static std::unique_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> &,
      const FileSource &,
      ShaderCompilerQueue &);

    static std::unique_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> &,
      ShaderCompilerQueue &);
};

DECLARE_PIMPL_DEL(ShaderSource)

struct ShaderObject::Data
{
    ShaderObject *self{};
    GLShaderObject handle{};
    ShaderIncludes includes;
    ShaderDependencies dependencies;
    std::weak_ptr<ShaderCache> cache;
    std::shared_ptr<ShaderSource> source;
    std::variant<StringShaderObject, FileShaderObject> object;

    Data(std::shared_ptr<ShaderSource> source_,
         std::variant<StringShaderObject, FileShaderObject> obj)
      : source(std::move(source_)), object(std::move(obj))
    {}

    Data(Data &&) = default;

    ~Data()
    {
        auto c = cache.lock();
        if (c)
            ShaderCache::remove(c, self);
    }

    std::pair<std::unique_ptr<ShaderObject>, ReloadState> reloadIfOutdated(
      ShaderCompilerQueue &);

    void linkCache(const std::shared_ptr<ShaderCache> &);
    void unlinkCache(const std::shared_ptr<ShaderCache> &);

    static std::unique_ptr<ShaderObject> makeStringShaderObject(
      std::shared_ptr<ShaderSource>,
      const StringSource &);

    static std::unique_ptr<ShaderObject> makeFileShaderObject(
      std::shared_ptr<ShaderSource>,
      const FileSource &,
      sys::fs::FileTime mtime);
};

DECLARE_PIMPL_DEL(ShaderObject)

struct ShaderCache::Data
{
    ShaderCacheEntries entries;
    ShaderCache &self;
    void checkValid();
    bool remove(ShaderObject *);

    Data(ShaderCache &self_) : self(self_) {}

    ~Data() { self.flush(); }
};

DECLARE_PIMPL_DEL(ShaderCache)

struct ShaderCompiler::Data
{
    ShaderManager &shaderManager;
    PreprocessorDefinitions defines;
    std::shared_ptr<ShaderCache> cache;

    Data(ShaderManager &sm) : shaderManager(sm) {}
    void initPreprocessor(GLSLPreprocessor & /*proc*/);
};

DECLARE_PIMPL_DEL(ShaderCompiler)

struct ShaderCompilerQueue::Data
{
    ShaderCompilerQueue &self;
    ShaderCompiler &compiler;
    const ShaderCompileFlags flags;
    ShaderObjects &compiled;
    QueuedJobs inQueue;
    CompileJobs toCompile;

    Data(ShaderCompilerQueue &self_,
         ShaderCompiler &comp,
         ShaderObjects &_compiled,
         ShaderCompileFlags flgs)
      : self(self_), compiler(comp), flags(flgs), compiled(_compiled)
    {}

    void put(const std::shared_ptr<ShaderObject> &);
    std::shared_ptr<ShaderObject> reload(const std::shared_ptr<ShaderObject> &);
};

DECLARE_PIMPL_DEL(ShaderCompilerQueue)

namespace {

template<class T>
struct always_false : std::false_type
{};

std::string
hash(const std::string &source)
{
    return "h" + std::to_string(std::hash<std::string>{}(source));
}

struct ShaderTypeMapping
{
    const char *fileExtension;
    ShaderManager::ShaderType type;
    GLenum glType;
};

const ShaderTypeMapping shaderTypeMappings[] = {
    { "frag", ShaderManager::FragmentShader, GL_FRAGMENT_SHADER },
    { "vert", ShaderManager::VertexShader, GL_VERTEX_SHADER },
    { "geom", ShaderManager::GeometryShader, GL_GEOMETRY_SHADER },
    { "tctl", ShaderManager::TesselationControl, GL_TESS_CONTROL_SHADER },
    { "tevl", ShaderManager::TesselationEvaluation, GL_TESS_EVALUATION_SHADER }
};

bool
compilePreprocessed(ShaderCompilerQueue & /*scq*/,
                    GLenum /*shader_type*/,
                    const std::string & /*name*/,
                    GLSLPreprocessor & /*proc*/,
                    GLShaderObject & /*shader*/);

void
printShaderLog(GLShaderObject & /*shader*/, sys::io::OutStream &out);

bool
translateShaderType(ShaderManager::ShaderType type,
                    GLenum *gltype,
                    const std::string &basename = "");

ReloadState
includesNeedReload(const ShaderIncludes & /*incs*/);

ReloadState
includesNeedReload(const ShaderIncludes &incs)
{
    for (const auto &inc : incs) {
        auto mtime = sys::fs::modificationTime(inc.first);
        if (!mtime)
            return ReloadState::Failed;
        if (*mtime != inc.second)
            return ReloadState::Outdated;
    }

    return ReloadState::Uptodate;
}

bool
compilePreprocessed(ShaderCompilerQueue &scq,
                    GLenum shader_type,
                    const std::string &name,
                    GLSLPreprocessor &proc,
                    GLShaderObject &shader)
{

    auto nsegments = GLsizei(proc.segments.size());
    const char **segments = &proc.segments[0];
    const auto *segLengths =
      reinterpret_cast<const GLint *>(&proc.segLengths[0]);

    shader.ensure(shader_type);
    if (!shader.valid()) {
        COMPILER_ERR_MSG(
          scq, ShaderCompilerError::OpenGLError, "couldnt create shader");
        return false;
    }

    LOG_BEGIN(scq, err::Info);
    LOG_PUT(scq,
            std::string("compiling ") +
              (name.empty() ? " <embedded code> " : name) + " ... ");

    if (scq.shaderCompiler().shaderManager().dumpShadersEnabled()) {
        sys::io::OutStream &out = scq.shaderCompiler().shaderManager().out();
        out << sys::io::endl;
        out << "BEGIN SHADER SOURCE[" << *shader << "]" << sys::io::endl;
        for (const auto i : irange(nsegments))
            out << std::string_view{ segments[i], size_t(segLengths[i]) };
        out << sys::io::endl;
        out << "END SHADER SOURCE" << sys::io::endl;
    }

    GL_CALL(glShaderSource, *shader, nsegments, segments, segLengths);
    double wct;
    measure_time(wct, glCompileShader(*shader));
    GL_CHECK_ERRORS();

    GLint success;
    GL_CALL(glGetShaderiv, *shader, GL_COMPILE_STATUS, &success);
    bool ok = success == GL_TRUE;
    LOG_PUT(scq, (ok ? "success" : "failed"))
      << " (" << (wct * 1000) << " ms)" << sys::io::endl;
    LOG_END(scq);

    if ((!ok && LOG_LEVEL(scq, err::Error)) || LOG_LEVEL(scq, err::Info)) {
        LOG_BEGIN(scq, ok ? err::Info : err::Error);
        printShaderLog(shader, LOG_DESTINATION(scq));
        LOG_END(scq);
    }

    if (!ok) {
        scq.pushError(ShaderCompilerError::CompilationFailed);
        shader.release();
    }

    return shader.valid();
}

void
printShaderLog(GLShaderObject &shader, sys::io::OutStream &out)
{
    GLint log_len;
    GL_CALL(glGetShaderiv, *shader, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len > 0) {

        auto log = std::make_unique<GLchar[]>(size_t(log_len));
        GL_CALL(glGetShaderInfoLog, *shader, log_len, nullptr, log.get());

        GLchar *logBegin = log.get();
        while (logBegin < log.get() + log_len - 1 && isspace(*logBegin))
            ++logBegin;

        GLchar *end = log.get() + log_len - 1;
        while (end > logBegin && isspace(end[-1]))
            --end;
        *end = 0;
        const char *log_msg = log.get();

        if (logBegin == end) {
            out << "shader compile log empty" << sys::io::endl;
        } else {
            out << "shader compile log: " << sys::io::endl
                << log_msg << sys::io::endl
                << "end compile log" << sys::io::endl;
        }
    }
}

bool
translateShaderType(ShaderManager::ShaderType type,
                    GLenum *gltype,
                    const std::string &basename)
{

    if (type == ShaderManager::GuessShaderType &&
        !ShaderCompiler::guessShaderType(basename, &type))
        return false;

    for (auto shaderTypeMapping : shaderTypeMappings) {
        if (shaderTypeMapping.type == type) {
            *gltype = shaderTypeMapping.glType;
            return true;
        }
    }

    return false;
}
} // namespace

ShaderSource::ShaderSource(Data &&data) : self(new Data(std::move(data))) {}

const ShaderSourceKey &
ShaderSource::key() const
{
    return self->key;
}

std::shared_ptr<ShaderSource>
ShaderSource::makeFileSource(ShaderManager::ShaderType ty,
                             const std::string &path)
{
    ASSERT(sys::fs::exists(path, sys::fs::File));
    return std::make_shared<ShaderSource>(
      Data{ path, ty, { FileSource{ path } } });
}

std::shared_ptr<ShaderSource>
ShaderSource::makeStringSource(ShaderManager::ShaderType ty,
                               const std::string &source)
{
    return std::make_shared<ShaderSource>(
      Data{ hash(source), ty, { StringSource{ source } } });
}

std::unique_ptr<ShaderObject>
ShaderSource::Data::load(const std::shared_ptr<ShaderSource> &src,
                         const StringSource &strsrc,
                         ShaderCompilerQueue &scq)
{
    GLenum gltype;

    if (!translateShaderType(src->self->type, &gltype)) {
        COMPILER_ERR_MSG(
          scq, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return nullptr;
    }

    auto so = ShaderObject::Data::makeStringShaderObject(src, strsrc);
    GLSLPreprocessor preproc(
      scq.shaderCompiler().shaderManager().shaderDirectories(),
      so->self->includes,
      so->self->dependencies);
    preproc.name("");
    scq.shaderCompiler().self->initPreprocessor(preproc);

    preproc.process(strsrc.source);

    if (preproc.wasError())
        return nullptr;

    if (!compilePreprocessed(scq, gltype, "", preproc, so->self->handle))
        return nullptr;
    return so;
}

std::unique_ptr<ShaderObject>
ShaderSource::Data::load(const std::shared_ptr<ShaderSource> &src,
                         const FileSource &filesrc,
                         ShaderCompilerQueue &scq)
{
    GLenum gltype;

    const auto &path = filesrc.path;
    if (!translateShaderType(src->self->type, &gltype, path)) {
        COMPILER_ERR_MSG(
          scq, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return nullptr;
    }

    auto mtime = sys::fs::modificationTime(filesrc.path);
    if (!mtime) {
        COMPILER_ERR_MSG(
          scq, ShaderCompilerError::FileNotFound, "couldnt query mtime");
        return nullptr;
    }

    auto so = ShaderObject::Data::makeFileShaderObject(src, filesrc, *mtime);

    GLSLPreprocessor preproc(
      scq.shaderCompiler().shaderManager().shaderDirectories(),
      so->self->includes,
      so->self->dependencies);
    scq.shaderCompiler().self->initPreprocessor(preproc);

    preproc.processFileRecursively(path);
    if (preproc.wasError())
        return nullptr;

    if (!compilePreprocessed(
          scq, gltype, so->self->source->key(), preproc, so->self->handle))
        return nullptr;
    return so;
}

std::unique_ptr<ShaderObject>
ShaderSource::Data::load(const std::shared_ptr<ShaderSource> &src,
                         ShaderCompilerQueue &scq)
{
    ASSERT(src);
    return std::visit([&](auto &&arg) { return load(src, arg, scq); },
                      src->self->source);
}

ShaderObject::ShaderObject(Data *d) : self(d) {}

const GLShaderObject &
ShaderObject::handle() const
{
    return self->handle;
}

const std::shared_ptr<ShaderSource> &
ShaderObject::shaderSource()
{
    return self->source;
}

void
ShaderObject::Data::unlinkCache(const std::shared_ptr<ShaderCache> &newcache)
{
    auto curr = cache.lock();
    ASSERT(newcache && curr);
    if (curr == newcache)
        cache.reset();
}

std::pair<std::unique_ptr<ShaderObject>, ReloadState>
ShaderObject::Data::reloadIfOutdated(ShaderCompilerQueue &scq)
{
    auto state = std::visit(
      [this](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, StringShaderObject>) {
              return includesNeedReload(includes);
          } else if constexpr (std::is_same_v<T, FileShaderObject>) {
              auto current_mtime = sys::fs::modificationTime(arg.source.path);
              if (!current_mtime)
                  return ReloadState::Failed;
              if (*current_mtime != arg.mtime || includesNeedReload(includes))
                  return ReloadState::Outdated;
              return ReloadState::Uptodate;
          } else {
              static_assert(always_false<T>::value, "non-exhaustive visitor!");
          }
      },
      object);

    switch (state) {
    case ReloadState::Failed:
    case ReloadState::Uptodate:
        return { nullptr, state };
    case ReloadState::Outdated:
        auto new_so = ShaderSource::Data::load(source, scq);
        if (!new_so)
            return { nullptr, ReloadState::Failed };
        return { std::move(new_so), ReloadState::Outdated };
    }
    CASE_UNREACHABLE;
}

void
ShaderObject::Data::linkCache(const std::shared_ptr<ShaderCache> &newcache)
{
    ASSERT(newcache && cache.expired());
    cache = newcache;
}

std::unique_ptr<ShaderObject>
ShaderObject::Data::makeStringShaderObject(std::shared_ptr<ShaderSource> src,
                                           const StringSource &ssrc)
{
    auto so = std::make_unique<ShaderObject>(
      new Data(std::move(src), { StringShaderObject{ ssrc } }));
    so->self->self = so.get();
    return so;
}

std::unique_ptr<ShaderObject>
ShaderObject::Data::makeFileShaderObject(std::shared_ptr<ShaderSource> src,
                                         const FileSource &fsrc,
                                         sys::fs::FileTime mtime)
{

    ASSERT(sys::fs::exists(fsrc.path, sys::fs::File));
    auto so = std::make_unique<ShaderObject>(
      new Data(std::move(src), { FileShaderObject{ fsrc, mtime } }));
    so->self->self = so.get();
    return so;
}

ShaderCache::ShaderCache() : self(new Data(*this)) {}

const ShaderCacheEntries &
ShaderCache::cacheEntries() const
{
    return self->entries;
}

std::shared_ptr<ShaderObject>
ShaderCache::lookup(const ShaderSourceKey &key)
{
    int best_version = -1;
    std::weak_ptr<ShaderObject> best_ref;

    for (auto itpair = self->entries.equal_range(key);
         itpair.first != itpair.second;
         ++itpair.first) {
        auto &ent = itpair.first->second;
        if (ent.first > best_version) {
            best_version = ent.first;
            best_ref = ent.second;
        }
    }

    std::shared_ptr<ShaderObject> so;
    bool found = best_version >= 0;
    if (found) {
        so = best_ref.lock();
        ASSERT(so);
        ASSERT(key == so->shaderSource()->key());
    }

    self->checkValid();
    return so;
}

bool
ShaderCache::put(const std::shared_ptr<ShaderCache> &cache,
                 const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(cache && so);

    auto curr_cache = so->self->cache.lock();
    ASSERT(!curr_cache || curr_cache == cache);

    const ShaderSourceKey &key = so->shaderSource()->key();
    int next_version = 1;
    bool already_present = false;

    for (auto itpair = cache->self->entries.equal_range(key);
         itpair.first != itpair.second;
         ++itpair.first) {
        const auto &ent = itpair.first->second;

        if (so == ent.second.lock()) {
            already_present = true;
            break;
        }

        if (ent.first >= next_version)
            next_version = ent.first + 1;
    }

    if (!already_present) {
        ASSERT(!curr_cache);
        so->self->cache = cache;
        ShaderCacheEntry ent = std::make_pair(next_version, so);
        cache->self->entries.insert(std::make_pair(key, ent));
    }

    cache->self->checkValid();
    return !already_present;
}

bool
ShaderCache::remove(const std::shared_ptr<ShaderCache> &cache, ShaderObject *so)
{
    return cache->self->remove(so);
}

void
ShaderCache::flush()
{
    // be careful not to invalidate iterators
    while (!self->entries.empty()) {
        auto so = self->entries.begin()->second.second.lock();
        ASSERT(so);
        self->remove(so.get());
    }
}

bool
ShaderCache::Data::remove(ShaderObject *so)
{
    ASSERT(so);
    auto cache = so->self->cache.lock();
    ASSERT(cache.get() == &self);

    auto removed = false;
    const auto &key = so->shaderSource()->key();
    for (auto itpair = entries.equal_range(key);
         itpair.first != itpair.second;) {
        const auto &ent = itpair.first->second;

        auto ref = ent.second.lock();
        if (!ref) {
            itpair.first = entries.erase(itpair.first);
        } else if (ref.get() == so) {
            itpair.first = entries.erase(itpair.first);
            removed = true;
            break;
        } else {
            ++itpair.first;
        }
    }

    checkValid();
    return removed;
}

void
ShaderCache::Data::checkValid()
{
    for (auto &kv : entries) {
        auto so = kv.second.second.lock();
        ASSERT(so);
        ASSERT(so->shaderSource()->key() == kv.first);
    }
}

namespace ShaderCompilerError {

std::string stringError(Type /*unused*/)
{
    // FIXME: implement
    return "{shader compiler error}";
}

} // namespace ShaderCompilerError

ShaderCompiler::ShaderCompiler(ShaderManager &sm) : self(new Data(sm)) {}

ShaderManager &
ShaderCompiler::shaderManager()
{
    return self->shaderManager;
}

const std::shared_ptr<ShaderCache> &
ShaderCompiler::shaderCache()
{
    return self->cache;
}

void
ShaderCompiler::init()
{
    self->cache = self->shaderManager.globalShaderCache();
}

bool
ShaderCompiler::guessShaderType(const std::string &path,
                                ShaderManager::ShaderType *res)
{
    ASSERT(res);

    std::string ext = sys::fs::extension(path);

    for (uint32_t i = 0; i < ARRAY_LENGTH(shaderTypeMappings); ++i) {
        if (ext == shaderTypeMappings[i].fileExtension) {
            *res = shaderTypeMappings[i].type;
            return true;
        }
    }

    return false;
}

void
ShaderCompiler::Data::initPreprocessor(GLSLPreprocessor &proc)
{
    auto &m = shaderManager;
    proc.out(m.out());

    if (m.shaderVersion() != 0) {
        sys::io::ByteStream glversdef;
        auto vers = m.shaderVersion();
        glversdef << "#version " << vers;
        if (vers > 150)
            glversdef << (m.shaderProfile() == ShaderManager::CoreProfile
                            ? " core"
                            : " compatibility");
        glversdef << sys::io::endl;
        if (vers <= 150) {
            glversdef << "#define SL_in attribute" << sys::io::endl;
            glversdef << "#define SL_out varying" << sys::io::endl;
        } else {
            glversdef << "#define SL_in in" << sys::io::endl;
            glversdef << "#define SL_out out" << sys::io::endl;
        }
        proc.appendString(glversdef.str());
    }

    proc.addDefines(m.globalDefines());
    proc.addDefines(defines);
}

ShaderCompilerQueue::ShaderCompilerQueue(ShaderCompiler &comp,
                                         ShaderObjects &compiled,
                                         ShaderCompileFlags flgs)
  : self(new Data(*this, comp, compiled, flgs))
{}

ShaderCompiler &
ShaderCompilerQueue::shaderCompiler()
{
    return self->compiler;
}

void
ShaderCompilerQueue::enqueueReload(const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(so);
    if (self->inQueue.count(so->shaderSource()->key()) > 0)
        return;
    auto exec = [so, this]() -> std::shared_ptr<ShaderObject> {
        sys::io::stdout() << "reloadIfOutdated: " << so->shaderSource()->key()
                          << sys::io::endl;
        auto [new_so, state] = so->self->reloadIfOutdated(*this);
        switch (state) {
        case ReloadState::Uptodate:
            ASSERT(!new_so);
            return so;
        case ReloadState::Outdated:
            ASSERT(new_so);
            return { std::move(new_so) };
        case ReloadState::Failed:
            ASSERT(!new_so);
            return so;
        }
        CASE_UNREACHABLE;
    };
    self->toCompile.push({ so->shaderSource(), std::move(exec) });
}

void
ShaderCompilerQueue::enqueueLoad(const std::shared_ptr<ShaderSource> &src)
{
    ASSERT(src);
    if (self->inQueue.count(src->key()) > 0)
        return;
    auto exec = [src, this]() -> std::shared_ptr<ShaderObject> {
        sys::io::stdout() << "load: " << src->key() << sys::io::endl;
        return ShaderSource::Data::load(src, *this);
    };
    self->toCompile.push({ src, std::move(exec) });
}

void
ShaderCompilerQueue::compileAll()
{
    for (; !wasError() && !self->toCompile.empty();) {

        auto job = std::move(self->toCompile.front());
        self->toCompile.pop();
        const ShaderSourceKey &key = job.source->key();

        auto it = self->compiled.find(key);
        if (it != self->compiled.end()) {
            continue;
        }

        std::shared_ptr<ShaderObject> so;
        bool cache_hit = false;
        if (self->flags & SC_LOOKUP_CACHE) {
            if ((so = self->compiler.shaderCache()->lookup(key))) {
                ASSERT(key == so->shaderSource()->key());
                cache_hit = true;

                ASSERT(so);
                if (self->flags & SC_CHECK_OUTDATED)
                    so = self->reload(so);
            }
        }

        if (!cache_hit)
            so = job.exec();

        if (!so) {
            this->pushError(ShaderCompilerError::CompilationFailed);
        } else {
            self->put(so);

            for (auto &dep : so->self->dependencies)
                enqueueLoad(dep);
        }
    }
}

void
ShaderCompilerQueue::Data::put(const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(so);
    ASSERT(compiled.count(so->shaderSource()->key()) == 0);

    compiled.insert(std::make_pair(so->shaderSource()->key(), so));

    if (flags & SC_PUT_CACHE) {
        std::shared_ptr<ShaderObject> so_mut(so);
        ShaderCache::put(compiler.shaderCache(), so_mut);
    }
}

std::shared_ptr<ShaderObject>
ShaderCompilerQueue::Data::reload(const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(so);
    auto new_so = so->self->reloadIfOutdated(self).first;
    if (!new_so)
        return so;
    return new_so;
}

} // namespace glt
