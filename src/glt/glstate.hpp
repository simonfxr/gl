#ifndef GLSTATE_H
#define GLSTATE_H

#include "defs.h"

namespace glt {

struct State {
    uint32 num_framebuffers;
    uint32 num_renderbuffers;
    uint32 num_textures;
    uint32 num_buffers;
    uint32 num_vertex_arrays;
};

extern State glstate;

void printState();

} // namespace glt

#endif
