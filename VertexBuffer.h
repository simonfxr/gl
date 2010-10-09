#ifndef _VERTEX_BUFFER_H
#define _VERTEX_BUFFER_H

#include "Vec3.hpp"

struct VertexBuffer {
    V3 * restrict vertices;
    uint16 * restrict elements;

    uint32 size;
    uint32 filled;

    VertexBuffer();
    ~VertexBuffer();

    void add(V3 vert);
};

#endif
