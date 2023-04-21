#include "glt/GLObject.hpp"

#include "glt/utils.hpp"

#include <array>
#include <unordered_map>

#if defined(DEBUG_GLOBJECT) || defined(DEBUG_ALL)
#    define DBG(...) __VA_ARGS__
#else
#    define DBG(...)
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
    std::array<size_t, bl::enum_count_of<ObjectType>()> instance_count{};
    std::array<ObjectKind, bl::enum_count_of<ObjectType>()> kinds{};
    DBG(std::unordered_map<GLuint, std::string> stacktrace_map;)
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
    ASSERT(i == bl::enum_count_of<ObjectType>());
}
} // namespace

void
generate(ObjectType t, GLsizei n, GLuint *names)
{
    DBG({
        ByteStream out;
        err::print_stacktrace(out);
        std::string call_stack_str = out.str();
    });

    auto tab = Tables::get();
    auto idx = static_cast<size_t>(t);
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
generateShader(GLenum shader_type, GLuint *shader)
{
    auto tab = Tables::get();
    GL_ASSIGN_CALL(*shader, glCreateShader, shader_type);
    if (*shader != 0)
        ++tab.instance_count[ObjectType::Shader];
}

void
release(ObjectType t, GLsizei n, const GLuint *names)
{
    auto tab = Tables::get();
    auto idx = static_cast<size_t>(t);
    GL_CALL(tab.kinds[idx].destructor, n, names);
    for (GLsizei i = 0; i < n; ++i) {
        if (names[i] != 0) {
            DBG(sys::io::stdout() << "releasing GLObject, stack: "
                                  << tab.stacktrace_map[names[i]] << "\n");
            --tab.instance_count[idx];
        }
    }
}

void
printStats(sys::io::OutStream &out)
{
    auto tab = Tables::get();
    out << "Active OpenGL Objects:\n";
    for (size_t i = 0; i < bl::enum_count_of<ObjectType>(); ++i)
        if (tab.instance_count[i] > 0)
            out << "  " << tab.kinds[i].kind << "s: " << tab.instance_count[i]
                << "\n";
}

} // namespace glt
