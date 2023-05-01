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

#include <bl/box.hpp>
#include <bl/string_view.hpp>

#include <unordered_map>

#define RAISE_ERR(sender, ec, msg) LOG_RAISE_ERROR(sender, ec, msg)

using namespace bl::literals;

template<>
struct err::LogTraits<glt::ShaderProgram>
{
    static auto getDestination(glt::ShaderProgram &x)
    {
        return makeLogSettings(x.shaderManager().out());
    }
};

namespace glt {

using Attributes = std::unordered_map<bl::string, GLuint>;

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

    static void printProgramLog(GLuint progh, sys::io::OutStream out);

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
    auto scq = bl::box<ShaderCompilerQueue>(
      std::in_place, self->sm.shaderCompiler(), newshaders);

    for (const auto &ent : self->rootdeps)
        scq->enqueueReload(self->shaders[ent.first]);

    scq->compileAll();
    if (scq->wasError())
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
          self, ShaderProgramError::OpenGLError, "couldnt create program"_k);
        return false;
    }
    return true;
}

void
ShaderProgram::Data::printProgramLog(GLuint progh, sys::io::OutStream out)
{
    GLint log_len;
    GL_CALL(glGetProgramiv, progh, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len > 0) {

        auto log = bl::vec<char>(static_cast<size_t>(log_len));
        GL_CALL(glGetProgramInfoLog, progh, log_len, nullptr, log.data());

        auto logBegin = log.data();
        while (logBegin < &log[log_len - 1] && isspace(*logBegin))
            ++logBegin;

        auto logmsg = log.data();

        if (logBegin == &log[log_len - 1]) {
            out << "link log empty\n"_k;
        } else {
            out << "link log:\n"_k << bl::from_cstring(logmsg) << "\n"_k
                << "end link log\n"_k;
        }
    }
}

void
ShaderProgram::Data::handleCompileError(ShaderCompilerError err)
{
    ERR(scat("compile error occurred: "_sv, err));
    self.pushError(ShaderProgramError::CompilationFailed);
}

bool
ShaderProgram::addShaderSrc(const bl::string &src, ShaderType type)
{
    auto scq = bl::box<ShaderCompilerQueue>(
      std::in_place, self->sm.shaderCompiler(), self->shaders);
    auto source = ShaderSource::makeStringSource(type, src);

    {
        self->rootdeps.insert(std::make_pair(source->key(), source));
        scq->enqueueLoad(source);
    }

    scq->compileAll();

    if (scq->wasError()) {
        self->handleCompileError(scq->getError());
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
        file = sys::fs::lookup(
          self->sm.shaderDirectories(), file, sys::fs::ObjectType::File);
        if (file.empty()) {
            RAISE_ERR(
              *this,
              ShaderProgramError::FileNotInPath,
              scat("couldnt find file in shader directories: "_sv, file));
            return false;
        }
    }

    file = sys::fs::absolutePath(file);
    ASSERT(!file.empty());

    auto scq = bl::box<ShaderCompilerQueue>(
      std::in_place, self->sm.shaderCompiler(), self->shaders);
    auto source = ShaderSource::makeFileSource(type, file);

    {
        self->rootdeps.insert(std::make_pair(source->key(), source));
        scq->enqueueLoad(source);
    }

    scq->compileAll();

    if (scq->wasError()) {
        self->handleCompileError(scq->getError());
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
    return addShaderFilePair(
      scat(basename, ".vert"_sv), scat(basename, ".frag"_sv), absolute);
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
                  "no shader objects to link"_k);
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

    bl::vec<GLuint> added;
    for (auto it = self->shaders.begin(); it != self->shaders.end(); ++it) {
        GL_CALL(glAttachShader, *self->program, *it->second->handle());
        added.push_back(*it->second->handle());
    }

    bool ok{};
    {
        auto logmsg = err::beginLog(*this, err::LogLevel::Info);
        logmsg << "linking ... "_k;

        double wct;
        measure_time(wct, glLinkProgram(*self->program));
        GL_CHECK_ERRORS();

        GLint success;
        GL_CALL(glGetProgramiv, *self->program, GL_LINK_STATUS, &success);
        ok = gl_unbool(success);
        logmsg << (ok ? "success"_k : "failed"_k) << " ("_k << (wct * 1000)
               << " ms)"_k
               << "\n"_k;

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
ShaderProgram::bindAttribute(bl::string_view s, GLuint position)
{
    if (self->linked) {
        RAISE_ERR(
          *this, ShaderProgramError::APIError, "program already linked"_k);
        return false;
    }

    self->attrs[s] = position;
    return true;
}

void
ShaderProgram::use()
{
    if (!self->program.valid() || !self->linked) {
        RAISE_ERR(*this, ShaderProgramError::APIError, "program not linked"_k);
        return;
    }

    if (wasError()) {
        auto logmsg = err::beginLog(*this);
        (logmsg << "trying to use program inspite of an error: "_k)
            .append(getError())
          << "\n"_k;
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
    (logmsg << "replaceWith failed, the replaceing program has errors: "_k)
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
        if (!bindAttribute(a.name, static_cast<GLuint>(i)))
            return false;
    return true;
}

bool
ShaderProgram::bindStreamOutVaryings(bl::span<const bl::string> vars)
{

    // FIXME: record vars to set stream out varyings again if relinked

    if (self->linked) {
        RAISE_ERR(
          *this, ShaderProgramError::APIError, "program alredy linked"_k);
        return false;
    }

    bl::vec<const char *> cvars;
    cvars.reserve(vars.size());
    for (const auto &var : vars)
        cvars.push_back(var.c_str());
    GL_CALL(glTransformFeedbackVaryings,
            *program(),
            GLsizei(vars.size()),
            cvars.data(),
            GL_INTERLEAVED_ATTRIBS);
    return true;
}

} // namespace glt
