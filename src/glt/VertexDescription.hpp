#ifndef VERTEX_DESCRIPTION_HPP
#define VERTEX_DESCRIPTION_HPP

#include "err/err.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"

#include "glt/color.hpp"
#include "math/glvec/type.hpp"

namespace glt {

template<typename T>
struct GLType
{
    typedef typename T::gl type;
};

template<>
struct GLType<math::real>
{
    typedef float type;
};

template<>
struct GLType<color>
{
    typedef color type;
};

struct VertexAttr
{
    defs::index offset;
    defs::size alignment;
    GLenum component_type;
    defs::size ncomponents;
    bool normalized;
    const char *name;
};

struct Any;

template<typename T>
struct VertexDescription
{
    typedef void (*store_function)(unsigned char *dest,
                                   const unsigned char *src);

    defs::size sizeof_vertex;
    defs::size alignment;
    defs::size nattributes;
    const char *name;
    const VertexAttr *attributes;
    store_function store;

    const VertexDescription<Any> &cast_gen() const
    {
        return *reinterpret_cast<const VertexDescription<Any> *>(this);
    }
};

typedef VertexDescription<Any> GenVertexDescription;

template<typename T>
struct VertexAttrInfo
{
    //    static const defs::size alignment;
    //    static const GLenum component_type;
    //    static const defs::size ncomponents;
    //    static const bool normalized;
};

template<>
struct VertexAttrInfo<float>
{
    static const defs::index alignment = alignof(float);
    static const GLenum component_type = GL_FLOAT;
    static const defs::size ncomponents = 1;
    static const bool normalized = false;
};

template<>
struct VertexAttrInfo<color>
{
    static const defs::index alignment = alignof(color);
    static const GLenum component_type = GL_UNSIGNED_BYTE;
    static const defs::size ncomponents = 4;
    static const bool normalized = true;
};

#define VD_DEF_GLVEC_ATTR_INFO(t)                                              \
    template<>                                                                 \
    struct VertexAttrInfo<t::gl>                                               \
    {                                                                          \
        static const defs::index alignment = alignof(t::gl);                   \
        static const GLenum component_type = GL_FLOAT;                         \
        static const defs::size ncomponents = t::size;                         \
        static const bool normalized = false;                                  \
    }

VD_DEF_GLVEC_ATTR_INFO(math::vec2_t);
VD_DEF_GLVEC_ATTR_INFO(math::vec3_t);
VD_DEF_GLVEC_ATTR_INFO(math::vec4_t);

VD_DEF_GLVEC_ATTR_INFO(math::mat2_t);
VD_DEF_GLVEC_ATTR_INFO(math::mat3_t);
VD_DEF_GLVEC_ATTR_INFO(math::mat4_t);

#define VD_GL_NS(name) CONCAT(name, _gl)

#define VD_STRUCT_V(name, ...)                                                 \
    namespace VD_GL_NS(name) { struct gl; }                                    \
    struct name                                                                \
    {                                                                          \
        typedef VD_GL_NS(name)::gl gl;                                         \
        __VA_ARGS__ /* fields */                                               \
        name() = default;                                                      \
        inline explicit name(const gl &);                                      \
    };                                                                         \
    namespace VD_GL_NS(name)                                                   \
    {
#define VD_STRUCT_F(t, n, ...)                                                 \
    t n;                                                                       \
    __VA_ARGS__
#define VD_STRUCT_Z(t, n) t n;

#define VD_STRUCT_CONSTR_V(name, ...)                                          \
    inline name::name(const name::gl &_gl_vertex) : __VA_ARGS__ {}
#define VD_STRUCT_CONSTR_F(t, n, ...) VD_STRUCT_CONSTR_Z(t, n), __VA_ARGS__
#define VD_STRUCT_CONSTR_Z(t, n) n(_gl_vertex.n)

#define VD_GL_STRUCT_BEGIN_V(name, ...)                                        \
    struct gl                                                                  \
    {                                                                          \
        __VA_ARGS__ /* fields */                                               \
        gl() = default;                                                        \
        gl(const name &_vertex) :
#define VD_GL_STRUCT_BEGIN_F(t, n, ...) VD_GL_STRUCT_BEGIN_Z(t, n) __VA_ARGS__
#define VD_GL_STRUCT_BEGIN_Z(t, n) ::glt::GLType<t>::type n;

#define VD_GL_CONSTR_V(name, ...)                                              \
    __VA_ARGS__ {}
#define VD_GL_CONSTR_F(t, n, ...) VD_GL_CONSTR_Z(t, n), __VA_ARGS__
#define VD_GL_CONSTR_Z(t, n) n(_vertex.n)

#define VD_GL_STORE_V(name, ...)                                               \
    static void store(unsigned char *gl_vertex, const unsigned char *vertex)   \
    {                                                                          \
        gl *dest = reinterpret_cast<gl *>(gl_vertex);                          \
        const name *src = reinterpret_cast<const name *>(vertex);              \
        __VA_ARGS__                                                            \
    }
#define VD_GL_STORE_F(t, n, ...) VD_GL_STORE_Z(t, n) __VA_ARGS__
#define VD_GL_STORE_Z(t, n) dest->n = src->n;

#define VD_GL_ATTRS_DECL_V(name, size)                                         \
    static const ::glt::VertexAttr vertex_attrs[size];
#define VD_GL_ATTRS_DECL_F(t, n, size) 1 + size
#define VD_GL_ATTRS_DECL_Z(t, n) 1

#define VD_GL_DESC_DECL_V(name, size)                                          \
    static const ::glt::VertexDescription<name> desc;
#define VD_GL_DESC_DECL_F(t, n, ...)
#define VD_GL_DESC_DECL_Z(t, n)

#define VD_GL_DESC_V(name, nattrs)                                             \
    const ::glt::VertexDescription<name> gl::desc = {                          \
        sizeof(gl),   alignof(gl),      nattrs,                                \
        #name "::gl", gl::vertex_attrs, gl::store                              \
    };
#define VD_GL_DESC_F(t, n, nattrs) 1 + nattrs
#define VD_GL_DESC_Z(t, n) 1

#define VD_GL_ATTRS_V(name, ...)                                               \
    const ::glt::VertexAttr gl::vertex_attrs[] = { __VA_ARGS__ };
#define VD_GL_ATTRS_F(t, n, ...) VD_GL_ATTRS_Z(t, n), __VA_ARGS__
#define VD_GL_ATTRS_Z(t, n)                                                    \
    {                                                                          \
        __builtin_offsetof(gl, n),                                             \
          ::glt::VertexAttrInfo<::glt::GLType<t>::type>::alignment,            \
          ::glt::VertexAttrInfo<::glt::GLType<t>::type>::component_type,       \
          ::glt::VertexAttrInfo<::glt::GLType<t>::type>::ncomponents,          \
          ::glt::VertexAttrInfo<::glt::GLType<t>::type>::normalized, #n        \
    }

#define DEFINE_VERTEX(V)                                                       \
    /* define the Vertex structure */                                          \
    V(VD_STRUCT_V, VD_STRUCT_F, VD_STRUCT_Z)                                   \
                                                                               \
    /* define the Vertex::gl structure */                                      \
    /* first define the fields */                                              \
    V(VD_GL_STRUCT_BEGIN_V, VD_GL_STRUCT_BEGIN_F, VD_GL_STRUCT_BEGIN_Z)        \
    /* define the conversion constructor */                                    \
    V(VD_GL_CONSTR_V, VD_GL_CONSTR_F, VD_GL_CONSTR_Z)                          \
    /* define the store function */                                            \
    V(VD_GL_STORE_V, VD_GL_STORE_F, VD_GL_STORE_Z)                             \
    /* declare the vertex attr array */                                        \
    V(VD_GL_ATTRS_DECL_V, VD_GL_ATTRS_DECL_F, VD_GL_ATTRS_DECL_Z)              \
    /* declare the vertex description structure */                             \
    V(VD_GL_DESC_DECL_V, VD_GL_DESC_DECL_F, VD_GL_DESC_DECL_Z)                 \
    }                                                                          \
    ; /* close struct gl */                                                    \
      /* declare the vertex attr array and the desc structure */               \
    V(VD_GL_ATTRS_V, VD_GL_ATTRS_F, VD_GL_ATTRS_Z)                             \
    V(VD_GL_DESC_V, VD_GL_DESC_F, VD_GL_DESC_Z)                                \
    } /* close namespace */                                                    \
                                                                               \
    V(VD_STRUCT_CONSTR_V, VD_STRUCT_CONSTR_F, VD_STRUCT_CONSTR_Z)

#if 0

#define VERTEX(V, F, Z)                                                        \
    V(Vertex, F(vec3_t, position, F(vec3_t, normal, Z(real, radius))))

DEFINE_VERTEX(VERTEX)

#endif

} // namespace glt

#endif
