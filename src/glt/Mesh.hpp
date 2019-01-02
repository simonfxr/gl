#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "glt/conf.hpp"

#include "data/BitSet.hpp"
#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/VertexDescription.hpp"
#include "glt/color.hpp"
#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"
#include "opengl.hpp"

namespace glt {

namespace {

uint32_t LOCAL_CONSTANT ALIGNMENT_DEFAULT = 0;
defs::size_t LOCAL_CONSTANT MIN_NUM_VERTICES = 8;
defs::size_t LOCAL_CONSTANT MIN_NUM_ELEMENTS = 8;

} // namespace

enum DrawType
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

    GLenum usage_hint{};
    GLenum prim_type{};

    bool owning_vertices{};
    defs::size_t vertices_capacity{};
    defs::size_t vertices_size{};
    defs::size_t gpu_vertices_size{};
    defs::uint8_t *RESTRICT vertex_data{};

    bool owning_elements{};
    defs::size_t elements_capacity{};
    defs::size_t elements_size{};
    defs::size_t gpu_elements_size{};
    defs::uint32_t *RESTRICT element_data{};

    DrawType draw_type;

    BitSet enabled_attributes;

    const GenVertexDescription *desc{};

protected:
    defs::uint8_t *vertexRef(defs::index_t i);
    const defs::uint8_t *vertexRef(defs::index_t i) const;

    void appendVertex(const defs::uint8_t *vert);
    void appendVertexElem(const defs::uint8_t *vert);

public:
    MeshBase();
    ~MeshBase();

    void initBase(const GenVertexDescription &layout,
                  defs::size_t initial_nverts = MIN_NUM_VERTICES,
                  defs::size_t initial_nelems = MIN_NUM_ELEMENTS);

    GLenum primType() const { return prim_type; }
    void primType(GLenum primType);

    GLenum usageHint() const { return usage_hint; }
    void usageHint(GLenum usageHint);

    defs::size_t verticesSize() const { return vertices_size; }
    defs::size_t elementsSize() const { return elements_size; }

    defs::size_t gpuVerticesSize() const { return gpu_vertices_size; }
    defs::size_t gpuElementsSize() const { return gpu_elements_size; }

    DrawType drawType() const { return draw_type; }
    void drawType(DrawType type);

    void enableAttribute(defs::index_t i, bool enabled = true);
    bool attributeEnabled(defs::index_t i);

    void setSize(defs::size_t);

    void setElementsSize(defs::size_t);

    void freeHost();

    void freeGPU();

    void send();
    void send(GLenum usageHint);

    void drawElements() { drawElements(prim_type); }
    void drawElements(GLenum primType);

    void drawElementsInstanced(defs::size_t instances)
    {
        drawElementsInstanced(instances, prim_type);
    }
    void drawElementsInstanced(defs::size_t num, GLenum type);

    void drawArrays() { drawArrays(prim_type); }
    void drawArrays(GLenum primType);

    void drawArraysInstanced(defs::size_t num)
    {
        drawArraysInstanced(num, prim_type);
    }
    void drawArraysInstanced(defs::size_t num, GLenum primType);

    void draw() { draw(prim_type); }
    void draw(GLenum primType);

    void drawInstanced(defs::size_t num) { drawInstanced(num, prim_type); }
    void drawInstanced(defs::size_t num, GLenum primType);

    void addElement(uint32_t index_t);
    uint32_t element(defs::index_t index_t) const
    {
        return element_data[index_t];
    }
    uint32_t &element(defs::index_t index_t) { return element_data[index_t]; }

    GLuint attributePosition(defs::size_t offset) const;

    void bind();

private:
    void enableAttributes();
    void disableAttributes();
    void initState();
    void initVertexBuffer();
    void initVertexArray();
    void initVertexAttribs();
    void free();
};

template<typename T>
struct Mesh : public MeshBase
{

    Mesh(const VertexDescription<T> &layout = T::gl::desc,
         GLenum primTy = GL_TRIANGLES,
         defs::size_t initial_nverts = MIN_NUM_VERTICES,
         defs::size_t initial_nelems = MIN_NUM_ELEMENTS)
    {
        initBase(layout.cast_gen(), initial_nverts, initial_nelems);
        primType(primTy);
    }

    void addVertex(const T &vert)
    {
        appendVertex(reinterpret_cast<const defs::uint8_t *>(&vert));
    }

    void addVertexElem(const T &vert)
    {
        appendVertexElem(reinterpret_cast<const defs::uint8_t *>(&vert));
    }

    const T at(defs::index_t i) const
    {
        return T(*reinterpret_cast<const typename T::gl *>(vertexRef(i)));
    }
};

} // namespace glt

#endif
