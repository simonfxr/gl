#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "defs.h"
#include "opengl.h"

#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"
#include "glt/color.hpp"

#include "error/error.hpp"

namespace glt {

namespace {

uint32 LOCAL_CONSTANT ALIGNMENT_DEFAULT = 0;
uint32 LOCAL_CONSTANT MIN_NUM_VERTICES = 8;
uint32 LOCAL_CONSTANT MIN_NUM_ELEMENTS = 8;

} // namespace anon

struct Any;

#define TYPE_WITNESS(t) static_cast<t *>(0)

#define VERTEX_ATTR(type, field) glt::meshTaggedAttr<type>(glt::meshAttr(offsetof(type, field), TYPE_WITNESS(type)->field))

#define VERTEX_ATTR_AS(type, field, fieldtype) glt::meshTaggedAttr<type>(glt::meshAttr(offsetof(type, field), *TYPE_WITNESS(fieldtype)))

#define DEFINE_VERTEX_ATTRS(name, type, ...)                            \
    static const glt::Attr<type> _vertex_attrs_##name[] = { __VA_ARGS__ }; \
    static const glt::VertexDesc<type> name = glt::meshAttrs(_vertex_attrs_##name, ARRAY_LENGTH(_vertex_attrs_##name))

template <typename T>
struct Attr {
    uint32 offset;
    uint32 alignment;
    GLenum component_type;
    uint32 ncomponents;
    bool normalized;

    Attr();
    Attr(size_t off, uint32 align, GLenum ty, uint32 ncomp, bool norm = false) :
        offset(uint32(off)),
        alignment(align),
        component_type(ty),
        ncomponents(ncomp),
        normalized(norm)
        {}        
};

typedef Attr<Any> AttrBase;

template <typename T>
struct VertexDesc {
    uint32 sizeof_vertex;
    uint32 alignment;
    uint32 nattributes;
    const Attr<T> *attributes;

    uint32 index(size_t off) const {
        for (uint32 i = 0; i < nattributes; ++i) {
            if (off == attributes[i].offset)
                return i;
        }
        
        FATAL_ERR("invalid attribute offset");
    }
};

typedef VertexDesc<Any> VertexDescBase;

struct MeshBase {
private:
    GLuint element_buffer_name;
    GLuint vertex_buffer_name;

    GLenum usage_hint;
    GLenum prim_type;

    bool owning_vertices;
    uint32 vertices_capacity;
    uint32 vertices_size;
    byte * RESTRICT vertex_data;

    bool owning_elements;
    uint32 elements_capacity;
    uint32 elements_size;
    uint32 * RESTRICT element_data;

    const VertexDescBase* desc;
    
protected:
    byte * vertexRef(uint32 i);
    const byte * vertexRef(uint32 i) const;

    void appendVertex(const byte *vert);
    void appendVertexElem(const byte *vert);
    
public:

    MeshBase();
    ~MeshBase();

    void initBase(const VertexDescBase *layout, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS);

    GLenum primType() const { return prim_type; }
    void primType(GLenum primType);

    GLenum usageHint() const { return usage_hint; }
    void usageHint(GLenum usageHint);

    uint32 verticesSize() const { return vertices_size; }
    uint32 elementsSize() const { return elements_size; }

    void setSize(uint32 size);
    void setElementsSize(uint32 size);
    
    void freeHost();
    
    void freeGPU();
    
    void send();
    void send(GLenum usageHint);
    
    void draw() { draw(prim_type); }
    void drawInstanced(uint32 instances) { drawInstanced(instances, prim_type); }
    void drawInstanced(uint32 instances, GLenum type);
    void draw(GLenum primType);
    
    void addElement(uint32 index);
    uint32 element(uint32 index) const { return element_data[index]; }
    uint32& element(uint32 index) { return element_data[index]; }

    GLuint attributePosition(size_t offset) const;

private:
    MeshBase(const MeshBase& _);
    MeshBase& operator =(const MeshBase& _);
};

template <typename T>
struct Mesh : public MeshBase {

    Mesh() {}
    
    Mesh(const VertexDesc<T>& layout, GLenum primTy = GL_TRIANGLES, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS) {
        initBase(reinterpret_cast<const VertexDescBase *>(&layout) , initial_nverts, initial_nelems);
        primType(primTy);
    }
    
    void init(const VertexDesc<T>& layout, GLenum primTy = GL_TRIANGLES, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS) {
        initBase(reinterpret_cast<const VertexDescBase *>(&layout) , initial_nverts, initial_nelems);
        primType(primTy);
    }
    
    void addVertex(const T& vert) {
        appendVertex(reinterpret_cast<const byte *>(&vert));
    }

    void addVertexElem(const T& vert) {
        appendVertexElem(reinterpret_cast<const byte *>(&vert));
    }

    T& operator[](uint32 i) {
        return *reinterpret_cast<T *>(vertexRef(i));
    }
    
    const T& operator[](uint32 i) const {
        return *reinterpret_cast<const T *>(vertexRef(i));
    }

    T& at(uint32 i) {
        return this->operator[](i);
    }

    const T& at(uint32 i) const {
        return this->operator[](i);
    }
};

template <typename T>
VertexDesc<T> meshAttrs(const Attr<T> attrs[], uint32 len) {
    VertexDesc<T> desc;
    desc.sizeof_vertex = sizeof(T);
    desc.alignment = ALIGNOF(T);
    desc.nattributes = len;
    desc.attributes = attrs;
    return desc;
}

inline AttrBase meshAttr(size_t offset, const float&) {
    return AttrBase(offset, ALIGNOF(float), GL_FLOAT, 1);
}

inline AttrBase meshAttr(size_t offset, const math::vec2_t&) {
    return AttrBase(offset, ALIGNOF(math::vec2_t), GL_FLOAT, 2);
}

inline AttrBase meshAttr(size_t offset, const math::vec3_t&) {
    return AttrBase(offset, ALIGNOF(math::vec3_t), GL_FLOAT, 3);
}

inline AttrBase meshAttr(size_t offset, const math::vec4_t&) {
    return AttrBase(offset, ALIGNOF(math::vec4_t), GL_FLOAT, 4);
}

inline AttrBase meshAttr(size_t offset, const glt::color&) {
    return AttrBase(offset, ALIGNOF(glt::color), GL_UNSIGNED_BYTE, 4, true);
}

template <typename T>
Attr<T> meshTaggedAttr(const AttrBase& a) {
    return reinterpret_cast<const Attr<T>&>(a);
}

} // namespace glt

#endif
