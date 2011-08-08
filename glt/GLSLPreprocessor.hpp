#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include <vector>
#include <string>
#include <map>

#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs/fs.hpp"

namespace glt {

struct ProcessingState;

struct FileContents {
    const char *contents;
    uint32 size;

    FileContents(const char *_contents, uint32 _size) :
        contents(_contents), size(_size) {}
};

struct IncludeHandler : public Preprocessor::DirectiveHandler {
    void beginProcessing(const ContentContext&);
    void directiveEncountered(const Preprocessor::DirectiveContext&);
    void endProcessing(const ContentContext&);
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
    std::vector<FileContents> fileContents;

    ProcessingState *state;

    IncludeHandler includeHandler;
    DependencyHandler dependencyHandler;

    GLSLPreprocessor(const IncludePath&, ShaderIncludes&, ShaderDependencies&);
    ~GLSLPreprocessor();

    void appendString(const std::string&);
    
    void addDefines(const PreprocessorDefinitions&);

    bool processRecursively(const std::string&);
    bool processRecursively(const char *contents, uint32 size);
    bool processFileRecursively(const std::string&, std::string *, sys::fs::MTime *);

    // internal use only
    void advanceSegments(const Preprocessor::DirectiveContext&);
};

} // namespace glt

#endif
