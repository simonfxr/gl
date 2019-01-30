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

#define DEF_WRAP_RT1(nm, r, t)                                                 \
    r PP_CAT(nm, _wrap)(t x) { return nm(x); }
#define DEF_WRAP_RT2(nm, r, t)                                                 \
    r PP_CAT(nm, _wrap)(t x, t y) { return nm(x, y); }

#define DEF_WRAP_1(nm)                                                         \
    DEF_WRAP_RT1(nm, float, float) DEF_WRAP_RT1(nm, double, double)

#define DEF_WRAP_1B(nm)                                                        \
    DEF_WRAP_RT1(nm, bool, float) DEF_WRAP_RT1(nm, bool, double)

#define DEF_WRAP_2(nm)                                                         \
    DEF_WRAP_RT2(nm, float, float) DEF_WRAP_RT2(nm, double, double)

DEF_WRAP_2(fmod)

DEF_WRAP_1(exp)
DEF_WRAP_1(exp2)
DEF_WRAP_1(expm1)

DEF_WRAP_1(log)
DEF_WRAP_1(log10)
DEF_WRAP_1(log1p)
DEF_WRAP_1(log2)

DEF_WRAP_2(pow)
DEF_WRAP_1(sqrt)
DEF_WRAP_1(cbrt)
DEF_WRAP_2(hypot)

DEF_WRAP_1(sin)
DEF_WRAP_1(cos)
DEF_WRAP_1(tan)
DEF_WRAP_1(asin)
DEF_WRAP_1(acos)
DEF_WRAP_1(atan)
DEF_WRAP_2(atan2)

DEF_WRAP_1(sinh)
DEF_WRAP_1(cosh)
DEF_WRAP_1(tanh)
DEF_WRAP_1(asinh)
DEF_WRAP_1(acosh)
DEF_WRAP_1(atanh)

DEF_WRAP_1(erf)
DEF_WRAP_1(erfc)
DEF_WRAP_1(lgamma)
DEF_WRAP_1(tgamma)

DEF_WRAP_1(ceil)
DEF_WRAP_1(floor)
DEF_WRAP_1(round)
DEF_WRAP_1(trunc)

DEF_WRAP_2(nextafter)
DEF_WRAP_2(nexttoward)

DEF_WRAP_1B(isfinite)
DEF_WRAP_1B(isinf)
DEF_WRAP_1B(isnan)

#if HU_COMP_GNULIKE_P
DEF_WRAP_1B(isnormal)
#endif

DEF_WRAP_2(copysign)

DEF_WRAP_1B(signbit)

} // namespace math
