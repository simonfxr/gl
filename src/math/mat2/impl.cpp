#include "math/mat2/defns.hpp"
#include "math/vec2.hpp"
#include "math/real.hpp"

#include "err/err.hpp"

MATH_BEGIN_NAMESPACE

mat2_t mat2() {
    return mat2(vec2(1.f, 0.f),
                vec2(0.f, 1.f));
}

mat2_t mat2(real x) {
    return mat2(vec2(x), vec2(x));
}

mat2_t mat2(const mat2_t::buffer mat) {
    mat2_t A;
    A[0] = vec2(&mat[0]);
    A[1] = vec2(&mat[2]);
    return A;
}

mat2_t mat2(const vec2_t& c1, const vec2_t& c2) {
    mat2_t A; A[0] = c1; A[1] = c2; return A;
}

void load(mat2_t::buffer buf, const mat2_t&A) {
    load(&buf[0], A[0]);
    load(&buf[2], A[1]);
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC const vec2_t& mat2_t::operator[](defs::index i) const {
    return columns[i];
}

MATH_INLINE_SPEC vec2_t& mat2_t::operator[](defs::index i) {
    return columns[i];
}

MATH_INLINE_SPEC real& mat2_t::operator()(defs::index i, defs::index j) {
    return components[i * vec2_t::padded_size + j];
}

} // namespace math