#include "glt/GLObject.hpp"
#include "glt/utils.hpp"

#include <unordered_map>

#if defined(DEBUG_GLOBJECT) || defined(DEBUG_ALL)
#define DBG(...) __VA_ARGS__
#else
#define DBG(...)
#endif

namespace glt {

namespace {

using Generator = PFNGLGENBUFFERSPROC;
using Destructor = PFNGLDELETEBUFFERSPROC;

void APIENTRY
gen_programs(GLsizei n, GLuint *names)
{
    for (GLsizei i = 0; i < n; ++i)
        names[i] = glCreateProgram();
}

void APIENTRY
del_programs(GLsizei n, const GLuint *names)
{
    for (GLsizei i = 0; i < n; ++i)
        glDeleteProgram(names[i]);
}

void APIENTRY
del_shaders(GLsizei n, const GLuint *names)
{
    for (GLsizei i = 0; i < n; ++i)
        glDeleteShader(names[i]);
}

size_t instance_count[ObjectType::Count] = {};

struct ObjectKind
{
    Generator generator{};
    Destructor destructor{};
    const char *kind{};
    ObjectKind() = default;
    ObjectKind(Generator g, Destructor d, const char *k)
      : generator(g), destructor(d), kind(k)
    {}
};

// has to initialized after glewInit()
const ObjectKind *kind_table;

DBG(std::unordered_map<GLuint, std::string> *stacktrace_map;)

void
init_table()
{
    if (kind_table != nullptr)
        return;

    DBG(stacktrace_map = new std::map<GLuint, std::string>());

    auto *table = new ObjectKind[ObjectType::Count];
    kind_table = table;
    int i = 0;
#define KIND(g, d, k) (table[ObjectType::k] = ObjectKind(g, d, #k)), ++i
    KIND(gen_programs, del_programs, Program);
    KIND(nullptr, del_shaders, Shader);
    KIND(glGenBuffers, glDeleteBuffers, Buffer);
    KIND(glGenFramebuffers, glDeleteFramebuffers, Framebuffer);
    KIND(glGenRenderbuffers, glDeleteRenderbuffers, Renderbuffer);
    KIND(glGenSamplers, glDeleteSamplers, Sampler);
    KIND(glGenTextures, glDeleteTextures, Texture);
    KIND(
      glGenTransformFeedbacks, glDeleteTransformFeedbacks, TransformFeedback);
    KIND(glGenVertexArrays, glDeleteVertexArrays, VertexArray);
    KIND(glGenQueries, glDeleteQueries, Query);
#undef KIND

    ASSERT(i == ObjectType::Count);
}

} // namespace

void
generate(ObjectType::Type t, GLsizei n, GLuint *names)
{
    init_table();

    DBG({
        ByteStream out;
        err::print_stacktrace(out);
        std::string call_stack_str = out.str();
    });

    ASSERT(0 <= t && t < ObjectType::Count);
    ASSERT(kind_table[t].generator != nullptr);
    GL_CALL(kind_table[t].generator, n, names);
    for (GLsizei i = 0; i < n; ++i)
        if (names[i] != 0) {
            ++instance_count[t];
            DBG((*stacktrace_map)[names[i]] = call_stack_str);
        }
}

void
generateShader(GLenum shader_type, GLuint *name)
{
    GL_ASSIGN_CALL(*name, glCreateShader, shader_type);
    if (*name != 0)
        ++instance_count[ObjectType::Shader];
}

void
release(ObjectType::Type t, GLsizei n, const GLuint *names)
{
    init_table();
    ASSERT(0 <= t && t < ObjectType::Count);
    GL_CALL(kind_table[t].destructor, n, names);
    for (GLsizei i = 0; i < n; ++i)
        if (names[i] != 0) {
            DBG(sys::io::stdout()
                << "releasing GLObject, stack: " << (*stacktrace_map)[names[i]]
                << sys::io::endl);
            --instance_count[t];
        }
}

void
printStats(sys::io::OutStream &out)
{
    init_table();
    out << "Active OpenGL Objects:" << sys::io::endl;
    for (size_t i = 0; i < ObjectType::Count; ++i)
        if (instance_count[i] > 0)
            out << "  " << kind_table[i].kind << "s: " << instance_count[i]
                << sys::io::endl;
}

} // namespace glt
