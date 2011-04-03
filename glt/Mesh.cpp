#include "Mesh.hpp"

#ifdef SYSTEM_UNIX
#include <stdlib.h>
#endif

#include <cstring>

namespace glt {

namespace {

void *alloc_aligned(uint64 size, uint32 alignment) {
    void *mem;
#ifdef SYSTEM_UNIX
    if (posix_memalign(&mem, alignment, size) != 0)
        mem = 0;
#else
    mem = malloc(size);
    if (((uint64) mem & (alignment - 1)) != 0) {
        ERROR("cannot alloc align memory");
        free(mem);
        mem = 0;
    }
#endif
    return mem;
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
    case GL_QUAD_STRIP:
    case GL_QUADS:
    case GL_POLYGON:
        break;
    default:
        ERROR("invalid OpenGL primitive type");
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
        ERROR("invalid BufferData usage hint");
    }
}

}

VertexDesc *allocVertexDesc(uint32 sizeof_vertex, uint32 nattrs, const Attr *attrs, uint32 alignment) {
    VertexDesc *desc = malloc(sizeof *desc + sizeof attrs[0] * nattrs);
    desc->sizeof_vertex = sizeof_vertex;

    uint32 required_alignement = 0;

    for (uint32 i = 0; i < nattrs; ++i) {
        desc->attrs[i] = attrs[i];
        if (attrs[i].alignment > required_alignment) // FIXME: not really correct
            required_alignment = attrs[i].alignment;
    }

    if (alignment == ALIGNMENT_DEFAULT)
        alignment = required_alignment;
    else if (required_alignment > alignment)
        ERROR("request alignment below required alignment!");

    desc->alignment = alignment;
    desc->sizeof_vertex = sizeof_vertex;
    desc->nattrs = nattrs;
}

Mesh::Mesh(const VertexDesc& layout, uint32 initial_nverts, uint32 initial_nelems) :
    element_buffer_name(0),
    vertex_buffer_name(0),
    usage_hint(GL_STATIC_DRAW),
    prim_type(GL_TRIANGLES),
    owning_vertices(true),
    vertices_capacity(initial_nverts < MIN_NUM_VERTICES ? MIN_NUM_VERTICES : initial_nverts),
    vertex_data(0)
    vertices_size(0),
    owning_elements(true),
    elements_capacity(initial_nelems < MIN_NUM_ELEMENTS ? MIN_NUM_ELEMENTS : initial_nelems),
    elements_size(0),
    element_data(0),
    desc(layout),
    consistent(true);
{
    vertex_data = alloc_aligned(vertices_capacity * desc.sizeof_vertex, desc.alignment);
    element_data = new uint32[elements_capacity];
}

Mesh::~Mesh() {
    GLuint names[] = { element_buffer_name, vertex_buffer_name };
    GL_CHECK(glDeleteBuffers(ARRAY_LENGTH(names), names));

    if (owning_vertices) {
        free(vertex_data);
        vertex_data = 0;
    }

    if (owning_elements) {
        delete[] element_data;
        element_data = 0;
    }
}

GLenum Mesh::primType() const {
    return prim_type;
}

void Mesh::primType(GLenum primType) {
    validatePrimType(primType);
    prim_type = primType;
}

GLenum Mesh::usageHint() const {
    return usage_hint;
}

void Mesh::usageHint(GLenum usageHint) {
    validateUsageHint(usageHint);
    usage_hint = usageHint;
}
    
void Mesh::send() {
    send(usage_hint);
}

void Mesh::send(GLenum usageHint) {
    validateUsageHint(usageHint);
    consistent = true;

    if (vertex_buffer_name == 0)
        GL_CHECK(glGenBuffers(1, &vertex_buffer_name));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_name));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices_size * desc.sizeof_vertex,
                          vertex_data, usageHint));

    if (element_buffer_name == 0)
        GL_CHECK(glGenBuffers(1, &element_buffer_name));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_name));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                          elements_size * sizeof (GLuint),
                          elements_data, usageHint));
}
    
void Mesh::draw() {
    draw(prim_type);
}

void Mesh::draw(GLenum primType) {
    validatePrimType(primType);
    
    if (!consistent)
        send();

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_name));

    for (uint32 i = 0; i < desc.nattrs; ++i) {
        Attr& a = desc.attrs[i];
        GL_CHECK(glVertexAttribPointer(i, a.ncomponents, a.component_type,
                                       a.normalized ? GL_TRUE : GL_FALSE,
                                       desc.sizeof_vertex,
                                       (void *) a.offset));
                                       
    }

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_name));
    GL_CHECK(glDrawElements(primType, elements_size, GL_UNSIGNED_INT, (void *) 0));
}
    
const byte *Mesh::vertexRef(uint32 i) const {
    return vertex_data + desc.sizeof_vertex * i;
}

byte *Mesh::vertexRef(uint32 i) {
    return vertex_data + desc.sizeof_vertex * i;
}

void Mesh::addVertex(const byte *vertex, bool insert_elem) {
    consistent = false;

    if (unlikely(vertices_size == vertices_capacity)) {
        byte *verts = alloc_aligned(desc.sizeof_vertex * vertices_capacity * 2, desc.alignment);
        memcpy(verts, vertex_data, desc.sizeof_vertex * vertices_capacity);
        free(vertex_data);
        vertices_capacity *= 2;
        vertex_data = verts;
    }

    memcpy(vertexRef(vertices_size), desc.sizeof_vertex);

    if (insert_elem)
        addElement(vertices_size);

    ++vertices_size;
}

void Mesh::addElement(uint32 index) {
    consistent = false;
    
    if (unlikely(elements_size == elements_capacity)) {
        uint32 *elems = new uint32[elements_capacity * 2];
        memcpy(elems, element_data, sizeof elems[0] * element_capacity);
        delete[] element_data;
        element_capacity *= 2;
        element_data = elems;
    }

    element_data[elements_size++] = index;
}

GLuint Mesh::attributePosition(uint32 offset) {
    for (uint32 i = 0; i < desc.nattrs; ++i) {
        if (offset == desc.attrs[i].offset)
            return i;
    }

    FATAL_ERROR("invalid attribute offset");
}

} // namespace glt
