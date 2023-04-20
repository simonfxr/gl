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
#include <array>
#include <functional>
#include <iterator>
#include <numeric>
#include <queue>
#include <stack>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

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
    std::string source;
};
} // namespace

namespace {
struct FileSource
{
    std::string path;
};
} // namespace

namespace {
struct StringShaderObject
{
    std::reference_wrapper<const StringSource> source;
};
} // namespace

namespace {
struct FileShaderObject
{
    std::reference_wrapper<const FileSource> source;
    sys::fs::FileTime mtime;
};
} // namespace

struct ShaderSource::Data
{
    const ShaderSourceKey key;
    const std::variant<StringSource, FileSource> source;
    const ShaderType type;

    static std::shared_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> & /*src*/,
      const StringSource & /*strsrc*/,
      ShaderCompilerQueue & /*scq*/);

    static std::shared_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> & /*src*/,
      const FileSource & /*filesrc*/,
      ShaderCompilerQueue & /*scq*/);

    static std::shared_ptr<ShaderObject> load(
      const std::shared_ptr<ShaderSource> & /*src*/,
      ShaderCompilerQueue & /*scq*/);
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
      : self(self_), source(bl::move(args.source)), object(bl::move(args.obj))
    {}

    std::pair<std::shared_ptr<ShaderObject>, ReloadState> reloadIfOutdated(
      ShaderCompilerQueue & /*scq*/);

    void linkCache(const std::shared_ptr<ShaderCache> & /*newcache*/);
    void unlinkCache(const std::shared_ptr<ShaderCache> & /*newcache*/);

    static std::shared_ptr<ShaderObject> makeStringShaderObject(
      std::shared_ptr<ShaderSource> /*src*/,
      const StringSource & /*ssrc*/);

    static std::shared_ptr<ShaderObject> makeFileShaderObject(
      std::shared_ptr<ShaderSource> /*src*/,
      const FileSource & /*fsrc*/,
      sys::fs::FileTime mtime);
};

DECLARE_PIMPL_DEL(ShaderObject)

struct ShaderCache::Data
{
    ShaderCacheEntries entries;
    ShaderCache &self;
    void checkValid();
    bool remove(ShaderObject * /*so*/);

    explicit Data(ShaderCache &self_) : self(self_) {}
};

DECLARE_PIMPL_DEL(ShaderCache)

struct ShaderCompiler::Impl : public ShaderCompiler
{
    ShaderManager &shaderManager;
    PreprocessorDefinitions defines;
    std::shared_ptr<ShaderCache> cache;

    explicit Impl(ShaderManager &sm) : shaderManager(sm) {}
    void initPreprocessor(GLSLPreprocessor & /*proc*/);
};

BL_DEFINE_RUNTIME_SIZED(ShaderCompiler, ShaderCompiler::Impl);

struct ShaderCompilerQueue::Impl : ShaderCompilerQueue
{
    ShaderCompiler &compiler;
    ShaderObjects &compiled;
    QueuedJobs inQueue;
    CompileJobs toCompile;
    const ShaderCompileFlags flags;

    Impl(ShaderCompiler &comp,
         ShaderObjects &_compiled,
         ShaderCompileFlags _flags)
      : compiler(comp), compiled(_compiled), flags(_flags)
    {}

    void put(const std::shared_ptr<ShaderObject> & /*so*/);
    std::shared_ptr<ShaderObject> reload(
      const std::shared_ptr<ShaderObject> & /*so*/);
};

BL_DEFINE_RUNTIME_SIZED(ShaderCompilerQueue, ShaderCompilerQueue::Impl);

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

constexpr const auto shaderTypeMappings = std::to_array<ShaderTypeMapping>(
  { { "frag", ShaderType::FragmentShader, GL_FRAGMENT_SHADER },
    { "vert", ShaderType::VertexShader, GL_VERTEX_SHADER },
    { "geom", ShaderType::GeometryShader, GL_GEOMETRY_SHADER },
    { "tctl", ShaderType::TesselationControl, GL_TESS_CONTROL_SHADER },
    { "tevl", ShaderType::TesselationEvaluation, GL_TESS_EVALUATION_SHADER } });

bool
compilePreprocessed(ShaderCompilerQueue & /*scq*/,
                    GLenum /*shader_type*/,
                    const std::string & /*name*/,
                    GLSLPreprocessor & /*proc*/,
                    GLShaderObject & /*shader*/);

void
printShaderLog(GLShaderObject & /*shader*/, sys::io::OutStream &out);

std::optional<GLenum>
translateShaderType(ShaderType type, const std::string &basename = "");

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

    auto nsegments = static_cast<GLsizei>(proc.segments.size());
    const char **segments = proc.segments.data();
    static_assert(sizeof(GLint) == sizeof(proc.segLengths[0]));
    auto *p0 = proc.segLengths.data();
    auto segLengths = reinterpret_cast<const GLint *>(p0); // NOLINT

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
            logmsg << "\n";
            logmsg << "BEGIN SHADER SOURCE[" << *shader << "]\n";
            for (const auto i : irange(nsegments))
                logmsg << std::string_view{
                    segments[i], static_cast<size_t>(segLengths[i])
                };
            logmsg << "\n";
            logmsg << "END SHADER SOURCE\n";
        }

        GL_CALL(glShaderSource, *shader, nsegments, segments, segLengths);
        double wct;
        measure_time(wct, glCompileShader(*shader));
        GL_CHECK_ERRORS();

        GLint success;
        GL_CALL(glGetShaderiv, *shader, GL_COMPILE_STATUS, &success);
        ok = success == GL_TRUE;
        logmsg << (ok ? "success" : "failed") << " (" << (wct * 1000) << " ms)"
               << "\n";
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
        auto log = std::vector<GLchar>(static_cast<size_t>(log_len));
        GL_CALL(glGetShaderInfoLog, *shader, log_len, nullptr, log.data());

        GLchar *logBegin = log.data();
        while (logBegin < &log[log_len - 1] && isspace(*logBegin))
            ++logBegin;

        GLchar *end = &log[log_len - 1];
        while (end > logBegin && isspace(end[-1]))
            --end;
        *end = 0;
        const char *log_msg = log.data();

        if (logBegin == end) {
            out << "shader compile log empty\n";
        } else {
            out << "shader compile log:\n"
                << log_msg << "\n"
                << "end compile log\n";
        }
    }
}

std::optional<GLenum>
translateShaderType(ShaderType type, const std::string &basename)
{

    if (type == ShaderType::GuessShaderType) {
        auto guess = ShaderCompiler::guessShaderType(basename);
        if (!guess)
            return false;
        type = *guess;
    }

    for (auto shaderTypeMapping : shaderTypeMappings)
        if (shaderTypeMapping.type == type)
            return { shaderTypeMapping.glType };

    return std::nullopt;
}
} // namespace

ShaderSource::ShaderSource(ShaderSource::Data &&args)
  : self(new Data(bl::move(args)))
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
      Data{ path, { FileSource{ bl::move(path) } }, ty });
}

std::shared_ptr<ShaderSource>
ShaderSource::makeStringSource(ShaderType ty, std::string source)
{
    auto h = hash(source);
    return std::make_shared<ShaderSource>(
      Data{ bl::move(h), { StringSource{ bl::move(source) } }, ty });
}

std::shared_ptr<ShaderObject>
ShaderSource::Data::load(const std::shared_ptr<ShaderSource> &src,
                         const StringSource &strsrc,
                         ShaderCompilerQueue &scq)
{
    auto opt_gltype = translateShaderType(src->self->type);

    if (!opt_gltype) {
        COMPILER_ERR_MSG(
          scq, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return nullptr;
    }
    auto gltype = *opt_gltype;

    auto so = ShaderObject::Data::makeStringShaderObject(src, strsrc);
    GLSLPreprocessor preproc(
      scq.shaderCompiler().shaderManager().shaderDirectories(),
      so->self->includes,
      so->self->dependencies);
    preproc.name("");
    scq.shaderCompiler().m().initPreprocessor(preproc);

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

    const auto &path = filesrc.path;
    auto opt_gltype = translateShaderType(src->self->type, path);
    if (!opt_gltype) {
        COMPILER_ERR_MSG(
          scq, ShaderCompilerError::InvalidShaderType, "unknown ShaderType");
        return nullptr;
    }
    auto gltype = *opt_gltype;

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
    scq.shaderCompiler().m().initPreprocessor(preproc);

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
  : self(new Data(*this, bl::move(args)))
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
              auto current_mtime =
                sys::fs::modificationTime(arg.source.get().path);
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
        return { bl::move(new_so), ReloadState::Outdated };
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
      InitArgs{ bl::move(src), { StringShaderObject{ std::ref(ssrc) } } });
    return so;
}

std::shared_ptr<ShaderObject>
ShaderObject::Data::makeFileShaderObject(std::shared_ptr<ShaderSource> src,
                                         const FileSource &fsrc,
                                         sys::fs::FileTime mtime)
{
    ASSERT(sys::fs::fileExists(fsrc.path));
    auto so = std::make_shared<ShaderObject>(
      InitArgs{ bl::move(src), { FileShaderObject{ fsrc, mtime } } });
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
    auto found = best_version >= 0;
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

ShaderCompiler &
ShaderCompiler::construct_at(void *p, ShaderManager &sm) noexcept
{
    return construct_impl(p, sm);
}

ShaderManager &
ShaderCompiler::shaderManager()
{
    return m().shaderManager;
}

const std::shared_ptr<ShaderCache> &
ShaderCompiler::shaderCache()
{
    return m().cache;
}

void
ShaderCompiler::init()
{
    m().cache = m().shaderManager.globalShaderCache();
}

std::optional<ShaderType>
ShaderCompiler::guessShaderType(std::string_view path)
{
    auto ext = sys::fs::extension(path);

    for (auto shaderTypeMapping : shaderTypeMappings)
        if (ext == shaderTypeMapping.fileExtension)
            return { shaderTypeMapping.type };

    return std::nullopt;
}

void
ShaderCompiler::Impl::initPreprocessor(GLSLPreprocessor &proc)
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
        glversdef << "\n";
        if (vers <= 150) {
            glversdef << "#define SL_in attribute\n";
            glversdef << "#define SL_out varying\n";
        } else {
            glversdef << "#define SL_in in\n";
            glversdef << "#define SL_out out\n";
        }
        proc.appendString(glversdef.str());
    }

    proc.addDefines(m.globalDefines());
    proc.addDefines(defines);
}

ShaderCompilerQueue &
ShaderCompilerQueue::construct_at(void *p,
                                  ShaderCompiler &comp,
                                  ShaderObjects &compiled,
                                  ShaderCompileFlags flgs) noexcept
{
    return construct_impl(p, comp, compiled, flgs);
}

ShaderCompiler &
ShaderCompilerQueue::shaderCompiler()
{
    return m().compiler;
}

void
ShaderCompilerQueue::enqueueReload(const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(so);
    if (m().inQueue.contains(so->shaderSource()->key()))
        return;
    auto exec = [so, this]() -> std::shared_ptr<ShaderObject> {
        sys::io::stdout() << "reloadIfOutdated: " << so->shaderSource()->key()
                          << "\n";
        auto [new_so, state] = so->self->reloadIfOutdated(*this);
        switch (state) {
        case ReloadState::Uptodate:
            ASSERT(!new_so);
            return so;
        case ReloadState::Outdated:
            ASSERT(new_so);
            return { bl::move(new_so) };
        case ReloadState::Failed:
            ASSERT(!new_so);
            return so;
        }
        UNREACHABLE;
    };
    m().toCompile.push({ so->shaderSource(), bl::move(exec) });
}

void
ShaderCompilerQueue::enqueueLoad(const std::shared_ptr<ShaderSource> &src)
{
    ASSERT(src);
    if (m().inQueue.contains(src->key()))
        return;
    auto exec = [src, this]() -> std::shared_ptr<ShaderObject> {
        sys::io::stdout() << "load: " << src->key() << "\n";
        return ShaderSource::Data::load(src, *this);
    };
    m().toCompile.push({ src, bl::move(exec) });
}

void
ShaderCompilerQueue::compileAll()
{
    for (; !wasError() && !m().toCompile.empty();) {

        auto job = bl::move(m().toCompile.front());
        m().toCompile.pop();
        const ShaderSourceKey &key = job.source->key();

        auto it = m().compiled.find(key);
        if (it != m().compiled.end())
            continue;

        std::shared_ptr<ShaderObject> so;
        auto cache_hit = false;
        if (m().flags & SC_LOOKUP_CACHE) {
            if ((so = m().compiler.shaderCache()->lookup(key))) {
                ASSERT(key == so->shaderSource()->key());
                cache_hit = true;

                ASSERT(so);
                if (m().flags & SC_CHECK_OUTDATED)
                    so = m().reload(so);
            }
        }

        if (!cache_hit)
            so = job.exec();

        if (!so)
            return this->pushError(ShaderCompilerError::CompilationFailed);

        m().put(so);

        for (auto &dep : so->self->dependencies)
            enqueueLoad(dep);
    }
}

void
ShaderCompilerQueue::Impl::put(const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(so);
    ASSERT(!compiled.contains(so->shaderSource()->key()));

    compiled.insert(std::make_pair(so->shaderSource()->key(), so));

    if (flags & SC_PUT_CACHE)
        if (auto cache = compiler.shaderCache())
            cache->put(so);
}

std::shared_ptr<ShaderObject>
ShaderCompilerQueue::Impl::reload(const std::shared_ptr<ShaderObject> &so)
{
    ASSERT(so);
    auto new_so = so->self->reloadIfOutdated(m()).first;
    if (!new_so)
        return so;
    return new_so;
}

} // namespace glt
