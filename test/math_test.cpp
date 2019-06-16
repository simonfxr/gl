#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "sys/clock.hpp"
#include "sys/io/Stream.hpp"

#include <iostream>

using namespace math;

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

HU_NOINLINE static void
mul(vec4_t *RESTRICT rr,
    const mat4_t *RESTRICT m,
    const vec4_t *RESTRICT v,
    size_t n)
{
    mat4_t M = *m;
    vec4_t r = vec4(0.f);
    for (size_t i = 0; i < n; ++i)
        r += M * v[i];
    *rr = r;
}

HU_NOINLINE static void
mul(mat4_t *RESTRICT c,
    const mat4_t *RESTRICT a,
    const mat4_t *RESTRICT b,
    size_t n)
{
    mat4_t M = *a;
    mat4_t R = mat4(0.f);
    for (size_t i = 0; i < n; ++i)
        R += M * b[i];
    *c = R;
}

template<typename T>
float
test(size_t n, const mat4_t &m_init, const T &init)
{

    T *in = new T[n];
    T out;

    for (size_t i = 0; i < n; ++i)
        in[i] = init;

    float T0 = sys::queryTimer();
    mul(&out, &m_init, in, n);
    float delta = sys::queryTimer() - T0;

    delete[] in;
    return delta;
}

int
main(void)
{

    const int N_VERTS = 50000000;
    const int N_MATS = 10000000;

    // glt::Frame frame;
    // frame.rotateLocal(3.f, vec3(0.f, 1.f, 0.f));
    // frame.rotateWorld(-1.8f, vec3(1.f, 0.f, 0.f));
    // frame.translateLocal(vec3(3, 4, 5));
    // // frame.rotateWorld(3.f, vec3(1.f, 0.f, 0.f));

    // std::cerr << "x-axis: " << frame.localX() << std::endl;
    // std::cerr << "y-axis: " << frame.localY() << std::endl;
    // std::cerr << "z-axis: " << frame.localZ() << std::endl;
    // std::cerr << std::endl;
    // std::cerr << std::endl;

    // std::cerr << "Orientation_World->Local: " <<
    // mat4(rotationWorldToLocal(frame)); std::cerr << std::endl; std::cerr <<
    // std::endl;

    // std::cerr << "Frame_World->Local: " << transformationWorldToLocal(frame);
    // std::cerr << std::endl;
    // std::cerr << std::endl;

    // std::cerr << "Frame_Local->World: " << transformationLocalToWorld(frame);
    // std::cerr << std::endl;
    // std::cerr << std::endl;

    // std::cerr << "Frame_Local->Local: " << transformationLocalToWorld(frame)
    // * transformationWorldToLocal(frame); std::cerr << std::endl; std::cerr <<
    // std::endl;

    std::cerr << "multiplying mat4 with " << N_VERTS << " vec4's" << std::endl;
    std::cerr << test(N_VERTS, mat4(), vec4(1.f)) << " seconds" << std::endl;
    std::cerr << std::endl;

    std::cerr << "multiplying mat4 with " << N_MATS << " mat4's" << std::endl;
    std::cerr << test(N_MATS, mat4(), mat4()) << " seconds" << std::endl;
    std::cerr << std::endl;

    return 0;
}
