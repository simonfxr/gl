#ifndef PARSE_SPLY_HPP
#define PARSE_SPLY_HPP

#include "defs.h"
#include "teapot/mesh.h"
#include "math/vec3/type.hpp"

struct Vertex {
    math::point3_t position;
    math::direction3_t normal;
};

int32 parse_sply(const char *filename, CubeMesh& model);

#endif
