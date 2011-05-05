#ifndef PARSE_SPLY_HPP
#define PARSE_SPLY_HPP

#include "defs.h"
#include "glt/GenBatch.hpp"
#include "math/vec3/type.hpp"

struct Vertex {
    math::point3_t position;
    math::direction3_t normal;
};

int32 parse_sply(const char *filename, glt::GenBatch<Vertex>& model);

#endif
