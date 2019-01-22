#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include "bl/string.hpp"
#include "bl/vector.hpp"
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

    bl::vector<uint32_t> segLengths;
    bl::vector<const char *> segments;
    bl::vector<bl::string> contents;

    bl::unique_ptr<ProcessingState, ProcessingStateDeleter> state;

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    GLSLPreprocessor(const IncludePath &,
                     ShaderIncludes &,
                     ShaderDependencies &);
    ~GLSLPreprocessor();

    void appendString(bl::string_view);

    void addDefines(const PreprocessorDefinitions &);

    void processFileRecursively(bl::string &&);

    // internal use only
    void advanceSegments(const Preprocessor::DirectiveContext &);
};

} // namespace glt

#endif
