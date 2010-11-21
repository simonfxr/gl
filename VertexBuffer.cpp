
#include <GL/glew.h>
#include <GL/glut.h>

#include "VertexBuffer.hpp"

namespace {
    const uint32 MIN_SIZE = 4;
    
    template <typename T>
    T* resize(T *a, uint32 &n) {
        uint32 m = n * 2;
        T *b = new T[m];

        for (uint32 i = 0; i < n; ++i)
            b[i] = a[i];

        delete[] a;
        n = m;
        return b;
    }
}

VertexBuffer::VertexBuffer(uint32 size) {
    if (size < MIN_SIZE)
        size = MIN_SIZE;

    vert_size = size;
    elem_size = size;

    vert_filled = 0;
    elem_filled = 0;

    vert_buf_name = 0;
    elem_buf_name = 0;

    vertices = new vec4[vert_size];
    elements = new uint16[elem_size];
}

VertexBuffer::~VertexBuffer() {
    delete[] vertices;
    delete[] elements;

    GLuint buffs[] = { vert_buf_name, elem_buf_name };
    glDeleteBuffers(ARRAY_LENGTH(buffs), buffs);
}

void VertexBuffer::add(const vec4& v) {
    if (elem_filled >= elem_size)
        elements = resize(elements, elem_size);
    elements[elem_filled++] = vert_filled;
    
    if (vert_filled >= vert_size)
        vertices = resize(vertices, vert_size);
    vertices[vert_filled++] = v;
}

void VertexBuffer::send(GLenum usage_hint) {
    GLuint buffs[] = { vert_buf_name, elem_buf_name };
    glDeleteBuffers(ARRAY_LENGTH(buffs), buffs);

    glGenBuffers(ARRAY_LENGTH(buffs), buffs);

    vert_buf_name = buffs[0];
    elem_buf_name = buffs[1];

    glBindBuffer(GL_ARRAY_BUFFER, vert_buf_name);
    glBufferData(GL_ARRAY_BUFFER, vert_filled * sizeof *vertices, vertices, usage_hint);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_buf_name);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_filled * sizeof *elements, elements, usage_hint);
}

void VertexBuffer::use_as(GLuint attrib) {
    glBindBuffer(GL_ARRAY_BUFFER, vert_buf_name);
    glVertexAttribPointer(attrib, 4, GL_FLOAT, GL_FALSE, 4 * sizeof (GLfloat), 0);
}

void VertexBuffer::draw() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_buf_name);
    glDrawElements(GL_TRIANGLE_STRIP, elem_filled, GL_UNSIGNED_SHORT, 0);
}
