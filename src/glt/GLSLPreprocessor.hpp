#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include <string>
#include <vector>

#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "util/Array.hpp"

namespace glt {

struct ProcessingState;

struct ProcessingStateDeleter
{
    void operator()(ProcessingState *) noexcept;
};

struct GLT_API IncludeHandler : public Preprocessor::DirectiveHandler
{
    void beginProcessing(const Preprocessor::ContentContext & /*ctx*/) final;
    void directiveEncountered(
      const Preprocessor::DirectiveContext & /*ctx*/) final;
    void endProcessing(const Preprocessor::ContentContext & /*ctx*/) final;
};

struct GLT_API DependencyHandler : public Preprocessor::DirectiveHandler
{
    void directiveEncountered(
      const Preprocessor::DirectiveContext & /*ctx*/) final;
};

struct GLT_API GLSLPreprocessor : public Preprocessor
{
    const IncludePath &includePath;
    ShaderIncludes &includes;
    ShaderDependencies &dependencies;

    std::vector<uint32_t> segLengths;
    std::vector<const char *> segments;
    std::vector<Array<char>> contents;

    std::unique_ptr<ProcessingState, ProcessingStateDeleter> state;

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    GLSLPreprocessor(const IncludePath &,
                     ShaderIncludes &,
                     ShaderDependencies &);
    ~GLSLPreprocessor();

    void appendString(std::string_view);

    void addDefines(const PreprocessorDefinitions &);

    void processFileRecursively(std::string &&);

    // internal use only
    void advanceSegments(const Preprocessor::DirectiveContext &);
};

} // namespace glt

#endif
