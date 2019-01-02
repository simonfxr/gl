#ifndef GLT_SHADER_PROGRAM_HPP
#define GLT_SHADER_PROGRAM_HPP

#include <string>

#include "data/Array.hpp"
#include "err/WithError.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/VertexDescription.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"

namespace glt {

using namespace defs;

namespace ShaderProgramError {

enum Type
{
    NoError,
    FileNotInPath,
    CompilationFailed,
    LinkageFailed,
    AttributeNotBound,
    UniformNotKnown,
    ValidationFailed,
    APIError,
    OpenGLError
};

std::string GLT_API stringError(Type);

} // namespace ShaderProgramError

struct GLT_API ShaderProgram
  : public err::WithError<ShaderProgramError::Type,
                          ShaderProgramError::NoError,
                          ShaderProgramError::stringError>
{

    ShaderProgram(ShaderManager &sm);

    ShaderManager &shaderManager();

    GLProgramObject &program();

    bool addShaderSrc(const std::string &src, ShaderManager::ShaderType type);

    bool addShaderFile(
      const std::string &file,
      ShaderManager::ShaderType type = ShaderManager::GuessShaderType,
      bool absolute = false);

    bool addShaderFilePair(const std::string &vert_file,
                           const std::string &frag_file,
                           bool absolute = false);

    bool addShaderFilePair(const std::string &basename, bool absolute = false);

    bool bindAttribute(const std::string &s, GLuint position);

    bool bindStreamOutVaryings(const Array<std::string> &);

    template<typename T>
    bool bindAttributes(const VertexDescription<T> &desc);

    template<typename T>
    bool bindAttributes()
    {
        return bindAttributes(T::gl::desc);
    }

    bool bindAttributesGeneric(const GenVertexDescription &desc);

    bool tryLink();

    bool link();

    void use();

    void reset();

    bool reload();

    bool replaceWith(ShaderProgram &new_program);

    GLint uniformLocation(const std::string &name);

    bool validate(bool printLogOnError = true);

private:
    struct Data;
    friend struct Data;

    struct DataDeleter
    {
        void operator()(Data *p) noexcept;
    };

    const std::unique_ptr<Data, DataDeleter> self;
    ShaderProgram(const ShaderProgram &);
};

using ShaderProgramRef = std::shared_ptr<ShaderProgram>;

template<typename T>
bool
ShaderProgram::bindAttributes(const VertexDescription<T> &desc)
{
    return bindAttributesGeneric(desc.cast_gen());
}

} // namespace glt

#endif
