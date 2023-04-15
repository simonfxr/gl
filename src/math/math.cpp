#include "math/mat.hpp"
#include "math/vec.hpp"

namespace math {

template mat<float, 3> &
operator*=(mat<float, 3> &, const mat<float, 3> &);

template mat<double, 3> &
operator*=(mat<double, 3> &, const mat<double, 3> &);

template mat<float, 4> &
operator*=(mat<float, 4> &, const mat<float, 4> &);

template mat<double, 4> &
operator*=(mat<double, 4> &, const mat<double, 4> &);

template mat<float, 3>
inverse(const mat<float, 3> &A);

template mat<double, 3>
inverse(const mat<double, 3> &);

template mat<float, 4>
inverse(const mat<float, 4> &A);

template mat<double, 4>
inverse(const mat<double, 4> &A);
} // namespace math
