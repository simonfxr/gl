#include "glt/ShaderProgram.hpp"

#include "bl/enumerate.hpp"
#include "bl/range.hpp"
#include "err/err.hpp"
#include "err/log.hpp"
#include "glt/ShaderCompiler.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"
#include "sys/fs.hpp"
#include "sys/measure.hpp"
#include "util/string.hpp"

#include <ctype.h>

#define RAISE_ERR(sender, ec, msg) LOG_RAISE_ERROR(sender, ec, msg)

namespace err {
template<>
struct LogTraits<glt::ShaderProgram>
{
    static auto getDestination(glt::ShaderProgram &x)
    {
        return makeLogSettings(x.shaderManager().out());
    }
};
} // namespace err

namespace glt {

PP_DEF_ENUM_IMPL(GLT_SHADER_PROGRAM_ERROR_ENUM_DEF);

typedef bl::hashtable<bl::string, GLuint> Attributes;

struct ShaderProgram::Data
{
    ShaderProgram &self;
    GLProgramObject program;
    ShaderManager &sm;
    ShaderObjects shaders;
    ShaderRootDependencies rootdeps;
    Attributes attrs;
    bool linked;

    Data(ShaderProgram &owner, ShaderManager &_sm)
      : self(owner), program(0), sm(_sm), linked(false)
    {}

    Data(ShaderProgram &owner, const Data &rhs)
      : self(owner)
      , program()
      , sm(rhs.sm)
      , shaders()
      , rootdeps(rhs.rootdeps)
      , attrs(rhs.attrs)
      , linked(false)
    {}

    bool createProgram();

    void printProgramLog(GLuint program, sys::io::OutStream &out);

    void handleCompileError(ShaderCompilerError /*unused*/);

    void swap(Data &rhs)
    {
        if (this == &rhs)
            return;
        ASSERT(&sm == &rhs.sm);
        using bl::swap;
        bl::swap(program, rhs.program);
        bl::swap(shaders, rhs.shaders);
        bl::swap(rootdeps, rhs.rootdeps);
        bl::swap(attrs, rhs.attrs);
        bl::swap(linked, rhs.linked);
    }
};

DECLARE_PIMPL_DEL(ShaderProgram)

ShaderProgram::ShaderProgram(ShaderManager &sm) : self(new Data(*this, sm)) {}

ShaderProgram::ShaderProgram(const Data &rhs) : self(new Data(*this, rhs)) {}

ShaderProgram::~ShaderProgram()
{
    reset();
}

void
ShaderProgram::reset()
{
    self->program.release();
    self->shaders.clear();
    self->rootdeps.clear();
    self->attrs.clear();
    clearError();
}

bool
ShaderProgram::reload()
{
    ShaderObjects newshaders;
    auto scq = ShaderCompilerQueue(self->sm.shaderCompiler(), newshaders);

    for (const auto &ent : self->rootdeps)
        scq.enqueueReload(self->shaders[ent.key]);

    scq.compileAll();
    if (scq.wasError())
        return false;

    auto unchanged = newshaders.size() == self->shaders.size();
    if (unchanged) {
        for (auto it1 = self->shaders.begin(), it2 = newshaders.begin();
             it1 != self->shaders.end() && it2 != newshaders.end();
             ++it1, ++it2) {
            if (it1->value != it2->value) {
                unchanged = false;
                break;
            }
        }
    }

    if (unchanged)
        return true;

    ShaderProgram new_prog(*this->self);
    new_prog.self->shaders = newshaders;
    if (new_prog.tryLink())
        return replaceWith(new_prog);
    return false;
}

ShaderManager &
ShaderProgram::shaderManager()
{
    return self->sm;
}

GLProgramObject &
ShaderProgram::program()
{
    ASSERT(self->createProgram());
    return self->program;
}

bool
ShaderProgram::Data::createProgram()
{
    program.ensure();
    if (!program.valid()) {
        RAISE_ERR(
          self, ShaderProgramError::OpenGLError, "couldnt create program");
        return false;
    }
    return true;
}

void
ShaderProgram::Data::printProgramLog(GLuint progh, sys::io::OutStream &out)
{
    GLint log_len;
    GL_CALL(glGetProgramiv, progh, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len > 0) {

        bl::string log(log_len);
        GL_CALL(glGetProgramInfoLog, progh, log_len, nullptr, log.data());

        auto logBegin = log.data();
        while (logBegin < log.endp() - 1 && isspace(*logBegin))
            ++logBegin;

        const char *logmsg = log.data();

        if (logBegin == log.data() + log_len - 1) {
            out << "link log empty" << sys::io::endl;
        } else {
            out << "link log: " << sys::io::endl
                << logmsg << sys::io::endl
                << "end link log" << sys::io::endl;
        }
    }
}

void
ShaderProgram::Data::handleCompileError(ShaderCompilerError err)
{
    ERR(string_concat("compile error occurred: ", err)); // FIXME
    self.pushError(ShaderProgramError::CompilationFailed);
}

bool
ShaderProgram::addShaderSrc(const bl::string &src, ShaderType type)
{
    auto scq = ShaderCompilerQueue(self->sm.shaderCompiler(), self->shaders);
    auto source = ShaderSource::makeStringSource(type, src);

    {
        self->rootdeps.insert(source->key(), source);
        scq.enqueueLoad(source);
    }

    scq.compileAll();

    if (scq.wasError()) {
        self->handleCompileError(scq.getError());
        self->rootdeps.erase(source->key());
        return false;
    }

    return true;
}

bool
ShaderProgram::addShaderFile(const bl::string &file0,
                             ShaderType type,
                             bool absolute)
{
    bl::string file = file0;

    if (!absolute) {
        file = sys::fs::lookup(self->sm.shaderDirectories(), file);
        if (file.empty()) {
            RAISE_ERR(*this,
                      ShaderProgramError::FileNotInPath,
                      "couldnt find file in shader directories: " + file);
            return false;
        }
    }

    file = sys::fs::absolutePath(file);
    ASSERT(!file.empty());

    auto scq = ShaderCompilerQueue(self->sm.shaderCompiler(), self->shaders);
    auto source = ShaderSource::makeFileSource(type, file);

    {
        self->rootdeps.insert(source->key(), source);
        scq.enqueueLoad(source);
    }

    scq.compileAll();

    if (scq.wasError()) {
        self->handleCompileError(scq.getError());
        self->rootdeps.erase(source->key());
        return false;
    }

    return true;
}

bool
ShaderProgram::addShaderFilePair(const bl::string &vert_file,
                                 const bl::string &frag_file,
                                 bool absolute)
{
    return addShaderFile(vert_file, ShaderType::VertexShader, absolute) &&
           addShaderFile(frag_file, ShaderType::FragmentShader, absolute);
}

bool
ShaderProgram::addShaderFilePair(const bl::string &basename, bool absolute)
{
    return addShaderFilePair(basename + ".vert", basename + ".frag", absolute);
}

bool
ShaderProgram::tryLink()
{
    return !wasError() && link();
}

bool
ShaderProgram::link()
{

    if (self->shaders.empty()) {
        RAISE_ERR(*this,
                  ShaderProgramError::LinkageFailed,
                  "no shader objects to link");
        return false;
    }

    if (!self->createProgram())
        return false;

    if (self->linked)
        return true;

    for (const auto &attr : self->attrs) {
        GL_CALL(
          glBindAttribLocation, *self->program, attr.value, attr.key.c_str());
        // FIXME: check wether attrib was added correctly
    }

    bl::vector<GLuint> added;
    for (auto &ent : self->shaders) {
        GL_CALL(glAttachShader, *self->program, *ent.value->handle());
        added.push_back(*ent.value->handle());
    }

    bool ok{};
    {
        auto logmsg = err::beginLog(*this, err::LogLevel::Info);
        logmsg << "linking ... ";

        double wct;
        measure_time(wct, glLinkProgram(*self->program));
        GL_CHECK_ERRORS();

        GLint success;
        GL_CALL(glGetProgramiv, *self->program, GL_LINK_STATUS, &success);
        ok = gl_unbool(success);
        logmsg << (ok ? "success" : "failed") << " (" << (wct * 1000) << " ms)"
               << sys::io::endl;

        if (!ok) {
            pushError(ShaderProgramError::LinkageFailed);

            for (size_t i = 0; i < added.size(); ++i)
                GL_CALL(glDetachShader, *self->program, added[size_t(i)]);
        }
    }

    auto logmsg =
      err::beginLog(*this, ok ? err::LogLevel::Info : err::LogLevel::Error);
    if (logmsg)
        self->printProgramLog(*self->program, logmsg.out());

    if (ok)
        self->linked = true;

    return ok;
}

bool
ShaderProgram::bindAttribute(const bl::string &s, GLuint position)
{
    if (self->linked) {
        RAISE_ERR(
          *this, ShaderProgramError::APIError, "program already linked");
        return false;
    }

    self->attrs[s] = position;
    return true;
}

void
ShaderProgram::use()
{
    if (!self->program.valid() || !self->linked) {
        RAISE_ERR(*this, ShaderProgramError::APIError, "program not linked");
        return;
    }

    if (wasError()) {
        auto logmsg = err::beginLog(*this);
        (logmsg << "trying to use program inspite of an error: ")
            .append(getError())
          << sys::io::endl;
    }

    GL_CALL(glUseProgram, *self->program);
}

bool
ShaderProgram::replaceWith(ShaderProgram &new_prog)
{
    if (&new_prog == this)
        return true;
    ASSERT(&new_prog.self->sm == &self->sm);

    if (new_prog.self->program.valid() &&
        new_prog.getError() == ShaderProgramError::NoError) {
        self->swap(*new_prog.self);
        return true;
    }

    auto logmsg = err::beginLog(*this, err::LogLevel::Error);
    (logmsg << "replaceWith failed, the replaceing program has errors: ")
      .append(new_prog.getError());
    return false;
}

GLint
ShaderProgram::uniformLocation(const bl::string &name)
{
    GLint loc;
    GL_ASSIGN_CALL(loc, glGetUniformLocation, *self->program, name.c_str());
    if (loc == -1)
        pushError(ShaderProgramError::UniformNotKnown);
    return loc;
}

bool
ShaderProgram::validate(bool printLogOnError)
{
    bool ok = false;

    if (!self->program.valid()) {
        pushError(ShaderProgramError::APIError);
        goto ret;
    }

    {
        GL_CALL(glValidateProgram, *self->program);
        GLint valid = GL_FALSE;
        GL_CALL(glGetProgramiv, *self->program, GL_VALIDATE_STATUS, &valid);

        if (valid == GL_FALSE)
            pushError(ShaderProgramError::ValidationFailed);
        else
            ok = true;
    }
ret:
    if (!ok && printLogOnError)
        self->printProgramLog(*self->program, shaderManager().out());

    return ok;
}

bool
ShaderProgram::bindAttributes(const StructInfo &si)
{
    for (const auto [i, a] : enumerate(si.fields))
        if (!bindAttribute(a.name, GLuint(i)))
            return false;
    return true;
}

bool
ShaderProgram::bindStreamOutVaryings(bl::array_view<const bl::string> vars)
{

    // FIXME: record vars to set stream out varyings again if relinked

    if (self->linked) {
        RAISE_ERR(*this, ShaderProgramError::APIError, "program alredy linked");
        return false;
    }

    bl::vector<const char *> cvars;
    cvars.reserve(vars.size());
    for (const auto &var : vars)
        cvars.push_back(var.c_str());
    GL_CALL(glTransformFeedbackVaryings,
            *program(),
            GLsizei(vars.size()),
            &cvars[0],
            GL_INTERLEAVED_ATTRIBS);
    return true;
}

} // namespace glt
