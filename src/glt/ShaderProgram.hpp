#ifndef GLT_SHADER_PROGRAM_HPP
#define GLT_SHADER_PROGRAM_HPP

#include <string>

#include "err/WithError.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/conf.hpp"
#include "glt/type_info.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"

#include <span>

namespace glt {

#define GLT_SHADER_PROGRAM_ERROR_ENUM_DEF(T, V0, V)                            \
    T(ShaderProgramError,                                                      \
      uint8_t,                                                                 \
      V0(NoError) V(FileNotInPath) V(CompilationFailed) V(LinkageFailed)       \
        V(AttributeNotBound) V(UniformNotKnown) V(ValidationFailed)            \
          V(APIError) V(OpenGLError))

PP_DEF_ENUM_WITH_API(GLT_API, GLT_SHADER_PROGRAM_ERROR_ENUM_DEF);

struct GLT_API ShaderProgram
  : public std::enable_shared_from_this<ShaderProgram>
  , public err::WithError<ShaderProgramError>
{
    ShaderProgram(ShaderManager &sm);
    ~ShaderProgram();

    ShaderManager &shaderManager();

    GLProgramObject &program();

    bool addShaderSrc(const std::string &src, ShaderType type);

    bool addShaderFile(const std::string &file,
                       ShaderType type = ShaderType::GuessShaderType,
                       bool absolute = false);

    bool addShaderFilePair(const std::string &vert_file,
                           const std::string &frag_file,
                           bool absolute = false);

    bool addShaderFilePair(const std::string &basename, bool absolute = false);

    bool bindAttribute(const std::string &, GLuint position);

    bool bindStreamOutVaryings(std::span<const std::string>);

    bool bindAttributes(const StructInfo &);

    bool tryLink();

    bool link();

    void use();

    void reset();

    bool reload();

    bool replaceWith(ShaderProgram &new_program);

    GLint uniformLocation(const std::string &name);

    bool validate(bool printLogOnError = true);

    std::shared_ptr<ShaderProgram> get_shared_ptr()
    {
        return shared_from_this();
    }

private:
    DECLARE_PIMPL(GLT_API, self);
    ShaderProgram(const Data &);
};

using ShaderProgramRef = std::shared_ptr<ShaderProgram>;

} // namespace glt

#endif
