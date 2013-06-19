#ifndef MESH_H
#define MESH_H

#define MESH_CUBEMESH
// #define MESH_MESH

#include "glt/Mesh.hpp"
#ifdef MESH_CUBEMESH
#include "glt/CubeMesh.hpp"
#endif

struct Vertex;
struct Vertex2;

#ifdef MESH_CUBEMESH

typedef glt::CubeMesh<Vertex> CubeMesh;
typedef glt::Mesh<Vertex> Mesh;
typedef glt::CubeMesh<Vertex2> CubeMesh2;

#else

typedef glt::Mesh<Vertex> CubeMesh;
typedef glt::Mesh<Vertex> Mesh;
typedef glt::Mesh<Vertex2> CubeMesh2;

#endif

#endif
