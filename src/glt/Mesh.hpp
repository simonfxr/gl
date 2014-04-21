#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "glt/conf.hpp"
#include "opengl.hpp"

#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

#include "glt/color.hpp"
#include "glt/VertexDescription.hpp"
#include "glt/GLObject.hpp"

#include "err/err.hpp"

#include "data/BitSet.hpp"

namespace glt {

typedef uint8 byte;

using namespace defs;

namespace {

uint32 LOCAL_CONSTANT ALIGNMENT_DEFAULT = 0;
size LOCAL_CONSTANT MIN_NUM_VERTICES = 8;
size LOCAL_CONSTANT MIN_NUM_ELEMENTS = 8;

} // namespace anon

enum DrawType {
    DrawArrays,
    DrawElements        
};

struct GLT_API MeshBase {
private:
    GLVertexArrayObject vertex_array_name;
    GLBufferObject element_buffer_name;
    GLBufferObject vertex_buffer_name;

    GLenum usage_hint;
    GLenum prim_type;

    bool owning_vertices;
    size vertices_capacity;
    size vertices_size;
    size gpu_vertices_size;
    uint8 * RESTRICT vertex_data;

    bool owning_elements;
    size elements_capacity;
    size elements_size;
    size gpu_elements_size;
    uint32 * RESTRICT element_data;
    
    DrawType draw_type;

    BitSet enabled_attributes;

    const GenVertexDescription *desc;
    
protected:
    byte * vertexRef(defs::index i);
    const byte * vertexRef(defs::index i) const;

    void appendVertex(const byte *vert);
    void appendVertexElem(const byte *vert);
    
public:

    MeshBase();
    ~MeshBase();

    void initBase(const GenVertexDescription& layout, size initial_nverts = MIN_NUM_VERTICES, size initial_nelems = MIN_NUM_ELEMENTS);

    GLenum primType() const { return prim_type; }
    void primType(GLenum primType);

    GLenum usageHint() const { return usage_hint; }
    void usageHint(GLenum usageHint);

    size verticesSize() const { return vertices_size; }
    size elementsSize() const { return elements_size; }

    size gpuVerticesSize() const { return gpu_vertices_size; }
    size gpuElementsSize() const { return gpu_elements_size; }

    DrawType drawType() const { return draw_type; }
    void drawType(DrawType type);

    void enableAttribute(defs::index i, bool enabled = true);
    bool attributeEnabled(defs::index i);

    void setSize(size size);
    
    void setElementsSize(size size);

    void freeHost();
    
    void freeGPU();
    
    void send();
    void send(GLenum usageHint);
    
    void drawElements() { drawElements(prim_type); }
    void drawElements(GLenum primType);
    
    void drawElementsInstanced(size instances) { drawElementsInstanced(instances, prim_type); }
    void drawElementsInstanced(size instances, GLenum type);

    void drawArrays() { drawArrays(prim_type); }
    void drawArrays(GLenum primType);

    void drawArraysInstanced(size num) { drawArraysInstanced(num, prim_type); }
    void drawArraysInstanced(size num, GLenum primType);

    void draw() { draw(prim_type); }
    void draw(GLenum primType);

    void drawInstanced(size num) { drawInstanced(num, prim_type); }
    void drawInstanced(size num, GLenum prim_type);    
    
    void addElement(uint32 index);
    uint32 element(defs::index index) const { return element_data[index]; }
    uint32& element(defs::index index) { return element_data[index]; }

    GLuint attributePosition(size offset) const;

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

    Mesh(const VertexDescription<T>& layout = T::gl::desc, GLenum primTy = GL_TRIANGLES, size initial_nverts = MIN_NUM_VERTICES, size initial_nelems = MIN_NUM_ELEMENTS) {
        initBase(layout.cast_gen(), initial_nverts, initial_nelems);
        primType(primTy);
    }
    
    void addVertex(const T& vert) {
        appendVertex(reinterpret_cast<const byte *>(&vert));
    }

    void addVertexElem(const T& vert) {
        appendVertexElem(reinterpret_cast<const byte *>(&vert));
    }

    T& operator[](defs::index i) {
        return *reinterpret_cast<T *>(vertexRef(i));
    }
    
    const T& operator[](defs::index i) const {
        return *reinterpret_cast<const T *>(vertexRef(i));
    }

    T& at(defs::index i) {
        return this->operator[](i);
    }

    const T& at(defs::index i) const {
        return this->operator[](i);
    }
};

} // namespace glt

#endif
