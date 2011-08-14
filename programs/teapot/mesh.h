#ifndef MESH_H
#define MESH_H

#define MESH_GENBATCH

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
#include "glt/CubeMesh.hpp"

typedef glt::CubeMesh<Vertex> CubeMesh;
typedef glt::Mesh<Vertex> Mesh;
typedef glt::CubeMesh<Vertex2> CubeMesh2;

#define QUAD_MESH(m) UNUSED(m)
#define ADD_VERTEX(m, v) m.addVertexElem(v)
#define FREEZE_MESH(m) m.send()

#else
#error "no meshtype defined"
#endif

#endif
