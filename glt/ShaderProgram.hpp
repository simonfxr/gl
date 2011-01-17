#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#include <string>
#include <ostream>
#include <GL/gl.h>

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
        FragmentShader
    };

    ShaderProgram(ShaderManager& sm);
    ~ShaderProgram();

    GLuint program();

    bool addShaderSrc(ShaderType type, const std::string& src);
    bool addShaderFile(ShaderType type, const std::string& file);

    bool bindAttribute(const std::string& name, GLuint position);

    bool tryLink();
    bool link();

    void use();

    void reset();

    Error clearError();

    bool wasError();

    bool replaceWith(ShaderProgram& new_program);

    GLint uniformLocation(const std::string& name);

    void printError(std::ostream& out);

    bool validate(bool printLogOnError = false);

    // static void printShaderLog(GLuint shader, std::ostream& out);
    // static void printProgramLog(GLuint program, std::ostream& out);
    // static void printProgramLog(const ShaderProgram& prog, std::ostream& out);

private:

    struct Data;
    friend struct Data;
    
    Data * const self;
    
    ShaderProgram(const ShaderProgram& _);
    ShaderProgram& operator =(const ShaderProgram& _);
};

} // namespace glt

#endif
