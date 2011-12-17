#ifndef MESH_H
#define MESH_H

#define MESH_MESH

struct Vertex;
struct Vertex2;

#ifdef MESH_GENBATCH

#include "glt/GenBatch.hpp"

typedef glt::GenBatch<Vertex> CubeMesh;
typedef glt::GenBatch<Vertex> Mesh;
typedef glt::GenBatch<Vertex2> CubeMesh2;

#define QUAD_MESH(m) m.primType(GL_QUADS)
#define ADD_VERTEX(m, v) m.add(v)
#define FREEZE_MESH(m) m.freeze()


#elif defined(MESH_MESH)

#include "glt/Mesh.hpp"
// #include "glt/CubeMesh.hpp"

typedef glt::Mesh<Vertex> CubeMesh;
typedef glt::Mesh<Vertex> Mesh;
typedef glt::Mesh<Vertex2> CubeMesh2;

#define QUAD_MESH(m) m.primType(GL_QUADS)
#define ADD_VERTEX(m, v) m.addVertexElem(v)
#define FREEZE_MESH(m) m.send()

#else
#error "no meshtype defined"
#endif

#endif
