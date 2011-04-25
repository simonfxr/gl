#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#include <string>
#include <iostream>

#include "opengl.h"

namespace glt {

struct ShaderManager;

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

    enum ShaderType {
        VertexShader,
        FragmentShader,
        GeometryShader
    };

    ShaderProgram(ShaderManager& sm);
    ~ShaderProgram();

    GLuint program();

    bool addShaderSrc(ShaderType type, const std::string& src);
    bool addShaderFile(ShaderType type, const std::string& file);
    bool addShaderFile(const std::string& file);
    bool addShaderFilePair(const std::string& vert_file, const std::string& frag_file);
    bool addShaderFilePair(const std::string& basename);

    bool bindAttribute(const std::string& name, GLuint position);

    bool tryLink();
    bool link();

    void use();

    void reset();

    Error clearError();

    bool wasError();

    bool replaceWith(ShaderProgram& new_program);

    GLint uniformLocation(const std::string& name);

    void printError(std::ostream& out = std::cerr);

    bool validate(bool printLogOnError = false);

private:

    struct Data;
    friend struct Data;
    
    Data * const self;
    
    ShaderProgram(const ShaderProgram& _);
    ShaderProgram& operator =(const ShaderProgram& _);
};

} // namespace glt

#endif
