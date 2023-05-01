#include "glt/GLSLPreprocessor.hpp"

#include "err/err.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "sys/io.hpp"

#include <cstring>
#include <stack>
#include <unordered_set>

namespace glt {

using namespace bl::literals;

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

GLSLPreprocessor::GLSLPreprocessor(const IncludePath &incPath,
                                   ShaderIncludes &incs,
                                   ShaderDependencies &deps)
  : includePath(incPath)
  , includes(incs)
  , dependencies(deps)
  , includeHandler()
  , dependencyHandler()
{
    includeHandler.glslPreprocessor = this;
    dependencyHandler.glslPreprocessor = this;
    preprocessor->installHandler("include"_s, includeHandler);
    preprocessor->installHandler("need"_s, dependencyHandler);
}

GLSLPreprocessor::~GLSLPreprocessor() = default;

void
GLSLPreprocessor::appendString(bl::string_view str)
{
    if (!str.empty()) {
        auto &data = contents.emplace_back(str);
        segments.push_back(data.data());
        segLengths.push_back(static_cast<uint32_t>(data.size()));
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
GLSLPreprocessor::advanceSegments(const Preprocessor::DirectiveContext &ctx)
{
    FileContext &frame = state->stack.top();

    size_t seglen = ctx.content.data + ctx.lineOffset - frame.pos;

    if (seglen > 0) {
        segments.push_back(frame.pos);
        segLengths.push_back(static_cast<uint32_t>(seglen));
    }

    frame.pos = ctx.content.data + ctx.lineOffset + ctx.lineLength;
}

void
GLSLPreprocessor::processFileRecursively(bl::string &&file)
{
    if (preprocessor->wasError())
        return;

    ASSERT(sys::fs::isAbsolute(file));
    auto data = sys::io::readFile(file);
    if (!data) {
        preprocessor->setError();
        return;
    }

    auto &dref = contents.emplace_back(bl::move(data).value());

    preprocessor->name(bl::move(file));
    preprocessor->process(bl::string_view(dref.data(), dref.size()));
}

void
DependencyHandler::directiveEncountered(
  const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = *glslPreprocessor->preprocessor;

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
    auto realPath = sys::fs::lookup(
      glslPreprocessor->includePath, file, sys::fs::ObjectType::File);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot find file: "_k << file
                   << "\n"_k;
        proc.setError();
        return;
    }

    glslPreprocessor->advanceSegments(ctx);

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
    if (glslPreprocessor->state->deps.insert(absPath).second)
        glslPreprocessor->dependencies.push_back(
          ShaderSource::makeFileSource(stype, absPath));
}

void
IncludeHandler::beginProcessing(const Preprocessor::ContentContext &ctx)
{
    if (!ctx.name.empty())
        glslPreprocessor->state->visitingFiles.insert(ctx.name);

    FileContext frame{};
    frame.pos = ctx.data;
    glslPreprocessor->state->stack.push(frame);
}

void
IncludeHandler::directiveEncountered(const Preprocessor::DirectiveContext &ctx)
{
    auto &proc = *glslPreprocessor->preprocessor;

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
    auto realPath = sys::fs::lookup(
      glslPreprocessor->includePath, file, sys::fs::ObjectType::File);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot find file: "_k << file
                   << "\n"_k;
        proc.setError();
        return;
    }

    glslPreprocessor->advanceSegments(ctx);

    auto filestat = sys::fs::stat(realPath);
    if (!filestat) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot open file: "_k << realPath
                   << "\n"_k;
        proc.setError();
        return;
    }
    glslPreprocessor->state->incs.insert(filestat->absolute);

    if (!glslPreprocessor->state->visitingFiles.contains(filestat->absolute)) {
        glslPreprocessor->includes.emplace_back(filestat->absolute,
                                                filestat->mtime);

        auto res = sys::io::readFile(filestat->absolute);
        if (!res) {
            proc.setError();
            return;
        }

        auto &dref =
          glslPreprocessor->contents.emplace_back(bl::move(res).value());
        proc.name(bl::move(filestat->absolute));
        proc.process(bl::string_view(dref.data(), dref.size()));
    }
}

void
IncludeHandler::endProcessing(const Preprocessor::ContentContext &ctx)
{
    auto &frame = glslPreprocessor->state->stack.top();
    auto seglen = ctx.data + ctx.size - frame.pos;

    if (seglen > 0) {
        glslPreprocessor->segments.push_back(frame.pos);
        glslPreprocessor->segLengths.push_back(static_cast<uint32_t>(seglen));
    }

    glslPreprocessor->state->stack.pop();

    if (!ctx.name.empty())
        glslPreprocessor->state->visitingFiles.erase(ctx.name);

    if (glslPreprocessor->state->stack.empty())
        *glslPreprocessor->state = ProcessingState{};
}

} // namespace glt
