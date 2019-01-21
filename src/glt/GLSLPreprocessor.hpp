#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include <string>
#include <vector>

#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"

namespace glt {

struct ProcessingState;

struct ProcessingStateDeleter
{
    void operator()(ProcessingState *) noexcept;
};

struct GLT_API IncludeHandler : public Preprocessor::DirectiveHandler
{
    virtual void beginProcessing(
      const Preprocessor::ContentContext &) final override;
    virtual void directiveEncountered(
      const Preprocessor::DirectiveContext &) final override;
    virtual void endProcessing(
      const Preprocessor::ContentContext &) final override;
};

struct GLT_API DependencyHandler : public Preprocessor::DirectiveHandler
{
    virtual void directiveEncountered(
      const Preprocessor::DirectiveContext &) final override;
};

struct GLT_API GLSLPreprocessor : public Preprocessor
{

    const IncludePath &includePath;
    ShaderIncludes &includes;
    ShaderDependencies &dependencies;

    std::vector<uint32_t> segLengths;
    std::vector<const char *> segments;
    std::vector<std::string> contents;

    std::unique_ptr<ProcessingState, ProcessingStateDeleter> state;

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    GLSLPreprocessor(const IncludePath &,
                     ShaderIncludes &,
                     ShaderDependencies &);
    ~GLSLPreprocessor();

    void appendString(const std::string &);

    void addDefines(const PreprocessorDefinitions &);

    void processFileRecursively(const std::string &);

    // internal use only
    void advanceSegments(const Preprocessor::DirectiveContext &);
};

} // namespace glt

#endif
