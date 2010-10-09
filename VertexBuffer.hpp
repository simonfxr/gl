#ifndef _VERTEX_BUFFER_H
#define _VERTEX_BUFFER_H

#include <GL/gl.h>

#include "Vec3.hpp"

struct VertexBuffer {

    V3 * restrict vertices;
    uint16 * restrict elements;

    uint32 vert_size;
    uint32 vert_filled;

    uint32 elem_size;
    uint32 elem_filled;

    GLuint vert_buf_name;
    GLuint elem_buf_name;

    VertexBuffer(uint32 initial_size = 4);
    ~VertexBuffer();

    void add(const V3& vert);
    void send(GLenum usage_hint = GL_STATIC_DRAW);
};

#endif
