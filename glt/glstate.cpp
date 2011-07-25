#include "glt/glstate.hpp"

#include <iostream>

namespace glt {

State glstate;

void printState() {
    std::cerr << "OpenGL State: " << std::endl
              << "  Framebuffers: " << glstate.num_framebuffers << std::endl
              << "  Renderbuffers: " << glstate.num_renderbuffers << std::endl
              << "  Textures: " << glstate.num_textures << std::endl
              << "  Buffers: " << glstate.num_buffers << std::endl
              << "  Vertexarrayobjects: " << glstate.num_vertex_arrays << std::endl;
}

} // namespace glt
