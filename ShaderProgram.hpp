#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#include <string>
#include <ostream>
#include <GL/gl.h>

struct ShaderProgram {

    ShaderProgram();
    ~ShaderProgram();

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

private:

    struct Impl;
    friend struct Impl;
    
    Error last_error;
    
public:    
    GLuint program;

private:

    void push_error(Error err) {
        if (last_error == NoError)
            last_error = err;
    }

public:    

    bool addShaderSrc(ShaderType type, const std::string& src);
    bool addShaderFile(ShaderType type, const std::string& file);

    bool bindAttribute(const std::string& name, GLuint position);

    bool link();

    void use();

    void reset();

    Error getError() {
        Error err = last_error;
        last_error = NoError;
        return err;
    }
    
    void clearError() {
        last_error = NoError;
    }

    bool wasError() {
        return last_error != NoError;
    }

    bool replaceWith(ShaderProgram& new_program);

    GLint uniformLocation(const std::string& name);

    void printError(std::ostream& out);

    bool validate(bool printLogOnError = false);

    static void printShaderLog(GLuint shader, std::ostream& out);
    static void printProgramLog(GLuint program, std::ostream& out);
    static void printProgramLog(const ShaderProgram& prog, std::ostream& out);

private:
    ShaderProgram(const ShaderProgram& _);
    ShaderProgram& operator =(const ShaderProgram& _);
};

#endif
