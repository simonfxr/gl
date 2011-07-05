#include "glt/Mesh.hpp"
#include "glt/utils.hpp"

#ifdef SYSTEM_UNIX
#include <stdlib.h>
#endif

#include <cstring>
#include <iostream>

namespace glt {

namespace {

void *alloc_aligned(uint64 size, uint32 alignment) {
    void *mem;
#ifdef SYSTEM_UNIX
    if (alignment < sizeof (void *))
        alignment = sizeof (void *);
    if (posix_memalign(&mem, alignment, size) != 0)
        mem = 0;
#else
    mem = malloc(size);
    if (static_cast<uint64>(mem) & (alignment - 1)) != 0) {
        ERR("cannot alloc align memory");
        free(mem);
        mem = 0;
    }
#endif
    return mem;
}

byte *reallocVerts(const VertexDescBase* desc, byte *vertex_data, uint32 old_size, uint32 new_capa) {
    byte *verts = static_cast<byte *>(alloc_aligned(desc->sizeof_vertex * new_capa, desc->alignment));
    memcpy(verts, vertex_data, desc->sizeof_vertex * old_size);
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

MeshBase::MeshBase() :
    element_buffer_name(0),
    vertex_buffer_name(0),
    usage_hint(GL_STATIC_DRAW),
    prim_type(GL_TRIANGLES),
    owning_vertices(true),
    vertices_capacity(0),
    vertices_size(0),
    gpu_vertices_size(0),
    vertex_data(0),
    owning_elements(true),
    elements_capacity(0),
    elements_size(0),
    gpu_elements_size(0),
    element_data(0),
    desc(0)
{}

void MeshBase::initBase(const VertexDescBase *layout, uint32 initial_nverts, uint32 initial_nelems) {
    free();

    vertices_capacity = MIN_NUM_VERTICES;
    while (vertices_capacity < initial_nverts)
        vertices_capacity *= 2;

    elements_capacity = MIN_NUM_ELEMENTS;
    while (elements_capacity < initial_nelems)
        elements_capacity *= 2;

    element_buffer_name = 0;
    vertex_buffer_name = 0;
    usage_hint = GL_STATIC_DRAW;
    prim_type = GL_TRIANGLES;
    owning_vertices = true;
    vertices_size = 0;
    vertex_data = static_cast<byte *>(alloc_aligned(layout->sizeof_vertex * vertices_capacity, layout->alignment));
    ASSERT(vertex_data != 0);
    owning_elements = true;
    elements_size = 0;
    element_data = new uint32[elements_capacity];
    desc = layout;
    enabled_attributes = BitSet(layout->nattributes, true);
}

void MeshBase::free() {
    if (element_buffer_name != 0 || vertex_buffer_name != 0) {
        GLuint names[] = { element_buffer_name, vertex_buffer_name };
        GL_CHECK(glDeleteBuffers(ARRAY_LENGTH(names), names));
    }

    if (owning_vertices)
        ::free(vertex_data);

    if (owning_elements)
        delete[] element_data;
}


MeshBase::~MeshBase() {
    free();
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

    if (vertex_buffer_name == 0)
        GL_CHECK(glGenBuffers(1, &vertex_buffer_name));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_name));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices_size * desc->sizeof_vertex,
                          vertex_data, usageHint));

    gpu_vertices_size = vertices_size;

    if (element_buffer_name == 0)
        GL_CHECK(glGenBuffers(1, &element_buffer_name));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_name));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                          elements_size * sizeof (GLuint),
                          element_data, usageHint));

    gpu_elements_size = elements_size;
}

void MeshBase::drawInstanced(uint32 num, GLenum primType) {
    validatePrimType(primType);

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_name));

    for (uint32 i = 0; i < desc->nattributes; ++i) {
        if (!enabled_attributes[i])
            continue;
        const Attr<Any>& a = desc->attributes[i];
        GL_CHECK(glEnableVertexAttribArray(i));
        GL_CHECK(glVertexAttribPointer(i, a.ncomponents, a.component_type,
                                       a.normalized ? GL_TRUE : GL_FALSE,
                                       desc->sizeof_vertex,
                                       (void *) (size_t) a.offset));
        
    }

    if (elements_size > 0) {
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_name));
        GL_CHECK(glDrawElementsInstanced(primType, elements_size, GL_UNSIGNED_INT, 0, num));
    } else {
        GL_CHECK(glDrawArraysInstanced(primType, 0, vertices_size, num));
    }

    for (uint32 i = 0; i < desc->nattributes; ++i)
        if (enabled_attributes[i])
            GL_CHECK(glDisableVertexAttribArray(i));
}
    
void MeshBase::draw(GLenum primType) {
    validatePrimType(primType);

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_name));

    for (uint32 i = 0; i < desc->nattributes; ++i) {
        if (enabled_attributes[i]) {
            const Attr<Any>& a = desc->attributes[i];
            GL_CHECK(glEnableVertexAttribArray(i));
            GL_CHECK(glVertexAttribPointer(i, a.ncomponents, a.component_type,
                                           a.normalized ? GL_TRUE : GL_FALSE,
                                           desc->sizeof_vertex,
                                           (void *) (size_t) a.offset));
        }
    }

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_name));
    GL_CHECK(glDrawElements(primType, gpu_elements_size, GL_UNSIGNED_INT, 0));

    for (uint32 i = 0; i < desc->nattributes; ++i)
        if (enabled_attributes[i])
            GL_CHECK(glDisableVertexAttribArray(i));
}
    
const byte *MeshBase::vertexRef(uint32 i) const {
    return vertex_data + desc->sizeof_vertex * i;
}

byte *MeshBase::vertexRef(uint32 i) {
    return vertex_data + desc->sizeof_vertex * i;
}

void MeshBase::appendVertex(const byte *vertex) {
    if (unlikely(vertices_size >= vertices_capacity)) {
        vertex_data = reallocVerts(desc, vertex_data, vertices_capacity, vertices_capacity * 2);
        vertices_capacity *= 2;
    }

    memcpy(vertexRef(vertices_size), vertex, desc->sizeof_vertex);
    ++vertices_size;
}

void MeshBase::appendVertexElem(const byte *vertex) {
    appendVertex(vertex);
    addElement(vertices_size - 1);
}

void MeshBase::addElement(uint32 index) {
    
    if (unlikely(elements_size >= elements_capacity)) {
        uint32 *elems = new uint32[elements_capacity * 2];
        memcpy(elems, element_data, sizeof elems[0] * elements_capacity);
        delete[] element_data;
        elements_capacity *= 2;
        element_data = elems;
    }

    element_data[elements_size++] = index;
}

GLuint MeshBase::attributePosition(size_t offset) const {
    return static_cast<GLuint>(desc->index(offset));
}

void MeshBase::enableAttribute(uint32 i, bool enabled) {
    if (i >= desc->nattributes) {
        ERR("index out of range");
        return;
    }
    enabled_attributes[i] = enabled;
}

void MeshBase::setSize(uint32 size) {
    uint32 capa = MIN_NUM_VERTICES;
    while (capa < size)
        capa *= 2;

    if (size < vertices_size)
        vertices_size = size;
    
    if (capa != vertices_capacity) {
        vertex_data = reallocVerts(desc, vertex_data, vertices_size, capa);
        vertices_capacity = capa;
    }
}

void MeshBase::setElementsSize(uint32 size) {
    uint32 capa = MIN_NUM_ELEMENTS;
    while (capa < size)
        capa *= 2;

    if (size < elements_size)
        elements_size = size;

    if (capa != elements_capacity) {
        uint32 *elems = new uint32[capa];
        memcpy(elems, element_data, sizeof elems[0] * elements_size);
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
    WARN("not yet implemented");
}

} // namespace glt
