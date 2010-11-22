
#include "vec4.hpp"
#include "mat4.hpp"

void transform(const mat4& T, vec4& v) {
    v = T * v;
}

float blah(const vec4& a, const vec4& b) {
    return mat4::dot4(a, b);
}
