#ifndef GLT_SHADER_PROGRAM_HPP
#define GLT_SHADER_PROGRAM_HPP

#include <string>
#include <iostream>

#include "opengl.hpp"
#include "data/Ref.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/VertexDescription.hpp"
#include "err/WithError.hpp"

namespace glt {

using namespace defs;

namespace ShaderProgramError {

enum Type {
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

std::string stringError(Type);

}

struct ShaderProgram : public err::WithError<ShaderProgramError::Type,
                                             ShaderProgramError::NoError,
                                             ShaderProgramError::stringError> {

    ShaderProgram(ShaderManager& sm);

    ~ShaderProgram();

    ShaderManager& shaderManager();

    GLuint program();

    bool addShaderSrc(const std::string& src, ShaderManager::ShaderType type);
    
    bool addShaderFile(const std::string& file, ShaderManager::ShaderType type = ShaderManager::GuessShaderType, bool absolute = false);
    
    bool addShaderFilePair(const std::string& vert_file, const std::string& frag_file, bool absolute = false);
    
    bool addShaderFilePair(const std::string& basename, bool absolute = false);

    bool bindAttribute(const std::string& name, GLuint position);
    
    template <typename T>
    bool bindAttributes(const VertexDesc<T>& desc = VertexTraits<T>::description());
    
    bool bindAttributesGeneric(const VertexDescBase& desc);

    bool tryLink();
    
    bool link();

    void use();

    void reset();

    bool reload();

    bool replaceWith(ShaderProgram& new_program);

    GLint uniformLocation(const std::string& name);

    bool validate(bool printLogOnError = true);

private:
    struct Data;
    friend struct Data;
    
    Data * const self;

    ShaderProgram(const ShaderProgram&);
    ShaderProgram& operator =(const ShaderProgram&);
};

template <typename T>
bool ShaderProgram::bindAttributes(const VertexDesc<T>& desc) {
    return bindAttributesGeneric(desc.generic());
}

} // namespace glt

#endif
