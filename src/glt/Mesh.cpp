#include "glt/Mesh.hpp"

#include "glt/utils.hpp"
#include "util/range.hpp"

#ifdef HU_OS_POSIX
#    include <cstdlib>
#endif

#include <cstdlib>
#include <cstring>

namespace glt {

namespace {

char *
realloc_vertex_buf(char *buf, size_t vsize, size_t n)
{
    auto p = static_cast<char *>(realloc(buf, vsize * n));
    ASSERT(reinterpret_cast<uintptr_t>(p) % alignof(std::max_align_t) == 0);
    return p;
}

void
free_vertex_buf(char *buf)
{
    free(buf);
}

void
validatePrimType(GLenum primType)
{
    switch (primType) {
    case GL_POINTS:
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    case GL_LINES:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLES:
    case GL_QUAD_STRIP: // deprecated in core 3.0
    case GL_QUADS:      // deprecated in core 3.0
    case GL_POLYGON:
        break;
    default:
        ERR("invalid OpenGL primitive type");
    }
}

void
validateUsageHint(GLenum usageHint)
{
    switch (usageHint) {
    case GL_STREAM_DRAW:
    case GL_STREAM_READ:
    case GL_STREAM_COPY:
    case GL_STATIC_DRAW:
    case GL_STATIC_READ:
    case GL_STATIC_COPY:
    case GL_DYNAMIC_DRAW:
    case GL_DYNAMIC_READ:
    case GL_DYNAMIC_COPY:
        break;
    default:
        ERR("invalid BufferData usage hint");
    }
}
} // namespace

MeshBase::MeshBase(const StructInfo &si,
                   size_t initial_nverts,
                   size_t initial_nelems,
                   GLenum prim_ty)
  : struct_info(si)
  , enabled_attributes(si.fields.size() * 2, false)
  , prim_type(prim_ty)
{
    ASSERT(si.align < alignof(std::max_align_t),
           "overaligned types not supported");
    elements.reserve(initial_nelems);
    auto nverts = MIN_NUM_VERTICES;
    while (nverts < initial_nverts)
        nverts *= 2;

    vertex_data = realloc_vertex_buf(nullptr, si.size, nverts);
    vertex_data_end = vertex_data;
    vertex_data_lim = vertex_data + si.size * nverts;
    for (auto i : irange(si.fields.size()))
        enableAttribute(i);
}

void
MeshBase::free()
{
    freeHost();
    freeGPU();
    vertex_array_name.release();
}

MeshBase::~MeshBase()
{
    free();
}

void
MeshBase::bind()
{
    if (!vertex_array_name.valid())
        initVertexArray();
    GL_CALL(glBindVertexArray, *vertex_array_name);
}

void
MeshBase::initVertexArray()
{
    ASSERT(vertex_array_name._name == 0);
    vertex_array_name.ensure();
}

void
MeshBase::initVertexBuffer()
{
    ASSERT(vertex_buffer_name._name == 0);
    vertex_buffer_name.ensure();
}

void
MeshBase::initVertexAttribs()
{
    ASSERT(vertex_buffer_name.valid());
    ASSERT(vertex_array_name.valid());

    for (const auto [i, a] : enumerate(struct_info.fields)) {
        GL_CALL(glVertexArrayVertexAttribOffsetEXT,
                *vertex_array_name,
                *vertex_buffer_name,
                GLuint(i),
                GLint(a.type_info.arity),
                toGLScalarType(a.type_info.scalar_type),
                gl_bool(a.type_info.normalized),
                GLsizei(struct_info.size),
                GLintptr(a.offset));
    }
}

void
MeshBase::primType(GLenum primType)
{
    validatePrimType(primType);
    prim_type = primType;
}

void
MeshBase::usageHint(GLenum usageHint)
{
    validateUsageHint(usageHint);
    usage_hint = usageHint;
}

void
MeshBase::send()
{
    send(usage_hint);
}

void
MeshBase::send(GLenum usageHint)
{
    validateUsageHint(usageHint);

    if (!vertex_array_name.valid())
        initVertexArray();

    if (!vertex_buffer_name.valid())
        initVertexBuffer();

    GL_CALL(glNamedBufferDataEXT,
            *vertex_buffer_name,
            vertex_count * struct_info.size,
            vertex_data,
            usageHint);
    gpu_vertex_count = vertex_count;
    initVertexAttribs();

    if (!element_buffer_name.valid() && !elements.empty())
        element_buffer_name.ensure();

    if (element_buffer_name.valid()) {
        GL_CALL(glNamedBufferDataEXT,
                *element_buffer_name,
                GLsizeiptr(elements.size() * sizeof(GLuint)),
                elements.data(),
                usageHint);
        gpu_element_count = elements.size();

        GL_CALL(glBindVertexArray, *vertex_array_name);
        GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, *element_buffer_name);
        GL_CALL(glBindVertexArray, 0);
    }
}

void
MeshBase::drawType(DrawType type)
{
    draw_type = type;
}

void
MeshBase::enableAttributes()
{
    ASSERT(vertex_buffer_name.valid());
    ASSERT(vertex_array_name.valid());

    for (const auto i : irange(struct_info.fields.size())) {
        if (enabled_attributes[2 * i] != enabled_attributes[2 * i + 1]) {
            enabled_attributes[2 * i + 1] = enabled_attributes[2 * i];

            if (enabled_attributes[2 * i])
                GL_CALL(
                  glEnableVertexArrayAttribEXT, *vertex_array_name, GLuint(i));
            else
                GL_CALL(
                  glDisableVertexArrayAttribEXT, *vertex_array_name, GLuint(i));
        }
    }

    GL_CALL(glBindVertexArray, *vertex_array_name);
}

void
MeshBase::disableAttributes()
{
    GL_CALL(glBindVertexArray, 0);
}

void
MeshBase::drawElementsInstanced(size_t num, GLenum primType)
{
    validatePrimType(primType);
    if (gpu_element_count == 0)
        return;
    enableAttributes();
    GL_CALL(glDrawElementsInstanced,
            primType,
            GLsizei(gpu_element_count),
            GL_UNSIGNED_INT,
            nullptr,
            GLsizei(num));
    disableAttributes();
}

void
MeshBase::drawElements(GLenum primType)
{
    validatePrimType(primType);
    if (gpu_element_count == 0)
        return;
    enableAttributes();
    GL_CALL(glDrawElements,
            primType,
            GLsizei(gpu_element_count),
            GL_UNSIGNED_INT,
            nullptr);
    disableAttributes();
}

void
MeshBase::drawArrays(GLenum primType)
{
    validatePrimType(primType);
    if (gpu_vertex_count == 0)
        return;
    enableAttributes();
    GL_CALL(glDrawArrays, primType, 0, GLsizei(gpu_vertex_count));
    disableAttributes();
}

void
MeshBase::drawArraysInstanced(size_t num, GLenum primType)
{
    if (gpu_vertex_count == 0)
        return;
    validatePrimType(primType);
    enableAttributes();
    GL_CALL(glDrawArraysInstanced,
            primType,
            0,
            GLsizei(gpu_vertex_count),
            GLsizei(num));
    disableAttributes();
}

void
MeshBase::draw(GLenum primType)
{
    switch (draw_type) {
    case DrawArrays:
        drawArrays(primType);
        break;
    case DrawElements:
        drawElements(primType);
        break;
    }
}

void
MeshBase::drawInstanced(size_t num, GLenum primType)
{
    switch (draw_type) {
    case DrawArrays:
        drawArraysInstanced(num, primType);
        break;
    case DrawElements:
        drawElementsInstanced(num, primType);
        break;
    }
}

GLuint
MeshBase::attributePosition(size_t offset) const
{

    for (const auto [i, a] : enumerate(struct_info.fields))
        if (a.offset == offset)
            return static_cast<GLuint>(i);
    FATAL_ERR("offset out of range");
}

void
MeshBase::enableAttribute(size_t i, bool enabled)
{
    if (i >= struct_info.fields.size()) {
        ERR("size_t out of range");
        return;
    }
    enabled_attributes[i * 2] = enabled;
}

bool
MeshBase::attributeEnabled(size_t i)
{
    if (i >= struct_info.fields.size()) {
        ERR("size_t out of range");
        return false;
    }

    return enabled_attributes[i * 2];
}

void
MeshBase::freeHost()
{
    free_vertex_buf(vertex_data);
    vertex_data = vertex_data_end = vertex_data_lim = nullptr;
    vertex_count = 0;
    elements.clear();
}

void
MeshBase::freeGPU()
{
    vertex_buffer_name.release();
    gpu_vertex_count = 0;
    element_buffer_name.release();
    gpu_element_count = 0;
}

void
MeshBase::growVertexBuf()
{
    size_t new_size = vertex_count == 0 ? MIN_NUM_VERTICES : vertex_count * 2;
    vertex_data = realloc_vertex_buf(vertex_data, struct_info.size, new_size);
    vertex_data_end = vertex_data + struct_info.size * vertex_count;
    vertex_data_lim = vertex_data + struct_info.size * new_size;
}

} // namespace glt
