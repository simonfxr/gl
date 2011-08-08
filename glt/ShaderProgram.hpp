#ifndef GLT_SHADER_PROGRAM_HPP
#define GLT_SHADER_PROGRAM_HPP

#include <string>
#include <iostream>

#include "opengl.h"
#include "data/Ref.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/VertexDescription.hpp"

namespace glt {

template <typename T> struct VertexDesc;


struct ShaderProgram {

    enum Error {
        NoError,
        CompilationFailed,
        LinkageFailed,
        AttributeNotBound,
        UniformNotKnown,
        ValidationFailed,
        APIError,
        OpenGLError
    };

    ShaderProgram(ShaderManager& sm);
    ShaderProgram(const ShaderProgram&);
    
    ~ShaderProgram();

    ShaderProgram& operator =(const ShaderProgram&);

    GLuint program();

    bool addShaderSrc(ShaderType type, const std::string& src);
    
    bool addShaderFile(ShaderType type, const std::string& file, bool absolute = false);
    
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

    Error clearError();

    bool wasError();

    bool replaceWith(ShaderProgram& new_program);

    GLint uniformLocation(const std::string& name);

    void printError();

    bool validate(bool printLogOnError = true);

private:

    struct Data;
    friend struct Data;
    
    Data * const self;
};

Ref<ShaderManager::CachedShaderObject> rebuildShaderObject(ShaderManager& self, Ref<ShaderManager::CachedShaderObject>& so);

template <typename T>
bool ShaderProgram::bindAttributes(const VertexDesc<T>& desc) {
    return bindAttributesGeneric(desc.generic());
}

} // namespace glt

#endif
