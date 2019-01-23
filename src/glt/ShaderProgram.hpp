#ifndef GLT_SHADER_PROGRAM_HPP
#define GLT_SHADER_PROGRAM_HPP

#include "bl/array_view.hpp"
#include "bl/shared_ptr.hpp"
#include "bl/string.hpp"
#include "err/WithError.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/conf.hpp"
#include "glt/type_info.hpp"
#include "opengl_fwd.hpp"
#include "pp/enum.hpp"

namespace glt {

#define GLT_SHADER_PROGRAM_ERROR_ENUM_DEF(T, V0, V)                            \
    T(ShaderProgramError,                                                      \
      uint8_t,                                                                 \
      V0(NoError) V(FileNotInPath) V(CompilationFailed) V(LinkageFailed)       \
        V(AttributeNotBound) V(UniformNotKnown) V(ValidationFailed)            \
          V(APIError) V(OpenGLError))

PP_DEF_ENUM_WITH_API(GLT_API, GLT_SHADER_PROGRAM_ERROR_ENUM_DEF);

struct GLT_API ShaderProgram
  : public bl::enable_shared_from_this<ShaderProgram>
  , public err::WithError<ShaderProgramError>
{
    ShaderProgram(ShaderManager &sm);
    ~ShaderProgram();

    ShaderManager &shaderManager();

    GLProgramObject &program();

    bool addShaderSrc(const bl::string &src, ShaderType type);

    bool addShaderFile(const bl::string &file,
                       ShaderType type = ShaderType::GuessShaderType,
                       bool absolute = false);

    bool addShaderFilePair(const bl::string &vert_file,
                           const bl::string &frag_file,
                           bool absolute = false);

    bool addShaderFilePair(const bl::string &basename, bool absolute = false);

    bool bindAttribute(const bl::string &, GLuint position);

    bool bindStreamOutVaryings(bl::array_view<const bl::string>);

    bool bindAttributes(const StructInfo &);

    bool tryLink();

    bool link();

    void use();

    void reset();

    bool reload();

    bool replaceWith(ShaderProgram &new_program);

    GLint uniformLocation(const bl::string &name);

    bool validate(bool printLogOnError = true);

    bl::shared_ptr<ShaderProgram> get_shared_ptr()
    {
        return shared_from_this();
    }

private:
    DECLARE_PIMPL(GLT_API, self);
    ShaderProgram(const Data &);
};

using ShaderProgramRef = bl::shared_ptr<ShaderProgram>;

} // namespace glt

#endif
