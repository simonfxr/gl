#include "glt/GLObject.hpp"

#include "bl/hashtable.hpp"
#include "glt/utils.hpp"

#include "opengl.hpp"

#if defined(DEBUG_GLOBJECT) || defined(DEBUG_ALL)
#    define DBG(...) __VA_ARGS__
#else
#    define DBG(...)
#endif

namespace glt {

PP_DEF_ENUM_IMPL(GLT_OBJECT_TYPE_ENUM_DEF)

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

struct Tables
{
    size_t instance_count[ObjectType::count]{};
    ObjectKind kinds[ObjectType::count];
    DBG(bl::hashtable<GLuint, bl::string> stacktrace_map;)
    Tables();

    static Tables &get() noexcept
    {
        static Tables tables;
        return tables;
    }
};

Tables::Tables()
{
    int i = 0;
#define KIND(g, d, k) (kinds[ObjectType::k] = ObjectKind(g, d, #k)), ++i
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
    ASSERT(i == ObjectType::count);
}
} // namespace

void
generate(ObjectType t, GLsizei n, GLuint *names)
{
    DBG({
        ByteStream out;
        err::print_stacktrace(out);
        bl::string call_stack_str = out.str();
    });

    auto tab = Tables::get();
    ASSERT(t.is_valid());
    auto idx = size_t(t.value);
    ASSERT(tab.kinds[idx].generator != nullptr);
    GL_CALL(tab.kinds[idx].generator, n, names);
    for (GLsizei i = 0; i < n; ++i) {
        if (names[i] != 0) {
            ++tab.instance_count[idx];
            DBG(tab.stacktrace_map[names[idx]] = call_stack_str);
        }
    }
}

void
generateShader(GLenum shader_type, GLuint *name)
{
    auto tab = Tables::get();
    GL_ASSIGN_CALL(*name, glCreateShader, shader_type);
    if (*name != 0)
        ++tab.instance_count[ObjectType::Shader];
}

void
release(ObjectType t, GLsizei n, const GLuint *names)
{
    auto tab = Tables::get();
    ASSERT(t.is_valid());
    auto idx = size_t(t.value);
    GL_CALL(tab.kinds[idx].destructor, n, names);
    for (GLsizei i = 0; i < n; ++i) {
        if (names[i] != 0) {
            DBG(sys::io::stdout()
                << "releasing GLObject, stack: " << tab.stacktrace_map[names[i]]
                << sys::io::endl);
            --tab.instance_count[idx];
        }
    }
}

void
printStats(sys::io::OutStream &out)
{
    auto tab = Tables::get();
    out << "Active OpenGL Objects:" << sys::io::endl;
    for (size_t i = 0; i < ObjectType::count; ++i)
        if (tab.instance_count[i] > 0)
            out << "  " << tab.kinds[i].kind << "s: " << tab.instance_count[i]
                << sys::io::endl;
}

} // namespace glt
