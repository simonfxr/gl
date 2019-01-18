#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "glt/conf.hpp"

#include "data/BitSet.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/color.hpp"
#include "glt/type_info.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "opengl.hpp"

namespace glt {

inline constexpr uint32_t ALIGNMENT_DEFAULT = 0;
inline constexpr size_t MIN_NUM_VERTICES = 8;
inline constexpr size_t MIN_NUM_ELEMENTS = 8;

enum DrawType : uint8_t
{
    DrawArrays,
    DrawElements
};

struct GLT_API MeshBase
{
private:
    GLVertexArrayObject vertex_array_name;
    GLBufferObject element_buffer_name;
    GLBufferObject vertex_buffer_name;

    GLenum usage_hint = GL_STATIC_DRAW;
    GLenum prim_type;

    bool owning_vertices = true;
    size_t vertices_capacity = 0;
    size_t vertices_size = 0;
    size_t gpu_vertices_size = 0;
    void *RESTRICT vertex_data = nullptr;

    bool owning_elements = true;
    size_t elements_capacity = 0;
    size_t elements_size = 0;
    size_t gpu_elements_size = 0;
    uint32_t *RESTRICT element_data = nullptr;

    DrawType draw_type = DrawArrays;

    BitSet enabled_attributes;

    const StructInfo &struct_info;

protected:
    void *vertexRef(size_t i);
    const void *vertexRef(size_t i) const;

    void *pushVertex();
    void *pushVertexElem();

public:
    MeshBase(const StructInfo &,
             size_t initial_nverts,
             size_t initial_nelems,
             GLenum prim_ty);
    ~MeshBase();

    GLenum primType() const { return prim_type; }
    void primType(GLenum primType);

    GLenum usageHint() const { return usage_hint; }
    void usageHint(GLenum usageHint);

    size_t verticesSize() const { return vertices_size; }
    size_t elementsSize() const { return elements_size; }

    size_t gpuVerticesSize() const { return gpu_vertices_size; }
    size_t gpuElementsSize() const { return gpu_elements_size; }

    DrawType drawType() const { return draw_type; }
    void drawType(DrawType type);

    void enableAttribute(size_t i, bool enabled = true);
    bool attributeEnabled(size_t i);

    void setSize(size_t);

    void setElementsSize(size_t);

    void freeHost();

    void freeGPU();

    void send();
    void send(GLenum usageHint);

    void drawElements() { drawElements(prim_type); }
    void drawElements(GLenum primType);

    void drawElementsInstanced(size_t instances)
    {
        drawElementsInstanced(instances, prim_type);
    }
    void drawElementsInstanced(size_t num, GLenum type);

    void drawArrays() { drawArrays(prim_type); }
    void drawArrays(GLenum primType);

    void drawArraysInstanced(size_t num)
    {
        drawArraysInstanced(num, prim_type);
    }
    void drawArraysInstanced(size_t num, GLenum primType);

    void draw() { draw(prim_type); }
    void draw(GLenum primType);

    void drawInstanced(size_t num) { drawInstanced(num, prim_type); }
    void drawInstanced(size_t num, GLenum primType);

    void addElement(uint32_t size_t);
    uint32_t element(size_t size_t) const { return element_data[size_t]; }
    uint32_t &element(size_t size_t) { return element_data[size_t]; }

    GLuint attributePosition(size_t offset) const;

    void bind();

private:
    void enableAttributes();
    void disableAttributes();
    void initVertexBuffer();
    void initVertexArray();
    void initVertexAttribs();
    void free();
};

template<typename T>
struct Mesh : public MeshBase
{
    using gl_t = typename T::gl;
    Mesh(GLenum primTy = GL_TRIANGLES,
         size_t initial_nverts = MIN_NUM_VERTICES,
         size_t initial_nelems = MIN_NUM_ELEMENTS)
      : MeshBase(gl_t::struct_info::info,
                 initial_nverts,
                 initial_nelems,
                 primTy)
    {}

    void addVertex(const T &vert)
    {
        *static_cast<gl_t *>(pushVertex()) = static_cast<gl_t>(vert);
    }

    void addVertexElem(const T &vert)
    {
        *static_cast<gl_t *>(pushVertexElem()) = static_cast<gl_t>(vert);
    }

    const T at(size_t i) const
    {
        return *static_cast<const gl_t *>(vertexRef(i));
    }
};

} // namespace glt

#endif
