#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include "glt/Preprocessor.hpp"
#include "glt/ShaderCompiler.hpp"

#include <bl/box.hpp>
#include <bl/ptr.hpp>
#include <bl/string.hpp>
#include <bl/vec.hpp>

namespace glt {

struct GLT_API GLSLPreprocessor : bl::runtime_sized_base<GLSLPreprocessor>
{
    static GLSLPreprocessor &construct_at(void *p,
                                          bl::ptr<Preprocessor>,
                                          const IncludePath &,
                                          ShaderIncludes &,
                                          ShaderDependencies &) noexcept;
    void appendString(bl::string_view);

    void addDefines(const PreprocessorDefinitions &);

    void processFileRecursively(bl::string &&);

    // internal use only
    void advanceSegments(const Preprocessor::DirectiveContext &);

    [[nodiscard]] const bl::vec<const char *> &segments() const noexcept;

    [[nodiscard]] const bl::vec<uint32_t> &segLengths() const noexcept;

    BL_DECLARE_RUNTIME_SIZED(GLSLPreprocessor, Impl);
};

} // namespace glt

#endif
