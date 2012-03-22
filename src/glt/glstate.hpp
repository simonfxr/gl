#ifndef GLSTATE_H
#define GLSTATE_H

#include "glt/conf.hpp"

namespace glt {

using namespace defs;

struct State {
    uint32 num_framebuffers;
    uint32 num_renderbuffers;
    uint32 num_textures;
    uint32 num_buffers;
    uint32 num_vertex_arrays;
};

extern GLT_API State glstate;

void GLT_API printState();

} // namespace glt

#endif
