
#include "vec4.hpp"
#include "mat4.hpp"

void transform(const mat4& A, const mat4& B, vec4& v) {
    v = A * (B * v);
}

void transp(mat4& A) {
    A = A.transpose();
}

void matbaz(mat4& D, const mat4& A, const mat4& B) {
    mat4 BT = B.transpose();
    D = A * BT;
}

void baz(float *x) {
    vec4 r = (identity * vec4(1, 2, 3, 0));
    *x = mat4::dot4(r, r);
}

float blah(const vec4 a, const vec4 b) {
    return mat4::dot4(a, b);
}
