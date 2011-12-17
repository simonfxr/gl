#ifndef PARSE_SPLY_HPP
#define PARSE_SPLY_HPP

#include "defs.hpp"
#include "teapot/mesh.h"
#include "math/vec3/type.hpp"

struct Vertex {
    math::point3_t position;
    math::direction3_t normal;
};

defs::int32 parse_sply(const char *filename, CubeMesh& model);

#endif
