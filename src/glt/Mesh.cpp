#include "glt/Mesh.hpp"

#include "util/range.hpp"
#include "glt/utils.hpp"

#ifdef HU_OS_POSIX
#include <cstdlib>
#endif

#include <cstring>

namespace glt {

namespace {

void *
alloc_aligned(size_t size, size_t alignment)
{
    void *mem;
#ifdef HU_OS_POSIX
    if (alignment < sizeof(void *))
        alignment = sizeof(void *);
    if (posix_memalign(&mem, size_t(alignment), size_t(size)) != 0)
        mem = nullptr;
#else
    mem = malloc(size);
    if ((uintptr_t(mem) & (alignment - 1)) != 0) {
        ERR("cannot alloc aligned memory");
        free(mem);
        mem = nullptr;
    }
#endif
    return mem;
}

void *
reallocVerts(const StructInfo &struct_info,
             void *vertex_data,
             size_t old_size,
             size_t new_capa)
{
    auto verts = alloc_aligned(struct_info.size * new_capa, struct_info.align);
    memcpy(verts, vertex_data, struct_info.size * old_size);
    free(vertex_data);
    return verts;
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

// VertexDesc *allocVertexDesc(uint32_t sizeof_vertex, uint32_t nattrs, const
// Attr *attrs, uint32_t alignment) {
//     VertexDesc *desc = malloc(sizeof *desc + sizeof attrs[0] * nattrs);
//     desc->sizeof_vertex = sizeof_vertex;

//     uint32_t required_alignement = 0;

//     for (uint32_t i = 0; i < nattrs; ++i) {
//         desc->attrs[i] = attrs[i];
//         if (attrs[i].alignment > required_alignment) // FIXME: not really
//         correct
//             required_alignment = attrs[i].alignment;
//     }

//     if (alignment == ALIGNMENT_DEFAULT)
//         alignment = required_alignment;
//     else if (required_alignment > alignment)
//         ERR("request alignment below required alignment!");

//     desc->alignment = alignment;
//     desc->sizeof_vertex = sizeof_vertex;
//     desc->nattrs = nattrs;
// }

MeshBase::MeshBase(const StructInfo &si,
                   size_t initial_nverts,
                   size_t initial_nelems,
                   GLenum prim_ty)
  : prim_type(prim_ty), struct_info(si)
{
    vertices_capacity = MIN_NUM_VERTICES;
    while (vertices_capacity < initial_nverts)
        vertices_capacity *= 2;

    elements_capacity = MIN_NUM_ELEMENTS;
    while (elements_capacity < initial_nelems)
        elements_capacity *= 2;

    vertex_data = alloc_aligned(si.size * vertices_capacity, si.align);
    element_data = new uint32_t[elements_capacity];
    enabled_attributes.resize(si.fields.size() * 2);
    enabled_attributes.set(false);
    for (auto i : irange(si.fields.size()))
        enableAttribute(i, true);
}

void
MeshBase::free()
{
    vertex_array_name.release();

    if (owning_vertices)
        ::free(vertex_data);

    if (owning_elements)
        delete[] element_data;

    vertex_array_name.release();
    freeGPU();

    vertex_data = nullptr;
    element_data = nullptr;
    vertices_capacity = 0;
    vertices_size = 0;
    gpu_vertices_size = 0;
    gpu_elements_size = 0;
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
            vertices_size * struct_info.size,
            vertex_data,
            usageHint);
    gpu_vertices_size = vertices_size;
    initVertexAttribs();

    if (!element_buffer_name.valid() && elements_size > 0)
        element_buffer_name.ensure();

    if (element_buffer_name.valid()) {
        GL_CALL(glNamedBufferDataEXT,
                *element_buffer_name,
                GLsizeiptr(elements_size * sizeof(GLuint)),
                element_data,
                usageHint);
        gpu_elements_size = elements_size;

        GL_CALL(glBindVertexArray, *vertex_array_name);
        GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, *element_buffer_name);
        GL_CALL(glBindVertexArray, 0);
    }
}

void
MeshBase::drawType(DrawType type)
{
    switch (type) {
    case DrawArrays:
    case DrawElements:
        break;
    }

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
    if (gpu_elements_size == 0)
        return;
    enableAttributes();
    GL_CALL(glDrawElementsInstanced,
            primType,
            GLsizei(gpu_elements_size),
            GL_UNSIGNED_INT,
            nullptr,
            GLsizei(num));
    disableAttributes();
}

void
MeshBase::drawElements(GLenum primType)
{
    validatePrimType(primType);
    if (gpu_elements_size == 0)
        return;
    enableAttributes();
    GL_CALL(glDrawElements,
            primType,
            GLsizei(gpu_elements_size),
            GL_UNSIGNED_INT,
            nullptr);
    disableAttributes();
}

void
MeshBase::drawArrays(GLenum primType)
{
    validatePrimType(primType);
    if (gpu_vertices_size == 0)
        return;
    enableAttributes();
    GL_CALL(glDrawArrays, primType, 0, GLsizei(gpu_vertices_size));
    disableAttributes();
}

void
MeshBase::drawArraysInstanced(size_t num, GLenum primType)
{
    if (gpu_vertices_size == 0)
        return;
    validatePrimType(primType);
    enableAttributes();
    GL_CALL(glDrawArraysInstanced,
            primType,
            0,
            GLsizei(gpu_vertices_size),
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

const void *
MeshBase::vertexRef(size_t i) const
{
    return static_cast<const char *>(vertex_data) + struct_info.size * i;
}

void *
MeshBase::vertexRef(size_t i)
{
    return const_cast<void *>(const_cast<const MeshBase &>(*this).vertexRef(i));
}

void *
MeshBase::pushVertex()
{
    if (unlikely(vertices_size >= vertices_capacity)) {
        vertex_data = reallocVerts(
          struct_info, vertex_data, vertices_capacity, vertices_capacity * 2);
        vertices_capacity *= 2;
    }
    return vertexRef(vertices_size++);
}

void *
MeshBase::pushVertexElem()
{
    auto ptr = pushVertex();
    addElement(uint32_t(vertices_size - 1));
    return ptr;
}

void
MeshBase::addElement(uint32_t size_t)
{

    if (unlikely(elements_size >= elements_capacity)) {
        auto *elems = new uint32_t[elements_capacity * 2];
        memcpy(elems, element_data, sizeof elems[0] * elements_capacity);
        delete[] element_data;
        elements_capacity *= 2;
        element_data = elems;
    }

    element_data[elements_size++] = size_t;
}

GLuint
MeshBase::attributePosition(size_t offset) const
{

    for (const auto [i, a] : enumerate(struct_info.fields))
        if (a.offset == offset)
            return static_cast<GLuint>(i);
    FATAL_ERR(ERROR_DEFAULT_STREAM, "offset out of range");
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

    // GLint is_enabled;
    // GL_CALL(glGetVertexArrayIntegeri_vEXT, vertex_array_name, i,
    // GL_VERTEX_ATTRIB_ARRAY_ENABLED, &is_enabled);

    // return is_enabled == GL_TRUE;
}

void
MeshBase::setSize(size_t size)
{
    size_t capa = MIN_NUM_VERTICES;
    while (capa < size)
        capa *= 2;

    if (size < vertices_size)
        vertices_size = size;

    if (capa != vertices_capacity) {
        vertex_data =
          reallocVerts(struct_info, vertex_data, vertices_size, capa);
        vertices_capacity = capa;
    }
}

void
MeshBase::setElementsSize(size_t size)
{
    size_t capa = MIN_NUM_ELEMENTS;
    while (capa < size)
        capa *= 2;

    if (size < elements_size)
        elements_size = size;

    if (capa != elements_capacity) {
        auto *elems = new uint32_t[capa];
        memcpy(elems, element_data, sizeof elems[0] * elements_size);
        delete[] element_data;
        element_data = elems;
        elements_capacity = capa;
    }
}

void
MeshBase::freeHost()
{
    setSize(0);
    setElementsSize(0);
}

void
MeshBase::freeGPU()
{
    vertex_buffer_name.release();
    element_buffer_name.release();
}

} // namespace glt
