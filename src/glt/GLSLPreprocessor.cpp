#include "glt/GLSLPreprocessor.hpp"

#include "err/err.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "sys/io.hpp"

#include <cstring>
#include <stack>
#include <unordered_set>

namespace glt {

namespace {

bool
parseFileArg(const Preprocessor::DirectiveContext &ctx,
             const char *&arg,
             uint32_t &len)
{

    arg = nullptr;
    len = 0;

    const char *begin = ctx.content.data + ctx.endDirective;
    const char *end = ctx.content.data + ctx.lineOffset + ctx.lineLength;

    while (begin < end && isspace(*begin))
        ++begin;

    if (begin >= end)
        return false;

    char term = *begin == '"' ? '"' : *begin == '<' ? '>' : '\0';

    if (term == 0)
        return false;

    begin++;
    arg = begin;

    while (begin < end && *begin != term)
        ++begin;

    if (begin < end && arg <= begin) {
        len = uint32_t(begin - arg);
        return true;
    }
    arg = nullptr;
    return false;
}

} // namespace

struct FileContext
{
    const char *pos;
};

struct ProcessingState
{
    std::unordered_set<std::string> visitingFiles;
    std::unordered_set<std::string> deps;
    std::unordered_set<std::string> incs;
    std::stack<FileContext> stack;
};

void
ProcessingStateDeleter::operator()(ProcessingState *p) noexcept
{
    delete p;
}

GLSLPreprocessor::GLSLPreprocessor(const IncludePath &incPath,
                                   ShaderIncludes &incs,
                                   ShaderDependencies &deps)
  : includePath(incPath), includes(incs), dependencies(deps)
{
    this->installHandler("include", includeHandler);
    this->installHandler("need", dependencyHandler);
}

GLSLPreprocessor::~GLSLPreprocessor() = default;

void
GLSLPreprocessor::appendString(std::string_view str)
{
    if (!str.empty()) {
        auto &data = contents.emplace_back(view_array(str));
        segments.push_back(data.data());
        segLengths.push_back(uint32_t(data.size()));
    }
}

void
GLSLPreprocessor::addDefines(const PreprocessorDefinitions &defines)
{
    for (const auto &define : defines) {
        sys::io::ByteStream defn;
        defn << "#define " << define.first << " " << define.second
             << sys::io::endl;
        appendString(defn.str());
    }
}

void
GLSLPreprocessor::advanceSegments(const Preprocessor::DirectiveContext &ctx)
{
    FileContext &frame = state->stack.top();

    size_t seglen = ctx.content.data + ctx.lineOffset - frame.pos;

    if (seglen > 0) {
        segments.push_back(frame.pos);
        segLengths.push_back(uint32_t(seglen));
    }

    frame.pos = ctx.content.data + ctx.lineOffset + ctx.lineLength;
}

void
GLSLPreprocessor::processFileRecursively(std::string &&file)
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

    auto &dref = contents.emplace_back(std::move(data));

    this->name(std::move(file));
    process(std::string_view(dref.data(), dref.size()));
}

void
DependencyHandler::directiveEncountered(
  const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.content.processor);

    const char *arg;
    uint32_t len;

    // FIXME: add an option to specify ShaderType explicitly

    if (!parseFileArg(ctx, arg, len)) {
        proc.out() << ctx.content.name << ": #need-directive: invalid parameter"
                   << sys::io::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookup(view_array(proc.includePath), file);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot find file: " << file
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    ShaderType stype;
    if (!ShaderCompiler::guessShaderType(file, &stype)) {
        proc.out()
          << ctx.content.name
          << ": #need-directive: cannot guess shader type based on name: "
          << file << sys::io::endl;
        proc.setError();
        return;
    }

    std::string absPath = sys::fs::absolutePath(realPath);
    if (proc.state->deps.insert(absPath).second)
        proc.dependencies.push_back(
          ShaderSource::makeFileSource(stype, absPath));
}

void
IncludeHandler::beginProcessing(const Preprocessor::ContentContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.processor);

    if (proc.state == nullptr)
        proc.state.reset(new ProcessingState);

    if (!ctx.name.empty())
        proc.state->visitingFiles.insert(ctx.name);

    FileContext frame{};
    frame.pos = ctx.data;
    proc.state->stack.push(frame);
}

void
IncludeHandler::directiveEncountered(const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.content.processor);

    const char *arg;
    uint32_t len;

    if (!parseFileArg(ctx, arg, len) || len == 0) {
        proc.out() << ctx.content.name
                   << ": #include-directive: invalid parameter"
                   << sys::io::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookup(view_array(proc.includePath), file);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot find file: " << file
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
    proc.state->incs.insert(filestat->absolute);

    if (proc.state->visitingFiles.count(filestat->absolute) == 0) {
        proc.includes.emplace_back(filestat->absolute, filestat->mtime);

        sys::io::HandleError err;
        auto contents = readFile(proc.out(), filestat->absolute, err);
        if (err != sys::io::HandleError::OK) {
            proc.setError();
            return;
        }

        auto &dref = proc.contents.emplace_back(std::move(contents));
        proc.name(std::move(filestat->absolute));
        proc.process(std::string_view(dref.data(), dref.size()));
    }
}

void
IncludeHandler::endProcessing(const Preprocessor::ContentContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.processor);

    FileContext &frame = proc.state->stack.top();
    size_t seglen = ctx.data + ctx.size - frame.pos;

    if (seglen > 0) {
        proc.segments.push_back(frame.pos);
        proc.segLengths.push_back(uint32_t(seglen));
    }

    proc.state->stack.pop();

    if (!ctx.name.empty())
        proc.state->visitingFiles.erase(ctx.name);

    if (proc.state->stack.empty()) {
        proc.state.reset();
    }
}

} // namespace glt
