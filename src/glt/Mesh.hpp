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

inline constexpr uint32_t ALIGNMENT_DEFAULT = 0;
inline constexpr size_t MIN_NUM_VERTICES = 8;
inline constexpr size_t MIN_NUM_ELEMENTS = 8;

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
    size_t vertices_capacity{};
    size_t vertices_size{};
    size_t gpu_vertices_size{};
    uint8_t *RESTRICT vertex_data{};

    bool owning_elements{};
    size_t elements_capacity{};
    size_t elements_size{};
    size_t gpu_elements_size{};
    uint32_t *RESTRICT element_data{};

    DrawType draw_type;

    BitSet enabled_attributes;

    const GenVertexDescription *desc{};

protected:
    uint8_t *vertexRef(size_t i);
    const uint8_t *vertexRef(size_t i) const;

    void appendVertex(const uint8_t *vert);
    void appendVertexElem(const uint8_t *vert);

public:
    MeshBase();
    ~MeshBase();

    void initBase(const GenVertexDescription &layout,
                  size_t initial_nverts = MIN_NUM_VERTICES,
                  size_t initial_nelems = MIN_NUM_ELEMENTS);

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
    void initState();
    void initVertexBuffer();
    void initVertexArray();
    void initVertexAttribs();
    void free();
};

template<typename T>
struct Mesh : public MeshBase
{
    using gl_t = typename T::gl;
    Mesh(const VertexDescription<T> &layout = gl_t::descr,
         GLenum primTy = GL_TRIANGLES,
         size_t initial_nverts = MIN_NUM_VERTICES,
         size_t initial_nelems = MIN_NUM_ELEMENTS)
    {
        initBase(layout.cast_gen(), initial_nverts, initial_nelems);
        primType(primTy);
    }

    void addVertex(const T &vert)
    {
        appendVertex(reinterpret_cast<const uint8_t *>(&vert));
    }

    void addVertexElem(const T &vert)
    {
        appendVertexElem(reinterpret_cast<const uint8_t *>(&vert));
    }

    const T at(size_t i) const
    {
        return T(*reinterpret_cast<const typename T::gl *>(vertexRef(i)));
    }
};

} // namespace glt

#endif
