#ifndef _VERTEX_BUFFER_H
#define _VERTEX_BUFFER_H

#include <GL/gl.h>

#include "defs.h"
#include "math/vec4.hpp"

struct VertexBuffer {

    vec4 * __restrict__ vertices;
    uint16 * __restrict__ elements;

    uint32 vert_size;
    uint32 vert_filled;

    uint32 elem_size;
    uint32 elem_filled;

    GLuint vert_buf_name;
    GLuint elem_buf_name;

    VertexBuffer(uint32 initial_size = 4);
    ~VertexBuffer();

    void add(const vec4& v);
    void send(GLenum usage_hint = GL_STATIC_DRAW);
    void use_as(GLenum attribute_name);
    void draw();    
};

#endif
