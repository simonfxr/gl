#include "glt/glstate.hpp"
#include "sys/io/Stream.hpp"

namespace glt {

State glstate;

void printState() {
    sys::io::stdout()
        << "OpenGL State: " << sys::io::endl
        << "  Framebuffers: " << glstate.num_framebuffers << sys::io::endl
        << "  Renderbuffers: " << glstate.num_renderbuffers << sys::io::endl
        << "  Textures: " << glstate.num_textures << sys::io::endl
        << "  Buffers: " << glstate.num_buffers << sys::io::endl
        << "  Vertexarrayobjects: " << glstate.num_vertex_arrays << sys::io::endl;
}

} // namespace glt
