#ifndef GLT_CUBE_MESH_HPP
#define GLT_CUBE_MESH_HPP

#include "glt/Mesh.hpp"

namespace glt {

template <typename T>
struct CubeMesh : public Mesh<T> {

    CubeMesh(const VertexDesc<T>& layout, uint32 initial_nverts = MIN_NUM_VERTICES, uint32 initial_nelems = MIN_NUM_ELEMENTS) :
        Mesh<T>(layout, GL_TRIANGLES, initial_nverts, initial_nelems)
        {}

    void add(const T& vert) {
        this->addVertex(vert);
        uint32 s = this->verticesSize();
        if (s % 4 == 0) {
            this->addElement(s - 4);
            this->addElement(s - 3);
            this->addElement(s - 2);
            this->addElement(s - 2);
            this->addElement(s - 1);
            this->addElement(s - 4);
        }
    }

    uint32 size() { return this->verticesSize(); }

    void primType(GLenum) { ERR("primType not available"); }
};

} // namespace glt

#endif
