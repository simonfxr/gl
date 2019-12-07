#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "glt/conf.hpp"

#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/color.hpp"
#include "glt/type_info.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "opengl.hpp"

#include <vector>

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
    char *vertex_data{};
    char *vertex_data_end{};
    char *vertex_data_lim{};
    size_t vertex_count{};
    std::vector<uint32_t> elements;
    const StructInfo &struct_info;
    std::vector<bool> enabled_attributes;
    size_t gpu_vertex_count = 0;
    size_t gpu_element_count = 0;
    GLVertexArrayObject vertex_array_name;
    GLBufferObject element_buffer_name;
    GLBufferObject vertex_buffer_name;

    GLenum usage_hint = GL_STATIC_DRAW;
    GLenum prim_type;

    DrawType draw_type = DrawArrays;

protected:
    void *vertexRef(size_t i)
    {
        return static_cast<void *>(vertex_data + i * struct_info.size);
    }

    const void *vertexRef(size_t i) const
    {
        return static_cast<const void *>(vertex_data + i * struct_info.size);
    }

    void *pushVertex()
    {
        if (hu_unlikely(vertex_data_end == vertex_data_lim))
            growVertexBuf();
        auto end = vertex_data_end;
        vertex_data_end += struct_info.size;
        ++vertex_count;
        return end;
    }

    void *pushVertexElem()
    {
        auto elem_idx = vertex_count;
        auto v = pushVertex();
        pushElement(elem_idx);
        return v;
    }

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

    size_t verticesSize() const { return vertex_count; }
    size_t elementsSize() const { return elements.size(); }

    size_t gpuVerticesSize() const { return gpu_vertex_count; }
    size_t gpuElementsSize() const { return gpu_element_count; }

    DrawType drawType() const { return draw_type; }
    void drawType(DrawType type);

    void enableAttribute(size_t i, bool enabled = true);
    bool attributeEnabled(size_t i);

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

    void pushElement(uint32_t idx) { elements.push_back(idx); }
    uint32_t element(size_t i) const { return elements[i]; }
    uint32_t &element(size_t i) { return elements[i]; }

    GLuint attributePosition(size_t offset) const;

    void bind();

private:
    void growVertexBuf();
    void enableAttributes();
    static void disableAttributes();
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
