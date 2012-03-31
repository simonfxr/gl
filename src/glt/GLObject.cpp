#include "glt/GLObject.hpp"
#include "glt/utils.hpp"

namespace glt {

using namespace defs;

namespace {

typedef void (*Generator)(GLsizei, GLuint  *);
typedef void (*Destructor)(GLsizei, const GLuint *);

void gen_programs(GLsizei n, GLuint *names) {
    for (GLsizei i = 0; i < n; ++i)
        names[i] = glCreateProgram();
}

void del_programs(GLsizei n, const GLuint *names) {
    for (GLsizei i = 0; i < n; ++i)
        glDeleteProgram(names[i]);
}

void del_shaders(GLsizei n, const GLuint *names) {
    for (GLsizei i = 0; i < n; ++i)
        glDeleteShader(names[i]);
}

size instance_count[ObjectType::Count] = { 0 };

struct ObjectKind {
    Generator generator;
    Destructor destructor;
    const char *kind;
    ObjectKind() {}
    ObjectKind(Generator g, Destructor d, const char *k) :
        generator(g), destructor(d), kind(k) {}
};

// has to initialized after glewInit()
const ObjectKind *kind_table;

void init_table() {
    if (kind_table != 0)
        return;
    
    ObjectKind *table = new ObjectKind[ObjectType::Count];
    kind_table = table;
    int i = 0;
#define KIND(g, d, k) table[ObjectType::k] = ObjectKind(g, d, #k), ++i
    KIND(gen_programs, del_programs, Program);
    KIND(0, del_shaders, Shader);
    KIND(glGenBuffers, glDeleteBuffers, Buffer);
    KIND(glGenFramebuffers, glDeleteFramebuffers, Framebuffer);
    KIND(glGenRenderbuffers, glDeleteRenderbuffers, Renderbuffer);
    KIND(glGenSamplers, glDeleteSamplers, Sampler);
    KIND(glGenTextures, glDeleteTextures, Texture);
    KIND(glGenTransformFeedbacks, glDeleteTransformFeedbacks, TransformFeedback);
    KIND(glGenVertexArrays, glDeleteVertexArrays, VertexArray);
#undef KIND

    ASSERT(i == ObjectType::Count);
}

} // namespace anon

void generate(ObjectType::Type t, GLsizei n, GLuint *names) {
    init_table();
    ASSERT(0 <= t && t < ObjectType::Count);
    ASSERT(kind_table[t].generator != 0);
    GL_CHECK(kind_table[t].generator(n, names));
    for (GLsizei i = 0; i < n; ++i)
        if (names[i] != 0)
            ++instance_count[t];
}

void generateShader(GLenum shader_type, GLuint *name) {
    GL_CHECK(*name = glCreateShader(shader_type));
    if (*name != 0)
        ++instance_count[ObjectType::Shader];
}

void release(ObjectType::Type t, GLsizei n, const GLuint *names) {
    init_table();
    ASSERT(0 <= t && t < ObjectType::Count);
    GL_CHECK(kind_table[t].destructor(n, names));
    for (GLsizei i = 0; i < n; ++i)
        if (names[i] != 0)
            --instance_count[t];
}

void printStats(sys::io::OutStream& out) {
    init_table();
    out << "Active OpenGL Objects:" << sys::io::endl;
    for (index i = 0; i < ObjectType::Count; ++i)
        out << "  " << kind_table[i].kind << "s: " << instance_count[i] << sys::io::endl;
}

} // namespace glt
