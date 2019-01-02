#ifndef PARSE_SPLY_HPP
#define PARSE_SPLY_HPP

#include "defs.hpp"
#include "teapot/mesh.h"

int32_t
parse_sply(const char *filename, CubeMesh &model);

#endif
