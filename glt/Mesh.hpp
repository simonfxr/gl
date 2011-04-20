#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "defs.h"
#include "opengl."

namespace glt {

namespace {

uint32 ALIGNMENT_DEFAULT = 0;
uint32 MIN_NUM_VERTICES = 8;
uint32 MIN_NUM_ELEMENTS = 16;

} // namespace anon

struct Attr {
    uint32 offset;
    uint32 alignment;
    GLenum component_type;
    uint32 ncomponents;
    bool normalized;
};

struct VertexDesc {
    uint32 sizeof_vertex;
    uint32 alignment;
    uint32 nattributes;
    Attr attributes[1]; // flexible member
};

VertexDesc *allocVertexDesc(uint32 sizeof_vertex, uint32 nattrs, const Attr *attrs, uint32 alignment = ALIGNMENT_DEFAULT);

struct Mesh {
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

    bool consistent; // is data on gpu consistent with this mesh
    
    const VertexDesc& desc;
public:

    Mesh(const VertexDesc& layout, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS);
    ~Mesh();

    GLenum primType() const;
    void primType(GLenum primType);

    GLenum usageHint() const;
    void usageHint(GLenum usageHint);
    
    void send();
    void send(GLenum usageHint);
    
    void draw();
    void draw(GLenum primType);
    
    const byte *vertexRef(uint32 i) const;
    byte *vertexRef(uint32 i);

    void addVertex(const byte *vertex, bool add_elem = true);
    void addElement(uint32 index);

    GLuint attributePosition(uint32 offset);
};

template <typename T>
struct GMesh {
    Mesh mesh;

    T& operator[](uint32 i) {
        return *static_cast<T *>(mesh.vertexRef(i));
    }

    T operator[](uint32 i) const {
        return *static_cast<T *>(mesh.vertexRef(i));
    }
};

} // namespace glt

#endif

