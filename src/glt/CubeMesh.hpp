#ifndef GLT_CUBE_MESH_HPP
#define GLT_CUBE_MESH_HPP

#include "glt/Mesh.hpp"

namespace glt {

template<typename T>
struct CubeMesh : public Mesh<T>
{
    using gl_t = typename T::gl;
    CubeMesh(const VertexDescription<T> &layout = gl_t::descr,
             size_t initial_nverts = MIN_NUM_VERTICES,
             size_t initial_nelems = MIN_NUM_ELEMENTS)
      : Mesh<T>(layout, GL_TRIANGLES, initial_nverts, initial_nelems)
    {
        this->drawType(DrawElements);
    }

    void add(const T &vert)
    {
        this->addVertex(vert);
        uint32_t s = uint32_t(this->verticesSize());
        if (s % 4 == 0) {
            this->addElement(s - 4);
            this->addElement(s - 3);
            this->addElement(s - 2);
            this->addElement(s - 4);
            this->addElement(s - 2);
            this->addElement(s - 1);
        }
    }

    uint32_t size() { return this->verticesSize(); }

    void primType(GLenum) { ERR("cannot change primType"); }
};

} // namespace glt

#endif
