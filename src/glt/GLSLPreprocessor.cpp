#include <cstring>
#include <set>
#include <stack>

#include "glt/GLSLPreprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "sys/io.hpp"

#include "err/err.hpp"

namespace glt {

namespace {

bool
parseFileArg(const Preprocessor::DirectiveContext &ctx,
             const char *&arg,
             uint32 &len)
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
        len = uint32(begin - arg);
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
    std::set<std::string> visitingFiles;
    std::set<std::string> deps;
    std::set<std::string> incs;
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
GLSLPreprocessor::appendString(const std::string &str)
{
    if (!str.empty()) {
        auto data = std::make_unique<char[]>(str.length());
        memcpy(data.get(), str.data(), str.length());
        segments.push_back(data.get());
        contents.push_back(std::move(data));
        segLengths.push_back(uint32(str.length()));
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

    defs::index seglen = SIZE(ctx.content.data + ctx.lineOffset - frame.pos);

    if (seglen > 0) {
        segments.push_back(frame.pos);
        segLengths.push_back(uint32(seglen));
    }

    frame.pos = ctx.content.data + ctx.lineOffset + ctx.lineLength;
}

void
GLSLPreprocessor::processFileRecursively(const std::string &file)
{
    if (wasError())
        return;

    ASSERT(sys::fs::isAbsolute(file));
    char *data;
    defs::size size;

    if (!sys::io::readFile(out(), file, &data, &size)) {
        setError();
        return;
    }

    contents.emplace_back(data);

    this->name(file);
    process(data, size);
}

void
DependencyHandler::directiveEncountered(
  const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.content.processor);

    const char *arg;
    uint32 len;

    // FIXME: add an option to specify ShaderType explicitly

    if (!parseFileArg(ctx, arg, len)) {
        proc.out() << ctx.content.name << ": #need-directive: invalid parameter"
                   << sys::io::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookup(proc.includePath, file);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot find file: " << file
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    ShaderManager::ShaderType stype;
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
          std::make_shared<FileSource>(stype, absPath));
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
    uint32 len;

    if (!parseFileArg(ctx, arg, len) || len == 0) {
        proc.out() << ctx.content.name
                   << ": #include-directive: invalid parameter"
                   << sys::io::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookup(proc.includePath, file);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot find file: " << file
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    sys::fs::Stat filestat;
    if (!sys::fs::stat(realPath, &filestat)) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot open file: " << realPath
                   << sys::io::endl;
        proc.setError();
        return;
    }
    proc.state->incs.insert(filestat.absolute);

    if (proc.state->visitingFiles.count(filestat.absolute) == 0) {
        proc.includes.push_back(
          ShaderInclude(filestat.absolute, filestat.mtime));
        proc.name(filestat.absolute);

        char *contents;
        defs::size size;

        if (!readFile(proc.out(), filestat.absolute, &contents, &size)) {
            proc.setError();
            return;
        }

        proc.contents.emplace_back(contents);
        proc.name(filestat.absolute);
        proc.process(contents, size);
    }
}

void
IncludeHandler::endProcessing(const Preprocessor::ContentContext &ctx)
{
    auto &proc = static_cast<GLSLPreprocessor &>(ctx.processor);

    FileContext &frame = proc.state->stack.top();
    defs::index seglen = SIZE(ctx.data + ctx.size - frame.pos);

    if (seglen > 0) {
        proc.segments.push_back(frame.pos);
        proc.segLengths.push_back(uint32(seglen));
    }

    proc.state->stack.pop();

    if (!ctx.name.empty())
        proc.state->visitingFiles.erase(ctx.name);

    if (proc.state->stack.empty()) {
        proc.state.reset();
    }
}

} // namespace glt
