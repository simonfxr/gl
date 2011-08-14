#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "defs.h"
#include "opengl.hpp"

#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

#include "glt/color.hpp"
#include "glt/VertexDescription.hpp"

#include "err/err.hpp"

#include "data/BitSet.hpp"

namespace glt {

namespace {

uint32 LOCAL_CONSTANT ALIGNMENT_DEFAULT = 0;
uint32 LOCAL_CONSTANT MIN_NUM_VERTICES = 8;
uint32 LOCAL_CONSTANT MIN_NUM_ELEMENTS = 8;

} // namespace anon

#define _TYPE_WITNESS(t) static_cast<t *>(0)

#define VERTEX_ATTR(type, field) glt::meshTaggedAttr<type>(glt::meshAttr(offsetof(type, field), _TYPE_WITNESS(type)->field), AS_STRING(field))

#define VERTEX_ATTR_AS(type, field, fieldtype) glt::meshTaggedAttr<type>(glt::meshAttr(offsetof(type, field), *_TYPE_WITNESS(fieldtype)), AS_STRING(field))

#define VERTEX_ATTRS(type, ...)                                         \
    ({ static const glt::Attr<type> _vertex_attrs[] = { __VA_ARGS__ };  \
        glt::meshAttrs(_vertex_attrs, ARRAY_LENGTH(_vertex_attrs)); })

#define DEFINE_VERTEX_TRAITS(type, desc)                        \
    template <>                                                 \
    struct VertexTraits<type> {                                 \
        static const glt::VertexDesc<type>& description() {     \
            return desc;                                        \
        }                                                       \
    }                                                           \
        
#define DEFINE_VERTEX_ATTRS(name, type, ...)                            \
    static const glt::Attr<type> CONCAT(_vertex_attrs_, __LINE__) [] = { __VA_ARGS__ }; \
    const glt::VertexDesc<type> name = glt::meshAttrs(CONCAT(_vertex_attrs_, __LINE__), ARRAY_LENGTH(CONCAT(_vertex_attrs_, __LINE__)))

#define DEFINE_VERTEX_DESC(type, ...) \
    DEFINE_VERTEX_ATTRS(CONCAT3(type, _DESC_, __LINE__), type, __VA_ARGS__);  \
    DEFINE_VERTEX_TRAITS(type, CONCAT3(type, _DESC_, __LINE__))


enum DrawType {
    DrawArrays,
    DrawElements        
};

struct MeshBase {
private:
    GLuint vertex_array_name;
    GLuint element_buffer_name;
    GLuint vertex_buffer_name;

    GLenum usage_hint;
    GLenum prim_type;

    bool owning_vertices;
    uint32 vertices_capacity;
    uint32 vertices_size;
    uint32 gpu_vertices_size;
    byte * RESTRICT vertex_data;

    bool owning_elements;
    uint32 elements_capacity;
    uint32 elements_size;
    uint32 gpu_elements_size;
    uint32 * RESTRICT element_data;
    
    DrawType draw_type;

    BitSet enabled_attributes;

    VertexDescBase desc;
    
protected:
    byte * vertexRef(uint32 i);
    const byte * vertexRef(uint32 i) const;

    void appendVertex(const byte *vert);
    void appendVertexElem(const byte *vert);
    
public:

    MeshBase();
    ~MeshBase();

    void initBase(const VertexDescBase& layout, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS);

    GLenum primType() const { return prim_type; }
    void primType(GLenum primType);

    GLenum usageHint() const { return usage_hint; }
    void usageHint(GLenum usageHint);

    uint32 verticesSize() const { return vertices_size; }
    uint32 elementsSize() const { return elements_size; }

    uint32 gpuVerticesSize() const { return gpu_vertices_size; }
    uint32 gpuElementsSize() const { return gpu_elements_size; }

    DrawType drawType() const { return draw_type; }
    void drawType(DrawType type);

    void enableAttribute(uint32 i, bool enabled = true);
    bool attributeEnabled(uint32 i);

    void setSize(uint32 size);
    
    void setElementsSize(uint32 size);

    void freeHost();
    
    void freeGPU();
    
    void send();
    void send(GLenum usageHint);
    
    void drawElements() { drawElements(prim_type); }
    void drawElements(GLenum primType);
    
    void drawElementsInstanced(uint32 instances) { drawElementsInstanced(instances, prim_type); }
    void drawElementsInstanced(uint32 instances, GLenum type);

    void drawArrays() { drawArrays(prim_type); }
    void drawArrays(GLenum primType);

    void drawArraysInstanced(uint32 num) { drawArraysInstanced(num, prim_type); }
    void drawArraysInstanced(uint32 num, GLenum primType);

    void draw() { draw(prim_type); }
    void draw(GLenum primType);

    void drawInstanced(uint32 num) { drawInstanced(num, prim_type); }
    void drawInstanced(uint32 num, GLenum prim_type);    
    
    void addElement(uint32 index);
    uint32 element(uint32 index) const { return element_data[index]; }
    uint32& element(uint32 index) { return element_data[index]; }

    GLuint attributePosition(size_t offset) const;

    void bind();

private:

    void enableAttributes();
    void disableAttributes();
    void initState();
    void initVertexBuffer();
    void initVertexArray();
    void initVertexAttribs();
    
    MeshBase(const MeshBase& _);
    MeshBase& operator =(const MeshBase& _);
    void free();
};

template <typename T>
struct Mesh : public MeshBase {

    Mesh(const VertexDesc<T>& layout = VertexTraits<T>::description(), GLenum primTy = GL_TRIANGLES, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS) {
        initBase(layout.generic(), initial_nverts, initial_nelems);
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
    desc.alignment = ALIGNOF_TYPE(T);
    desc.nattributes = len;
    desc.attributes = attrs;
    return desc;
}

inline AttrBase meshAttr(size_t offset, const float&) {
    return AttrBase(offset, ALIGNOF_TYPE(float), GL_FLOAT, 1);
}

inline AttrBase meshAttr(size_t offset, const math::vec2_t&) {
    return AttrBase(offset, ALIGNOF_TYPE(math::vec2_t), GL_FLOAT, 2);
}

inline AttrBase meshAttr(size_t offset, const math::vec3_t&) {
    return AttrBase(offset, ALIGNOF_TYPE(math::vec3_t), GL_FLOAT, 3);
}

inline AttrBase meshAttr(size_t offset, const math::vec4_t&) {
    return AttrBase(offset, ALIGNOF_TYPE(math::vec4_t), GL_FLOAT, 4);
}

inline AttrBase meshAttr(size_t offset, const glt::color&) {
    return AttrBase(offset, ALIGNOF_TYPE(glt::color), GL_UNSIGNED_BYTE, 4, true);
}

template <typename T>
Attr<T> meshTaggedAttr(AttrBase a, const char *name) {
    Attr<T> typed(a);
    typed.name = name;
    return typed;
}

} // namespace glt

#endif
