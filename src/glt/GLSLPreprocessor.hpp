#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include "bl/string.hpp"
#include "bl/vector.hpp"
#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "pp/pimpl.hpp"
#include "sys/fs.hpp"

namespace glt {

struct GLT_API GLSLPreprocessor : public Preprocessor
{
    // OpenGL's glShaderSource expects two arrays of strings and their lengths.
    // To avoid concatenation we store it in this way from the beginning

    bl::vector<uint32_t> segLengths;

    // pointers to data inside contents
    bl::vector<const char *> segments;

    // owned source code data
    bl::vector<bl::string> contents;

    GLSLPreprocessor(const IncludePath &,
                     ShaderIncludes &,
                     ShaderDependencies &);
    ~GLSLPreprocessor();

    void appendString(bl::string);

    void addDefines(const PreprocessorDefinitions &);

    void processFileRecursively(bl::string &&);

private:
    void advanceSegments(const Preprocessor::DirectiveContext &);

    struct IncludeHandler;
    friend struct IncludeHandler;

    struct DependencyHandler;
    friend struct DependencyHandler;

    DECLARE_PIMPL(GLT_API, self);
};

} // namespace glt

#endif
