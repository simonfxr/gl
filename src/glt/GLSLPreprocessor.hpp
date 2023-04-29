#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include <bl/string.hpp>
#include <bl/vec.hpp>

#include "bl/box.hpp"
#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"
#include "util/Array.hpp"

namespace glt {

struct ProcessingState;
struct GLSLPreprocessor;

struct GLT_API IncludeHandler : public Preprocessor::DirectiveHandler
{
    GLSLPreprocessor *glslPreprocessor;
    void beginProcessing(const Preprocessor::ContentContext & /*ctx*/) final;
    void directiveEncountered(
      const Preprocessor::DirectiveContext & /*ctx*/) final;
    void endProcessing(const Preprocessor::ContentContext & /*ctx*/) final;
};

struct GLT_API DependencyHandler : public Preprocessor::DirectiveHandler
{
    GLSLPreprocessor *glslPreprocessor;
    void directiveEncountered(
      const Preprocessor::DirectiveContext & /*ctx*/) final;
};

struct GLT_API GLSLPreprocessor
{
    const IncludePath &includePath;
    ShaderIncludes &includes;
    ShaderDependencies &dependencies;

    bl::vec<uint32_t> segLengths;
    bl::vec<const char *> segments;
    bl::vec<Array<char>> contents;
    bl::box<Preprocessor> preprocessor;

    std::unique_ptr<ProcessingState> state;

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
