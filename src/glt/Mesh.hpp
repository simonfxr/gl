#ifndef GLT_MESH_HPP
#define GLT_MESH_HPP

#include "glt/conf.hpp"

#include "err/err.hpp"
#include "glt/GLObject.hpp"
#include "glt/color.hpp"
#include "glt/type_info.hpp"
#include "math/fvec.hpp"
#include "opengl.hpp"

#include <bl/vec.hpp>

namespace glt {

inline constexpr uint32_t ALIGNMENT_DEFAULT = 0;
inline constexpr bl::usize MIN_NUM_VERTICES = 8;
inline constexpr bl::usize MIN_NUM_ELEMENTS = 8;

enum DrawType : uint8_t
{
    DrawArrays,
    DrawElements
};

struct GLT_API MeshBase : private bl::non_copyable
{
public:
    struct VertexDataFree
    {
        void operator()(char *) noexcept;
    };
    using unique_vertex_ptr = bl::owned_ptr<char, VertexDataFree>;

    MeshBase(MeshBase &&) = default;
    MeshBase &operator=(MeshBase &&) = default;

private:
    struct GLT_API NopDeleter
    {
        void operator()(char *) const {}
    };

    bl::vec<char> vertex_data;
    bl::usize vertex_count{};
    bl::vec<uint32_t> elements;
    bl::ptr<const StructInfo> struct_info;
    bl::vec<bool> enabled_attributes;
    bl::usize gpu_vertex_count = 0;
    bl::usize gpu_element_count = 0;
    GLVertexArrayObject vertex_array_name;
    GLBufferObject element_buffer_name;
    GLBufferObject vertex_buffer_name;

    GLenum usage_hint = GL_STATIC_DRAW;
    GLenum prim_type{};

    DrawType draw_type = DrawArrays;

protected:
    void *vertexRef(bl::usize i)
    {
        return static_cast<void *>(vertex_data.data() + i * struct_info->size);
    }

    [[nodiscard]] const void *vertexRef(bl::usize i) const
    {
        return static_cast<const void *>(vertex_data.data() +
                                         i * struct_info->size);
    }

    void *pushVertex()
    {
        auto end = vertex_data.size();
        vertex_data.resize(end + struct_info->size);
        ++vertex_count;
        return static_cast<void *>(&vertex_data[end]);
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
             bl::usize initial_nverts,
             bl::usize initial_nelems,
             GLenum prim_ty);
    ~MeshBase();

    [[nodiscard]] GLenum primType() const { return prim_type; }
    void primType(GLenum primType);

    [[nodiscard]] GLenum usageHint() const { return usage_hint; }
    void usageHint(GLenum usageHint);

    [[nodiscard]] bl::usize verticesSize() const { return vertex_count; }
    [[nodiscard]] bl::usize elementsSize() const { return elements.size(); }

    [[nodiscard]] bl::usize gpuVerticesSize() const { return gpu_vertex_count; }
    [[nodiscard]] bl::usize gpuElementsSize() const
    {
        return gpu_element_count;
    }

    [[nodiscard]] DrawType drawType() const { return draw_type; }
    void drawType(DrawType type);

    void enableAttribute(bl::usize i, bool enabled = true);
    [[nodiscard]] bool attributeEnabled(bl::usize i) const;

    void freeHost();

    void freeGPU();

    void send();
    void send(GLenum usageHint);

    void drawElements() { drawElements(prim_type); }
    void drawElements(GLenum primType);

    void drawElementsInstanced(bl::usize instances)
    {
        drawElementsInstanced(instances, prim_type);
    }
    void drawElementsInstanced(bl::usize num, GLenum type);

    void drawArrays() { drawArrays(prim_type); }
    void drawArrays(GLenum primType);

    void drawArraysInstanced(bl::usize num)
    {
        drawArraysInstanced(num, prim_type);
    }
    void drawArraysInstanced(bl::usize num, GLenum primType);

    void draw() { draw(prim_type); }
    void draw(GLenum primType);

    void drawInstanced(bl::usize num) { drawInstanced(num, prim_type); }
    void drawInstanced(bl::usize num, GLenum primType);

    void pushElement(uint32_t idx) { elements.push_back(idx); }
    [[nodiscard]] uint32_t element(bl::usize i) const { return elements[i]; }
    uint32_t &element(bl::usize i) { return elements[i]; }

    [[nodiscard]] GLuint attributePosition(bl::usize offset) const;

    void bind();

private:
    void enableAttributes();
    static void disableAttributes();
    void initVertexBuffer();
    void initVertexArray();
    void initVertexAttribs();
    void free();
};

static_assert(std::is_move_constructible_v<MeshBase>);
static_assert(std::is_move_assignable_v<MeshBase>);

template<typename T>
struct Mesh : public MeshBase
{
    using gl_t = typename T::gl;
    Mesh() : Mesh(GL_TRIANGLES) {}
    explicit Mesh(GLenum) : Mesh(GL_TRIANGLES, MIN_NUM_VERTICES) {}

    Mesh(GLenum primTy,
         bl::usize initial_nverts,
         bl::usize initial_nelems = MIN_NUM_ELEMENTS)
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

    T at(bl::usize i) const
    {
        return T{ *static_cast<const gl_t *>(vertexRef(i)) };
    }

    bl::span<const gl_t> vertices() const
    {
        return bl::span{ static_cast<const gl_t *>(vertexRef(0)),
                         verticesSize() };
    }
};

} // namespace glt

#endif
