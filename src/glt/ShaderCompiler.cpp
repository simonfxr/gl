#include "glt/ShaderCompiler.hpp"

#include "err/err.hpp"
#include "err/log.hpp"
#include "glt/GLObject.hpp"
#include "glt/GLSLPreprocessor.hpp"
#include "glt/utils.hpp"
#include "sys/fs.hpp"
#include "sys/io/Stream.hpp"
#include "sys/measure.hpp"
#include "util/range.hpp"

#include <algorithm>
#include <functional>
#include <queue>
#include <stack>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <variant>

namespace err {
template<>
struct LogTraits<glt::ShaderCompilerQueue>
{
    static auto getDestination(glt::ShaderCompilerQueue &x)
    {
        return makeLogSettings(x.shaderCompiler().shaderManager().out());
    }
};
} // namespace err

#define COMPILER_ERR_MSG(comp, ec, msg) LOG_RAISE_ERROR(comp, ec, msg)

namespace glt {

PP_DEF_ENUM_IMPL(GLT_SHADER_COMPILER_ERROR_ENUM_DEF)

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
    const ShaderType type;
    const std::variant<StringSource, FileSource> source;

    static std::shared_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> &,
      const StringSource &,
      ShaderCompilerQueue &);

    static std::shared_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> &,
      const FileSource &,
      ShaderCompilerQueue &);

    static std::shared_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> &,
      ShaderCompilerQueue &);
};

DECLARE_PIMPL_DEL(ShaderSource)

struct ShaderObject::InitArgs
{
    std::shared_ptr<ShaderSource> source;
    std::variant<StringShaderObject, FileShaderObject> obj;
};

struct ShaderObject::Data
{
    ShaderObject &self;
    GLShaderObject handle{};
    ShaderIncludes includes;
    ShaderDependencies dependencies;
    std::weak_ptr<ShaderCache> cache;
    std::shared_ptr<ShaderSource> source;
    std::variant<StringShaderObject, FileShaderObject> object;

    Data(ShaderObject &self_, InitArgs &&args)
      : self(self_), source(std::move(args.source)), object(std::move(args.obj))
    {}

    std::pair<std::shared_ptr<ShaderObject>, ReloadState> reloadIfOutdated(
      ShaderCompilerQueue &);

    void linkCache(const std::shared_ptr<ShaderCache> &);
    void unlinkCache(const std::shared_ptr<ShaderCache> &);

    static std::shared_ptr<ShaderObject> makeStringShaderObject(
      std::shared_ptr<ShaderSource>,
      const StringSource &);

    static std::shared_ptr<ShaderObject> makeFileShaderObject(
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
hash(std::string_view source)
{
    return "h" + std::to_string(std::hash<std::string_view>{}(source));
}

struct ShaderTypeMapping
{
    const char *fileExtension;
    ShaderType type;
    GLenum glType;
};

const ShaderTypeMapping shaderTypeMappings[] = {
    { "frag", ShaderType::FragmentShader, GL_FRAGMENT_SHADER },
    { "vert", ShaderType::VertexShader, GL_VERTEX_SHADER },
    { "geom", ShaderType::GeometryShader, GL_GEOMETRY_SHADER },
    { "tctl", ShaderType::TesselationControl, GL_TESS_CONTROL_SHADER },
    { "tevl", ShaderType::TesselationEvaluation, GL_TESS_EVALUATION_SHADER }
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
translateShaderType(ShaderType type,
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

    bool ok{};
    {
        auto logmsg = err::beginLog(scq);
        logmsg << "compiling " << (name.empty() ? " <embedded code> " : name)
               << " ... ";

        if (logmsg &&
            scq.shaderCompiler().shaderManager().dumpShadersEnabled()) {
            logmsg << sys::io::endl;
            logmsg << "BEGIN SHADER SOURCE[" << *shader << "]" << sys::io::endl;
            for (const auto i : irange(nsegments))
                logmsg << std::string_view{ segments[i],
                                            size_t(segLengths[i]) };
            logmsg << sys::io::endl;
            logmsg << "END SHADER SOURCE" << sys::io::endl;
        }

        GL_CALL(glShaderSource, *shader, nsegments, segments, segLengths);
        double wct;
        measure_time(wct, glCompileShader(*shader));
        GL_CHECK_ERRORS();

        GLint success;
        GL_CALL(glGetShaderiv, *shader, GL_COMPILE_STATUS, &success);
        ok = success == GL_TRUE;
        logmsg << (ok ? "success" : "failed") << " (" << (wct * 1000) << " ms)"
               << sys::io::endl;
    }

    auto logmsg =
      err::beginLog(scq, ok ? err::LogLevel::Info : err::LogLevel::Error);
    if (logmsg)
        printShaderLog(shader, logmsg.out());

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

        // log_len includes null terminator
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
translateShaderType(ShaderType type,
                    GLenum *gltype,
                    const std::string &basename)
{

    if (type == ShaderType::GuessShaderType &&
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

ShaderSource::ShaderSource(ShaderSource::Data &&args)
  : self(new Data(std::move(args)))
{}

const ShaderSourceKey &
ShaderSource::key() const
{
    return self->key;
}

std::shared_ptr<ShaderSource>
ShaderSource::makeFileSource(ShaderType ty, std::string path)
{
    ASSERT(sys::fs::fileExists(path));
    return std::make_shared<ShaderSource>(
      Data{ path, ty, { FileSource{ std::move(path) } } });
}

std::shared_ptr<ShaderSource>
ShaderSource::makeStringSource(ShaderType ty, std::string source)
{
    auto h = hash(source);
    return std::make_shared<ShaderSource>(
      Data{ std::move(h), ty, { StringSource{ std::move(source) } } });
}

std::shared_ptr<ShaderObject>
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

std::shared_ptr<ShaderObject>
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

    preproc.processFileRecursively(std::string(path));
    if (preproc.wasError())
        return nullptr;

    if (!compilePreprocessed(
          scq, gltype, so->self->source->key(), preproc, so->self->handle))
        return nullptr;
    return so;
}

std::shared_ptr<ShaderObject>
ShaderSource::Data::load(const std::shared_ptr<ShaderSource> &src,
                         ShaderCompilerQueue &scq)
{
    ASSERT(src);
    return std::visit([&](auto &&arg) { return load(src, arg, scq); },
                      src->self->source);
}

ShaderObject::ShaderObject(InitArgs &&args)
  : self(new Data(*this, std::move(args)))
{}

ShaderObject::~ShaderObject()
{
    auto c = self->cache.lock();
    if (c)
        c->remove(this);
}

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

std::pair<std::shared_ptr<ShaderObject>, ReloadState>
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
        [[fallthrough]];
    case ReloadState::Uptodate:
        return { nullptr, state };
    case ReloadState::Outdated:
        auto new_so = ShaderSource::Data::load(source, scq);
        if (!new_so)
            return { nullptr, ReloadState::Failed };
        return { std::move(new_so), ReloadState::Outdated };
    }
    UNREACHABLE;
}

void
ShaderObject::Data::linkCache(const std::shared_ptr<ShaderCache> &newcache)
{
    ASSERT(newcache && cache.expired());
    cache = newcache;
}

std::shared_ptr<ShaderObject>
ShaderObject::Data::makeStringShaderObject(std::shared_ptr<ShaderSource> src,
                                           const StringSource &ssrc)
{
    auto so = std::make_shared<ShaderObject>(
      InitArgs{ std::move(src), { StringShaderObject{ ssrc } } });
    return so;
}

std::shared_ptr<ShaderObject>
ShaderObject::Data::makeFileShaderObject(std::shared_ptr<ShaderSource> src,
                                         const FileSource &fsrc,
                                         sys::fs::FileTime mtime)
{
    ASSERT(sys::fs::fileExists(fsrc.path));
    auto so = std::make_shared<ShaderObject>(
      InitArgs{ std::move(src), { FileShaderObject{ fsrc, mtime } } });
    return so;
}

ShaderCache::ShaderCache() : self(new Data(*this)) {}

ShaderCache::~ShaderCache()
{
    flush();
}

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
ShaderCache::put(const std::shared_ptr<ShaderObject> &so)
{
    auto cache = shared_from_this();
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
        cache->self->entries.insert(
          std::make_pair(key, std::make_pair(next_version, so)));
    }

    cache->self->checkValid();
    return !already_present;
}

bool
ShaderCache::remove(ShaderObject *so)
{
    return self->remove(so);
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
ShaderCompiler::guessShaderType(std::string_view path, ShaderType *res)
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
            glversdef << (m.shaderProfile() == ShaderProfile::Core
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
        UNREACHABLE;
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
        auto cache = compiler.shaderCache();
        if (cache)
            cache->put(so_mut);
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
