#include "glt/GLSLPreprocessor.hpp"

#include "bl/hashset.hpp"
#include "err/err.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "sys/io.hpp"
#include "util/string.hpp"

#include <ctype.h>

namespace glt {

struct IncludeHandler : public Preprocessor::DirectiveHandler
{
    virtual void beginProcessing(
      const Preprocessor::ContentContext &) final override;
    virtual void directiveEncountered(
      const Preprocessor::DirectiveContext &) final override;
    virtual void endProcessing(
      const Preprocessor::ContentContext &) final override;
};

struct DependencyHandler : public Preprocessor::DirectiveHandler
{
    virtual void directiveEncountered(
      const Preprocessor::DirectiveContext &) final override;
};

namespace {
bool
parseFileArg(const Preprocessor::DirectiveContext &ctx,
             bl::string_view &parsed_path)
{
    parsed_path = {};

    const char *begin = ctx.directive.endp();
    const char *end = ctx.line.endp();

    while (begin < end && isspace(*begin))
        ++begin;

    if (begin >= end)
        return false;

    char term;
    if (*begin == '"')
        term = '"';
    else if (*begin == '<')
        term = '>';
    else
        return false;

    begin++;
    auto path_begin = begin;

    while (begin < end && *begin != term)
        ++begin;

    if (begin < end && path_begin <= begin) {
        parsed_path = bl::string_view(path_begin, begin - path_begin);
        return true;
    }
    return false;
}
} // namespace

namespace {
void
checkSegment(GLSLPreprocessor &proc, const char *s, size_t len)
{
    ASSERT(len < 8000);
    for (const auto &str : proc.contents) {
        if (str.beginp() <= s && s < str.endp()) {
            ASSERT(s + len <= str.endp());
            return;
        }
    }
    UNREACHABLE_MSG("no string containing segment found");
}
} // namespace

struct GLSLPreprocessor::IncludeHandler : public Preprocessor::DirectiveHandler
{
    virtual void beginProcessing(
      const Preprocessor::ContentContext &) final override;
    virtual void directiveEncountered(
      const Preprocessor::DirectiveContext &) final override;
    virtual void endProcessing(
      const Preprocessor::ContentContext &) final override;
};

struct GLSLPreprocessor::DependencyHandler
  : public Preprocessor::DirectiveHandler
{
    virtual void directiveEncountered(
      const Preprocessor::DirectiveContext &) final override;
};

struct GLSLPreprocessor::Data
{
    bl::hashset<bl::string> visitingFiles;
    bl::hashset<bl::string> deps;
    bl::hashset<bl::string> incs;
    bl::vector<bl::string_view> stack;

    const IncludePath &includePath;
    ShaderIncludes &includes;
    ShaderDependencies &dependencies;

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    Data(const IncludePath &includePath_,
         ShaderIncludes &includes_,
         ShaderDependencies &dependencies_)
      : includePath(includePath_)
      , includes(includes_)
      , dependencies(dependencies_)
    {}

    void clear()
    {
        visitingFiles.clear();
        deps.clear();
        incs.clear();
        stack.clear();
    }
};

DECLARE_PIMPL_DEL(GLSLPreprocessor)

GLSLPreprocessor::GLSLPreprocessor(const IncludePath &incPath,
                                   ShaderIncludes &incs,
                                   ShaderDependencies &deps)
  : self(new Data(incPath, incs, deps))
{
    this->installHandler("include", self->includeHandler);
    this->installHandler("need", self->dependencyHandler);
}

GLSLPreprocessor::~GLSLPreprocessor() = default;

void
GLSLPreprocessor::appendString(bl::string str)
{
    if (!str.empty()) {
        auto &data = contents.emplace_back(bl::move(str));
        checkSegment(*this, data.data(), data.size());
        segments.push_back(data.data());
        segLengths.push_back(uint32_t(data.size()));
    }
}

void
GLSLPreprocessor::addDefines(const PreprocessorDefinitions &defines)
{
    for (const auto &define : defines)
        appendString(string_concat(
          "#define ", define.key, " ", define.value, sys::io::endl));
}

void
GLSLPreprocessor::advanceSegments(const Preprocessor::DirectiveContext &ctx)
{
    auto &cur_seg = self->stack.back();

    ASSERT(cur_seg.beginp() <= ctx.line.beginp() &&
           ctx.line.endp() <= cur_seg.endp());

    size_t seglen = ctx.line.beginp() - cur_seg.beginp();

    if (seglen > 0) {
        checkSegment(*this, cur_seg.data(), seglen);
        segments.push_back(cur_seg.data());
        segLengths.push_back(uint32_t(seglen));
    }

    cur_seg = cur_seg.substr(ctx.line.endp() - cur_seg.beginp());
}

void
GLSLPreprocessor::processFileRecursively(bl::string &&file)
{
    if (wasError())
        return;

    ASSERT(sys::fs::isAbsolute(file));
    sys::io::HandleError err;
    auto data = sys::io::readFile(out(), file, err);
    if (err != sys::io::HandleError::OK) {
        setError();
        return;
    }

    auto &dref = contents.emplace_back(bl::move(data));
    this->name(bl::move(file));
    process(bl::string_view(dref.data(), dref.size()));
}

void
GLSLPreprocessor::DependencyHandler::directiveEncountered(
  const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.content.processor);

    // FIXME: add an option to specify ShaderType explicitly
    bl::string_view path;
    if (!parseFileArg(ctx, path) || path.empty()) {
        proc.out() << ctx.content.name << ": #need-directive: invalid parameter"
                   << sys::io::endl;
        proc.setError();
        return;
    }

    bl::string realPath = sys::fs::lookup(proc.self->includePath, path);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot find file: " << path
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    ShaderType stype;
    if (!ShaderCompiler::guessShaderType(realPath, &stype)) {
        proc.out()
          << ctx.content.name
          << ": #need-directive: cannot guess shader type based on name: "
          << realPath << sys::io::endl;
        proc.setError();
        return;
    }

    bl::string absPath = sys::fs::absolutePath(realPath);
    if (proc.self->deps.insert(absPath).snd())
        proc.self->dependencies.push_back(
          ShaderSource::makeFileSource(stype, bl::move(absPath)));
}

void
GLSLPreprocessor::IncludeHandler::beginProcessing(
  const Preprocessor::ContentContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.processor);

    if (!ctx.name.empty())
        proc.self->visitingFiles.insert(ctx.name);

    proc.self->stack.push_back(ctx.data);
}

void
GLSLPreprocessor::IncludeHandler::directiveEncountered(
  const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.content.processor);
    bl::string_view path;
    if (!parseFileArg(ctx, path) || path.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: invalid parameter"
                   << sys::io::endl;
        proc.setError();
        return;
    }

    bl::string realPath = sys::fs::lookup(proc.self->includePath, path);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot find file: " << path
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    auto filestat = sys::fs::stat(realPath);
    if (!filestat) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot open file: " << realPath
                   << sys::io::endl;
        proc.setError();
        return;
    }
    proc.self->incs.insert(filestat->absolute);

    if (!proc.self->visitingFiles.contains(filestat->absolute)) {
        proc.self->includes.emplace_back(
          ShaderInclude{ filestat->absolute, filestat->mtime });

        sys::io::HandleError err;
        auto source_code = readFile(proc.out(), filestat->absolute, err);
        if (err != sys::io::HandleError::OK) {
            proc.setError();
            return;
        }

        auto &dref = proc.contents.emplace_back(bl::move(source_code));
        proc.name(bl::move(filestat->absolute));
        proc.process(bl::string_view(dref.data(), dref.size()));
    }
}

void
GLSLPreprocessor::IncludeHandler::endProcessing(
  const Preprocessor::ContentContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.processor);

    auto &cur_seg = proc.self->stack.back();
    size_t seglen = cur_seg.size();

    if (seglen > 0) {
        checkSegment(proc, cur_seg.data(), seglen);
        proc.segments.push_back(cur_seg.data());
        proc.segLengths.push_back(uint32_t(seglen));
    }

    proc.self->stack.pop_back();

    if (!ctx.name.empty())
        proc.self->visitingFiles.erase(ctx.name);

    if (proc.self->stack.empty())
        proc.self->clear();
}

} // namespace glt
