
#include "VertexBuffer.h"

namespace {
    const uint32 MIN_SIZE = 4;
}

VertexBuffer::VertexBuffer(uint32 size) {
    
    if (size < MIN_SIZE) size = MINSIZE;

    vert_size = size;
    elem_size = size;

    vert_filled = 0;
    elem_filled = 0;

    vert_buf_name = 0;
    elem_buf_name = 0;

    vertices = new V3[vert_size];
    elements = new uint16[elem_size];
}

VertexBuffer::~VertexBuffer() {

    delete[] vertices;
    delete[] elements;

    GLuint buffs[] = { vert_buf_name, elem_buf_name };
    glDeleteBuffers(ARRAY_LENGTH(buffs), buffs);
}

template <typename T>
T *resize(T * restrict a, uint32 &n) {
    uint32 m = n * 2;
    T *b = new [m];

    for (uint32 i = 0; i < n; ++i)
        b[i] = a[i];

    delete[] a;
    n = m;
    return b;
}

void VertexBuffer::add(const V3& v) {
    if (elem_filled >= elem_size)
        elements = resize(elements, elem_size);
    elements[elem_filled++] = vert_filled;
    
    if (vert_filled >= vert_size)
        vertices = resize(vertices, vert_size);
    vertices[vert_filled++] = v;
}

void send(GLenum usage_hint) {

    GLuint buffs[] = { vert_buf_name, elem_buf_name };
    glDeleteBuffers(ARRAY_LENGTH(buffs), buffs);

    glGenBuffers(ARRAY_LENGTH(buffs), buffs);

    vert_buf_name = buffs[0];
    elem_buf_name = buffs[1];

    glBindBuffer(GL_ARRAY_BUFFER, vert_buf_name);
    glBufferData(GL_ARRAY_BUFFER, vert_filled, vertices, usage_hint);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_buf_name);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_filled, elements, usage_hint);
    
}
