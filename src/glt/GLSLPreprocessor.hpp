#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include <vector>
#include <string>
#include <set>

#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs/fs.hpp"

namespace glt {

struct ProcessingState;

struct IncludeHandler : public Preprocessor::DirectiveHandler {
    void beginProcessing(const Preprocessor::ContentContext&);
    void directiveEncountered(const Preprocessor::DirectiveContext&);
    void endProcessing(const Preprocessor::ContentContext&);
};

struct DependencyHandler : public Preprocessor::DirectiveHandler {
    void directiveEncountered(const Preprocessor::DirectiveContext&);
};

struct GLSLPreprocessor : public Preprocessor {

    const IncludePath& includePath;
    ShaderIncludes& includes;
    ShaderDependencies& dependencies;

    std::vector<uint32> segLengths;
    std::vector<const char *> segments;
    std::vector<char *> contents;

    ProcessingState *state;

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    GLSLPreprocessor(const IncludePath&, ShaderIncludes&, ShaderDependencies&);
    ~GLSLPreprocessor();

    void appendString(const std::string&);
    
    void addDefines(const PreprocessorDefinitions&);

    void processFileRecursively(const std::string&);

    // internal use only
    void advanceSegments(const Preprocessor::DirectiveContext&);
};

} // namespace glt

#endif
