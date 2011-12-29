#include "glt/Mesh.hpp"
#include "glt/utils.hpp"
#include "glt/glstate.hpp"

#ifdef SYSTEM_UNIX
#include <stdlib.h>
#endif

#include <cstring>

namespace glt {

using namespace defs;

namespace {

void *alloc_aligned(defs::size size, defs::size alignment) {
    ASSERT_SIZE(size);
    ASSERT_SIZE(alignment);
    void *mem;
#ifdef SYSTEM_UNIX
    if (UNSIZE(alignment) < sizeof (void *))
        alignment = sizeof (void *);
    if (posix_memalign(&mem, size_t(alignment), size_t(size)) != 0)
        mem = 0;
#else
    mem = malloc(size);
    if ((uptr(mem) & (alignment - 1)) != 0) {
        ERR("cannot alloc aligned memory");
        free(mem);
        mem = 0;
    }
#endif
    return mem;
}

byte *reallocVerts(const VertexDescBase& desc, byte *vertex_data, size old_size, size new_capa) {
    byte *verts = static_cast<byte *>(alloc_aligned(desc.sizeof_vertex * new_capa, desc.alignment));
    memcpy(verts, vertex_data, UNSIZE(desc.sizeof_vertex * old_size));
    free(vertex_data);
    return verts;
}

void validatePrimType(GLenum primType) {
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

void validateUsageHint(GLenum usageHint) {
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

}

// VertexDesc *allocVertexDesc(uint32 sizeof_vertex, uint32 nattrs, const Attr *attrs, uint32 alignment) {
//     VertexDesc *desc = malloc(sizeof *desc + sizeof attrs[0] * nattrs);
//     desc->sizeof_vertex = sizeof_vertex;

//     uint32 required_alignement = 0;

//     for (uint32 i = 0; i < nattrs; ++i) {
//         desc->attrs[i] = attrs[i];
//         if (attrs[i].alignment > required_alignment) // FIXME: not really correct
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

MeshBase::MeshBase() {
    initState();
}

void MeshBase::initState() {
    vertex_array_name = 0;
    element_buffer_name = 0;
    vertex_buffer_name = 0;
    usage_hint = GL_STATIC_DRAW;
    prim_type = GL_TRIANGLES;
    owning_vertices = true;
    vertices_capacity = 0;
    vertices_size = 0;
    gpu_vertices_size = 0;
    vertex_data = 0;
    owning_elements = true;
    elements_capacity = 0;
    elements_size = 0;
    gpu_elements_size = 0;
    element_data = 0;
    draw_type = DrawArrays;
    enabled_attributes.resize(0);
}

void MeshBase::initBase(const VertexDescBase& layout, size initial_nverts, size initial_nelems) {
    ASSERT_SIZE(initial_nverts);
    ASSERT_SIZE(initial_nelems);
    
    free();
    initState();

    vertices_capacity = MIN_NUM_VERTICES;
    while (vertices_capacity < initial_nverts)
        vertices_capacity *= 2;

    elements_capacity = MIN_NUM_ELEMENTS;
    while (elements_capacity < initial_nelems)
        elements_capacity *= 2;

    vertex_data = static_cast<byte *>(alloc_aligned(layout.sizeof_vertex * vertices_capacity, layout.alignment));
    element_data = new uint32[UNSIZE(elements_capacity)];
    desc = layout;
    enabled_attributes.resize(layout.nattributes * 2);
    enabled_attributes.set(false);
    for (defs::index i = 0; i < layout.nattributes; ++i)
        enableAttribute(i, true);
}

void MeshBase::free() {
    if (vertex_array_name != 0) {
        GL_CHECK(glDeleteVertexArrays(1, &vertex_array_name));
        --glstate.num_vertex_arrays;
    }

    freeGPU();
    
    if (owning_vertices)
        ::free(vertex_data);

    if (owning_elements)
        delete[] element_data;

    vertex_array_name = 0;
    element_buffer_name = 0;
    vertex_buffer_name = 0;
    vertex_data = 0;
    element_data = 0;
    vertices_capacity = 0;
    vertices_size = 0;
    gpu_vertices_size = 0;
    gpu_elements_size = 0;
    
}

MeshBase::~MeshBase() {
    free();
}

void MeshBase::bind() {
    if (vertex_array_name == 0)
        initVertexArray();
    GL_CHECK(glBindVertexArray(vertex_array_name));
}

void MeshBase::initVertexArray() {
    ASSERT(vertex_array_name == 0);

    ++glstate.num_vertex_arrays;
    GL_CHECK(glGenVertexArrays(1, &vertex_array_name));
}

void MeshBase::initVertexBuffer() {
    ASSERT(vertex_buffer_name == 0);
    ++glstate.num_buffers;
    GL_CHECK(glGenBuffers(1, &vertex_buffer_name));
}

void MeshBase::initVertexAttribs() {
    ASSERT(vertex_buffer_name != 0); // must be bound
    ASSERT(vertex_array_name != 0); // must also be bound

    for (defs::index i = 0; i < desc.nattributes; ++i) {
            const Attr<Any>& a = desc.attributes[i];
            GL_CHECK(glVertexAttribPointer(GLuint(i), a.ncomponents, a.component_type,
                                           gl_bool(a.normalized),
                                           desc.sizeof_vertex,
                                           reinterpret_cast<void *>(a.offset)));
    }
}

void MeshBase::primType(GLenum primType) {
    validatePrimType(primType);
    prim_type = primType;
}

void MeshBase::usageHint(GLenum usageHint) {
    validateUsageHint(usageHint);
    usage_hint = usageHint;
}
    
void MeshBase::send() {
    send(usage_hint);
}

void MeshBase::send(GLenum usageHint) {
    validateUsageHint(usageHint);

    if (vertex_array_name == 0)
        initVertexArray();

    if (vertex_buffer_name == 0)
        initVertexBuffer();

    GL_CHECK(glBindVertexArray(vertex_array_name));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_name));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices_size * desc.sizeof_vertex,
                          vertex_data, usageHint));
    gpu_vertices_size = vertices_size;
    initVertexAttribs();
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    if (element_buffer_name == 0 && elements_size > 0) {
        GL_CHECK(glGenBuffers(1, &element_buffer_name));
        ++glstate.num_buffers;
    }

    if (element_buffer_name != 0) {
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_name));
        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                              GLsizeiptr(UNSIZE(elements_size) * sizeof (GLuint)),
                              element_data, usageHint));
        gpu_elements_size = elements_size;
    }

    GL_CHECK(glBindVertexArray(0));
}

void MeshBase::drawType(DrawType type) {
    switch (type) {
    case DrawArrays:
    case DrawElements:
        break;
    default:
        ERR("invalid draw type");
        return;
    }

    draw_type = type;
}

void MeshBase::enableAttributes() {
    ASSERT(vertex_buffer_name != 0);
    ASSERT(vertex_array_name != 0);

    GL_CHECK(glBindVertexArray(vertex_array_name));

    for (defs::index i = 0; i < desc.nattributes; ++i) {
        if (enabled_attributes[2 * i] != enabled_attributes[2 * i + 1]) {
            enabled_attributes[2 * i + 1] = enabled_attributes[2 * i];
        
            if (enabled_attributes[2 * i])
                GL_CHECK(glEnableVertexArrayAttribEXT(vertex_array_name, GLuint(i)));
                // GL_CHECK(glEnableVertexAttribArray(GLuint(i)));
            else
                GL_CHECK(glDisableVertexArrayAttribEXT(vertex_array_name, GLuint(i)));
                // GL_CHECK(glDisableVertexAttribArray(GLuint(i)));
        }
    }
}

void MeshBase::disableAttributes() {
    GL_CHECK(glBindVertexArray(0));
}

void MeshBase::drawElementsInstanced(size num, GLenum primType) {
    validatePrimType(primType);
    if (gpu_elements_size == 0)
        return;
    enableAttributes();
    GL_CHECK(glDrawElementsInstanced(primType, gpu_elements_size, GL_UNSIGNED_INT, 0, num));
    disableAttributes();
}

void MeshBase::drawElements(GLenum primType) {
    validatePrimType(primType);
    if (gpu_elements_size == 0)
        return;
    enableAttributes();
    GL_CHECK(glDrawElements(primType, GLsizei(gpu_elements_size), GL_UNSIGNED_INT, 0));
    disableAttributes();
}

void MeshBase::drawArrays(GLenum primType) {
    validatePrimType(primType);
    if (gpu_vertices_size == 0)
        return;
    enableAttributes();
    GL_CHECK(glDrawArrays(primType, 0, GLsizei(gpu_vertices_size)));
    disableAttributes();
}

void MeshBase::drawArraysInstanced(size num, GLenum primType) {
    if (gpu_vertices_size == 0)
        return;
    validatePrimType(primType);
    enableAttributes();
    GL_CHECK(glDrawArraysInstanced(primType, 0, gpu_vertices_size, num));
    disableAttributes();
}

void MeshBase::draw(GLenum primType) {
    switch (draw_type) {
    case DrawArrays:
        drawArrays(primType);
        break;
    case DrawElements:
        drawElements(primType);
        break;
    }
}

void MeshBase::drawInstanced(size num, GLenum primType) {
    switch (draw_type) {
    case DrawArrays:
        drawArraysInstanced(num, primType);
        break;
    case DrawElements:
        drawElementsInstanced(num, primType);
        break;
    }
}
    
const byte *MeshBase::vertexRef(defs::index i) const {
    return vertex_data + desc.sizeof_vertex * i;
}

byte *MeshBase::vertexRef(defs::index i) {
    return vertex_data + desc.sizeof_vertex * i;
}

void MeshBase::appendVertex(const byte *vertex) {
    if (unlikely(vertices_size >= vertices_capacity)) {
        vertex_data = reallocVerts(desc, vertex_data, vertices_capacity, vertices_capacity * 2);
        vertices_capacity *= 2;
    }

    memcpy(vertexRef(vertices_size), vertex, UNSIZE(desc.sizeof_vertex));
    ++vertices_size;
}

void MeshBase::appendVertexElem(const byte *vertex) {
    appendVertex(vertex);
    addElement(defs::uint32(vertices_size - 1));
}

void MeshBase::addElement(uint32 index) {
    
    if (unlikely(elements_size >= elements_capacity)) {
        uint32 *elems = new uint32[UNSIZE(elements_capacity * 2)];
        memcpy(elems, element_data, sizeof elems[0] * UNSIZE(elements_capacity));
        delete[] element_data;
        elements_capacity *= 2;
        element_data = elems;
    }

    element_data[elements_size++] = index;
}

GLuint MeshBase::attributePosition(size offset) const {
    return static_cast<GLuint>(desc.index(offset));
}

void MeshBase::enableAttribute(defs::index i, bool enabled) {
    if (i >= desc.nattributes) {
        ERR("index out of range");
        return;
    }
    enabled_attributes[i * 2] = enabled;
}

bool MeshBase::attributeEnabled(defs::index i) {
    if (i >= desc.nattributes) {
        ERR("index out of range");
        return false;
    }

    return enabled_attributes[i * 2];

    // GLint is_enabled;
    // GL_CHECK(glGetVertexArrayIntegeri_vEXT(vertex_array_name, i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &is_enabled));
    
    // return is_enabled == GL_TRUE;
}

void MeshBase::setSize(size size) {
    defs::size capa = MIN_NUM_VERTICES;
    while (capa < size)
        capa *= 2;

    if (size < vertices_size)
        vertices_size = size;
    
    if (capa != vertices_capacity) {
        vertex_data = reallocVerts(desc, vertex_data, vertices_size, capa);
        vertices_capacity = capa;
    }
}

void MeshBase::setElementsSize(size size) {
    defs::size capa = MIN_NUM_ELEMENTS;
    while (capa < size)
        capa *= 2;

    if (size < elements_size)
        elements_size = size;

    if (capa != elements_capacity) {
        uint32 *elems = new uint32[UNSIZE(capa)];
        memcpy(elems, element_data, sizeof elems[0] * UNSIZE(elements_size));
        delete[] element_data;
        element_data = elems;
        elements_capacity = capa;
    }
}

void MeshBase::freeHost() {
    setSize(0);
    setElementsSize(0);
}

void MeshBase::freeGPU() {
    GLuint names[2] = {vertex_buffer_name, 0};
    int len = 0;

    if (vertex_buffer_name != 0) {
        names[len++] = vertex_buffer_name;
        --glstate.num_buffers;
    }
    
    if (element_buffer_name != 0) {
        names[len++] = element_buffer_name;
        --glstate.num_buffers;
    }

    if (len > 0)
        GL_CHECK(glDeleteBuffers(len, names));
}

} // namespace glt
