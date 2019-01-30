#include "math/genmat.hpp"
#include "math/genvec.hpp"

namespace math {

template genmat<float, 3> operator*(const genmat<float, 3> &,
                                    const genmat<float, 3> &);

template genmat<double, 3> operator*(const genmat<double, 3> &,
                                     const genmat<double, 3> &);

template genmat<float, 4> operator*(const genmat<float, 4> &,
                                    const genmat<float, 4> &);

template genmat<double, 4> operator*(const genmat<double, 4> &,
                                     const genmat<double, 4> &);

template genmat<float, 3> &
operator*=(genmat<float, 3> &, const genmat<float, 3> &);

template genmat<double, 3> &
operator*=(genmat<double, 3> &, const genmat<double, 3> &);

template genmat<float, 4> &
operator*=(genmat<float, 4> &, const genmat<float, 4> &);

template genmat<double, 4> &
operator*=(genmat<double, 4> &, const genmat<double, 4> &);

template genmat<float, 3>
inverse(const genmat<float, 3> &A);

template genmat<double, 3>
inverse(const genmat<double, 3> &);

template genmat<float, 4>
inverse(const genmat<float, 4> &A);

template genmat<double, 4>
inverse(const genmat<double, 4> &A);

} // namespace math
