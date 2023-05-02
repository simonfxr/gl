#include "glt/GLSLPreprocessor.hpp"

#include "err/err.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "sys/io.hpp"

#include <bl/ptr.hpp>

#include <cstring>
#include <stack>
#include <unordered_set>

using namespace bl::literals;
using pp = glt::Preprocessor;
using Impl = glt::GLSLPreprocessor::Impl;

namespace glt {
namespace {

struct FileContext
{
    const char *pos;
};

struct ProcessingState
{
    std::unordered_set<bl::string> visitingFiles;
    std::unordered_set<bl::string> deps;
    std::unordered_set<bl::string> incs;
    std::stack<FileContext> stack;
};

bool
parseFileArg(const pp::DirectiveContext &ctx, const char *&arg, uint32_t &len)
{

    arg = nullptr;
    len = 0;

    const char *begin = ctx.content.data.data() + ctx.endDirective;
    const char *end = ctx.content.data.data() + ctx.lineOffset + ctx.lineLength;

    while (begin < end && isspace(*begin))
        ++begin;

    if (begin >= end)
        return false;

    auto term = *begin == '"' ? '"' : *begin == '<' ? '>' : '\0';

    if (term == 0)
        return false;

    begin++;
    arg = begin;

    while (begin < end && *begin != term)
        ++begin;

    if (begin < end && arg <= begin) {
        len = static_cast<uint32_t>(begin - arg);
        return true;
    }
    arg = nullptr;
    return false;
}

} // namespace

struct GLSLPreprocessor::Impl : public GLSLPreprocessor
{
    bl::ptr<Preprocessor> preprocessor;
    const IncludePath &includePath;
    ShaderIncludes &includes;
    ShaderDependencies &dependencies;

    bl::vec<uint32_t> segLengths_;
    bl::vec<const char *> segments_;
    bl::vec<Array<char>> contents;

    bl::box<ProcessingState> state;

    struct DependencyHandler
    {
        using Self = DependencyHandler;
        GLSLPreprocessor::Impl &glslPP;
        explicit DependencyHandler(GLSLPreprocessor::Impl &glslPP)
          : glslPP(glslPP)
        {}

        friend void tag_invoke(pp::directiveScopeChanged_f,
                               Self &,
                               pp::DirectiveScopeChange,
                               const pp::ContentContext &)
        {}

        friend void tag_invoke(pp::directiveEncountered_f,
                               Self &self,
                               const pp::DirectiveContext &ctx);
    };

    struct IncludeHandler
    {
        using Self = IncludeHandler;
        GLSLPreprocessor::Impl &glslPP;

        explicit IncludeHandler(GLSLPreprocessor::Impl &glslPP) : glslPP(glslPP)
        {}

        friend void tag_invoke(pp::directiveScopeChanged_f,
                               Self &,
                               pp::DirectiveScopeChange,
                               const pp::ContentContext &);

        friend void tag_invoke(pp::directiveEncountered_f,
                               Self &self,
                               const pp::DirectiveContext &ctx);
    };

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    Impl(bl::ptr<Preprocessor> preprocessor,
         const IncludePath &incPath,
         ShaderIncludes &incs,
         ShaderDependencies &deps)
      : preprocessor(preprocessor)
      , includePath(incPath)
      , includes(incs)
      , dependencies(deps)
      , includeHandler(*this)
      , dependencyHandler(*this)
    {
        preprocessor->installHandler("include"_s, includeHandler);
        preprocessor->installHandler("need"_s, dependencyHandler);
    }
};

BL_DEF_RUNTIME_SIZED(GLSLPreprocessor, GLSLPreprocessor::Impl);

GLSLPreprocessor &
GLSLPreprocessor::construct_at(void *p,
                               bl::ptr<Preprocessor> preprocessor,
                               const IncludePath &incPath,
                               ShaderIncludes &incs,
                               ShaderDependencies &deps) noexcept
{
    return construct_impl(p, preprocessor, incPath, incs, deps);
}

void
GLSLPreprocessor::appendString(bl::string_view str)
{
    if (!str.empty()) {
        auto &data = m().contents.emplace_back(str);
        m().segments_.push_back(data.data());
        m().segLengths_.push_back(static_cast<uint32_t>(data.size()));
    }
}

void
GLSLPreprocessor::addDefines(const PreprocessorDefinitions &defines)
{
    for (const auto &define : defines) {
        sys::io::ByteStream defn;
        defn << "#define "_k << define.first << " "_k << define.second
             << "\n"_k;
        appendString(defn.str());
    }
}

void
GLSLPreprocessor::advanceSegments(const pp::DirectiveContext &ctx)
{
    FileContext &frame = m().state->stack.top();

    size_t seglen = ctx.content.data.data() + ctx.lineOffset - frame.pos;

    if (seglen > 0) {
        m().segments_.push_back(frame.pos);
        m().segLengths_.push_back(static_cast<uint32_t>(seglen));
    }

    frame.pos = ctx.content.data.data() + ctx.lineOffset + ctx.lineLength;
}

const bl::vec<const char *> &
GLSLPreprocessor::segments() const noexcept
{
    return m().segments_;
}

const bl::vec<uint32_t> &
GLSLPreprocessor::segLengths() const noexcept
{
    return m().segLengths_;
}

void
GLSLPreprocessor::processFileRecursively(bl::string &&file)
{
    if (m().preprocessor->wasError())
        return;

    ASSERT(sys::fs::isAbsolute(file));
    auto data = sys::io::readFile(file);
    if (!data) {
        m().preprocessor->setError();
        return;
    }

    auto &dref = m().contents.emplace_back(bl::move(data).value());

    m().preprocessor->name(bl::move(file));
    m().preprocessor->process(bl::string_view(dref.data(), dref.size()));
}

void
tag_invoke(pp::directiveEncountered_f,
           Impl::DependencyHandler &self,
           const pp::DirectiveContext &ctx)
{

    auto &proc = ctx.content.processor;

    const char *arg;
    uint32_t len;

    // FIXME: add an option to specify ShaderType explicitly

    if (!parseFileArg(ctx, arg, len)) {
        proc.out() << ctx.content.name
                   << ": #need-directive: invalid parameter"_k
                   << "\n"_k;
        proc.setError();
        return;
    }

    auto file = bl::string_view{ arg, arg + len };
    auto realPath =
      sys::fs::lookup(self.glslPP.includePath, file, sys::fs::ObjectType::File);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot find file: "_k << file
                   << "\n"_k;
        proc.setError();
        return;
    }

    self.glslPP.advanceSegments(ctx);

    auto opt_stype = ShaderCompiler::guessShaderType(file);
    if (!opt_stype) {
        proc.out()
          << ctx.content.name
          << ": #need-directive: cannot guess shader type based on name: "_k
          << file << "\n"_k;
        proc.setError();
        return;
    }
    auto stype = *opt_stype;

    auto absPath = sys::fs::absolutePath(realPath);
    if (self.glslPP.state->deps.insert(absPath).second)
        self.glslPP.dependencies.push_back(
          ShaderSource::makeFileSource(stype, absPath));
}

void
tag_invoke(pp::directiveScopeChanged_f,
           Impl::IncludeHandler &self,
           pp::DirectiveScopeChange scopeChange,
           const pp::ContentContext &ctx)
{
    if (scopeChange == pp::beginDirective) {
        if (!ctx.name.empty())
            self.glslPP.state->visitingFiles.insert(ctx.name);

        FileContext frame{};
        frame.pos = ctx.data.data();
        self.glslPP.state->stack.push(frame);
    } else {
        auto &frame = self.glslPP.state->stack.top();
        auto seglen = ctx.data.data() + ctx.data.size() - frame.pos;

        if (seglen > 0) {
            self.glslPP.segments_.push_back(frame.pos);
            self.glslPP.segLengths_.push_back(static_cast<uint32_t>(seglen));
        }

        self.glslPP.state->stack.pop();

        if (!ctx.name.empty())
            self.glslPP.state->visitingFiles.erase(ctx.name);

        if (self.glslPP.state->stack.empty())
            *self.glslPP.state = ProcessingState{};
    }
}

void
tag_invoke(pp::directiveEncountered_f,
           Impl::IncludeHandler &self,
           const pp::DirectiveContext &ctx)
{
    auto &proc = *self.glslPP.preprocessor;

    const char *arg;
    uint32_t len;

    if (!parseFileArg(ctx, arg, len) || len == 0) {
        proc.out() << ctx.content.name
                   << ": #include-directive: invalid parameter"_k
                   << "\n"_k;
        proc.setError();
        return;
    }

    auto file = bl::string_view{ arg, arg + len };
    auto realPath =
      sys::fs::lookup(self.glslPP.includePath, file, sys::fs::ObjectType::File);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot find file: "_k << file
                   << "\n"_k;
        proc.setError();
        return;
    }

    self.glslPP.advanceSegments(ctx);

    auto filestat = sys::fs::stat(realPath);
    if (!filestat) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot open file: "_k << realPath
                   << "\n"_k;
        proc.setError();
        return;
    }
    self.glslPP.state->incs.insert(filestat->absolute);

    if (!self.glslPP.state->visitingFiles.contains(filestat->absolute)) {
        self.glslPP.includes.emplace_back(filestat->absolute, filestat->mtime);

        auto res = sys::io::readFile(filestat->absolute);
        if (!res) {
            proc.setError();
            return;
        }

        auto &dref = self.glslPP.contents.emplace_back(bl::move(res).value());
        proc.name(bl::move(filestat->absolute));
        proc.process(bl::string_view(dref.data(), dref.size()));
    }
}

} // namespace glt
