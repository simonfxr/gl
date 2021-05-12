#include "defs.h"

#include <tuple>
#include <utility>

#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "sys/clock.hpp"
#include "sys/io/Stream.hpp"

#include <iostream>

using namespace math;

HU_NOINLINE
static void
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

static mat3_t
skew_sym(const vec3_t &RESTRICT v)
{
    auto E = mat3();
    auto &[e1, e2, e3] = E;
    return { cross(v, e1), cross(v, e2), cross(v, e3) };
}

HU_NOINLINE
void
rodriguez(mat3_t &RESTRICT R, const vec3_t &RESTRICT a)
{

    // a x (x_perp) = (a x (x_perp + x_par))
    // rot(da, x_perp) = x_perp + xda cross x_perp
    // with da = dtheta * a, |a| = 1
    // rot(da, x_perp) = (1 + dtheta K) x_perp
    // rot(theta * a, x_perp) = lim n->inf (1 + theta/n K)^n x_perp = exp(theta
    // K) x_perp with K^3 = -K
    // => (theta K)^(2n + 1) = (-theta)^(2n + 1) (-1)^n K
    // => (theta K)^(2n) (n > 0) = theta^2k (-1)^n
    // exp(theta K) = 1 + theta K + theta^2 K^2 / 2 + ...
    // exp(theta K) = 1 + sin(theta) K + (cos(theta) - 1) K^2
    // => rot(a, x_perp) = 1 + sin(theta) K + (cos(theta) - 1) K^2
    //

    auto theta = length(a);
    auto K = skew_sym(a * (1 / theta));
    decltype(theta) s, c;
    sincos(theta, s, c);
    R = K * s + K * K * (1 - c) + mat3();
}

HU_NOINLINE
void
rot2(mat3_t &RESTRICT R, const vec3_t &RESTRICT a)
{
    auto theta = length(a);
    auto n = a * (1 / theta);
    decltype(theta) s, c;
    sincos(-theta, s, c);

    R = mat3(vec3(n[0] * n[0] * (1 - c) + c,
                  n[0] * n[1] * (1 - c) - n[2] * s,
                  n[0] * n[2] * (1 - c) + n[1] * s),
             vec3(n[1] * n[0] * (1 - c) + n[2] * s,
                  n[1] * n[1] * (1 - c) + c,
                  n[1] * n[2] * (1 - c) - n[0] * s),
             vec3(n[2] * n[0] * (1 - c) - n[1] * s,
                  n[2] * n[1] * (1 - c) + n[0] * s,
                  n[2] * n[2] * (1 - c) + c));
}

static void HU_NOINLINE
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

    mat3_t out;
    rodriguez(out, vec3(1, 2, 3));
    std::cerr << "rodriguez: " << out << std::endl;

    mat3_t out2;
    rot2(out2, vec3(1, 2, 3));
    std::cerr << "rot2: " << out2 << std::endl;

    auto delta = out - out2;
    std::cerr << "delta: " << delta << std::endl;

    return 0;
}
