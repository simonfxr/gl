
#include "vec4.hpp"
#include "mat4.hpp"

void transform(const mat4& T, vec4& v) {
    v = T * v;
}
