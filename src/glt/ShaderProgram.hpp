#ifndef GLT_SHADER_PROGRAM_HPP
#define GLT_SHADER_PROGRAM_HPP

#include <bl/string.hpp>

#include "err/WithError.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/conf.hpp"
#include "glt/type_info.hpp"
#include "opengl.hpp"
#include "pp/enum.hpp"
#include "pp/pimpl.hpp"

#include <bl/span.hpp>

namespace glt {

PP_DEF_ENUM(ShaderProgramError,
            uint8_t,
            NoError,
            FileNotInPath,
            CompilationFailed,
            LinkageFailed,
            AttributeNotBound,
            UniformNotKnown,
            ValidationFailed,
            APIError,
            OpenGLError);

struct GLT_API ShaderProgram
  : public std::enable_shared_from_this<ShaderProgram>
  , public err::WithError<ShaderProgramError>
{
    explicit ShaderProgram(ShaderManager &sm);
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

    bool bindStreamOutVaryings(bl::span<const bl::string>);

    bool bindAttributes(const StructInfo &);

    template<typename VertexType>
    bool bindAttributes()
    {
        return bindAttributes(VertexType::gl::struct_info::info);
    }

    bool tryLink();

    bool link();

    void use();

    void reset();

    bool reload();

    bool replaceWith(ShaderProgram &new_program);

    GLint uniformLocation(const bl::string &name);

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
