#include "math/mat2/defns.hpp"
#include "math/real.hpp"
#include "math/vec2.hpp"

#include "err/err.hpp"

namespace math {

constexpr mat2_t
mat2()
{
    return mat2(vec2(1, 0), vec2(0, 1));
}

constexpr mat2_t
mat2(real x)
{
    return mat2(vec2(x), vec2(x));
}

constexpr mat2_t
mat2(const mat2_t::buffer mat)
{
    mat2_t A;
    A[0] = vec2(&mat[0]);
    A[1] = vec2(&mat[2]);
    return A;
}

constexpr mat2_t
mat2(const vec2_t &c1, const vec2_t &c2)
{
    mat2_t A;
    A[0] = c1;
    A[1] = c2;
    return A;
}

void
load(mat2_t::buffer buf, const mat2_t &A)
{
    load(&buf[0], A[0]);
    load(&buf[2], A[1]);
}
} // namespace math
