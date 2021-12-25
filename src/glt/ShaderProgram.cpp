#include "glt/ShaderProgram.hpp"

#include "err/err.hpp"
#include "err/log.hpp"
#include "glt/ShaderCompiler.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"
#include "sys/fs.hpp"
#include "sys/measure.hpp"
#include "util/range.hpp"
#include "util/string.hpp"

#include <unordered_map>

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

using Attributes = std::unordered_map<std::string, GLuint>;

struct ShaderProgram::Data
{
    ShaderProgram &self;
    ShaderManager &sm;
    ShaderObjects shaders;
    ShaderRootDependencies rootdeps;
    Attributes attrs;
    GLProgramObject program{ 0 };
    bool linked{ false };

    Data(ShaderProgram &owner, ShaderManager &_sm) : self(owner), sm(_sm) {}

    Data(ShaderProgram &owner, const Data &rhs)
      : self(owner), sm(rhs.sm), rootdeps(rhs.rootdeps), attrs(rhs.attrs)
    {}

    bool createProgram();

    static void printProgramLog(GLuint progh, sys::io::OutStream &out);

    void handleCompileError(ShaderCompilerError /*unused*/);

    void swap(Data &rhs)
    {
        if (this == &rhs)
            return;
        ASSERT(&sm == &rhs.sm);
        using std::swap;
        swap(program, rhs.program);
        swap(shaders, rhs.shaders);
        swap(rootdeps, rhs.rootdeps);
        swap(attrs, rhs.attrs);
        swap(linked, rhs.linked);
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
        scq.enqueueReload(self->shaders[ent.first]);

    scq.compileAll();
    if (scq.wasError())
        return false;

    auto unchanged = newshaders.size() == self->shaders.size();
    if (unchanged) {
        for (auto it1 = self->shaders.begin(), it2 = newshaders.begin();
             it1 != self->shaders.end() && it2 != newshaders.end();
             ++it1, ++it2) {
            if (it1->second != it2->second) {
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

        auto log = std::vector<char>(size_t(log_len));
        GL_CALL(glGetProgramInfoLog, progh, log_len, nullptr, log.data());

        auto logBegin = log.data();
        while (logBegin < &log[log_len - 1] && isspace(*logBegin))
            ++logBegin;

        auto logmsg = log.data();

        if (logBegin == &log[log_len - 1]) {
            out << "link log empty\n";
        } else {
            out << "link log:\n"
                << logmsg << "\n"
                << "end link log\n";
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
ShaderProgram::addShaderSrc(const std::string &src, ShaderType type)
{
    auto scq = ShaderCompilerQueue(self->sm.shaderCompiler(), self->shaders);
    auto source = ShaderSource::makeStringSource(type, src);

    {
        self->rootdeps.insert(std::make_pair(source->key(), source));
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
ShaderProgram::addShaderFile(const std::string &file0,
                             ShaderType type,
                             bool absolute)
{
    std::string file = file0;

    if (!absolute) {
        file = sys::fs::lookup(view_array(self->sm.shaderDirectories()), file);
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
        self->rootdeps.insert(std::make_pair(source->key(), source));
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
ShaderProgram::addShaderFilePair(const std::string &vert_file,
                                 const std::string &frag_file,
                                 bool absolute)
{
    return addShaderFile(vert_file, ShaderType::VertexShader, absolute) &&
           addShaderFile(frag_file, ShaderType::FragmentShader, absolute);
}

bool
ShaderProgram::addShaderFilePair(const std::string &basename, bool absolute)
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

    for (auto it = self->attrs.begin(); it != self->attrs.end(); ++it) {
        GL_CALL(
          glBindAttribLocation, *self->program, it->second, it->first.c_str());
        // FIXME: check wether attrib was added correctly
    }

    std::vector<GLuint> added;
    for (auto it = self->shaders.begin(); it != self->shaders.end(); ++it) {
        GL_CALL(glAttachShader, *self->program, *it->second->handle());
        added.push_back(*it->second->handle());
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
               << "\n";

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
ShaderProgram::bindAttribute(const std::string &s, GLuint position)
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
          << "\n";
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
ShaderProgram::uniformLocation(const std::string &name)
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
ShaderProgram::bindStreamOutVaryings(ArrayView<const std::string> vars)
{

    // FIXME: record vars to set stream out varyings again if relinked

    if (self->linked) {
        RAISE_ERR(*this, ShaderProgramError::APIError, "program alredy linked");
        return false;
    }

    std::vector<const char *> cvars;
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
