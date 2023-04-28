#include "defs.h"

#include <tuple>
#include <utility>

#include "math/fvec.hpp"
#include "math/mat4.hpp"
#include "sys/clock.hpp"
#include "sys/io/Stream.hpp"

using namespace math;
using namespace bl::literals;

HU_NOINLINE static void
mul(vec4 *RESTRICT rr,
    const mat4_t *RESTRICT m,
    const vec4 *RESTRICT v,
    size_t n)
{
    mat4_t M = *m;
    vec4 r = vec4(0.f);
    for (size_t i = 0; i < n; ++i)
        r += M(v[i]);
    *rr = r;
}

static mat3_t
skew_sym(const vec3 &RESTRICT v)
{
    auto E = mat3();
    auto &[e1, e2, e3] = E;
    return { cross(v, e1), cross(v, e2), cross(v, e3) };
}

HU_NOINLINE
void
rodriguez(mat3_t &RESTRICT R, const vec3 &RESTRICT a)
{

    // a x (x_perp) = (a x (x_perp + x_par))
    // rot(da, x_perp) = x_perp + xda x x_perp
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
    auto [c, s] = cossin(theta);
    R = K * s + K * K * (1 - c) + mat3();
}

HU_NOINLINE
void
rot2(mat3_t &RESTRICT R, const vec3 &RESTRICT a)
{
    auto theta = length(a);
    auto n = a * (1 / theta);
    auto [c, s] = cossin(-theta);

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

    // sys::io::stderr() << "x-axis: " << frame.localX() << std::endl;
    // sys::io::stderr() << "y-axis: " << frame.localY() << std::endl;
    // sys::io::stderr() << "z-axis: " << frame.localZ() << std::endl;
    // sys::io::stderr() << std::endl;
    // sys::io::stderr() << std::endl;

    // sys::io::stderr() << "Orientation_World->Local: " <<
    // mat4(rotationWorldToLocal(frame)); sys::io::stderr() << std::endl;
    // sys::io::stderr() << std::endl;

    // sys::io::stderr() << "Frame_World->Local: " <<
    // transformationWorldToLocal(frame); sys::io::stderr() << std::endl;
    // sys::io::stderr() << std::endl;

    // sys::io::stderr() << "Frame_Local->World: " <<
    // transformationLocalToWorld(frame); sys::io::stderr() << std::endl;
    // sys::io::stderr() << std::endl;

    // sys::io::stderr() << "Frame_Local->Local: " <<
    // transformationLocalToWorld(frame)
    // * transformationWorldToLocal(frame); sys::io::stderr() << std::endl;
    // sys::io::stderr() << std::endl;

    sys::io::stderr() << "multiplying mat4 with "_k << N_VERTS << " vec4's"_k
                      << "\n"_k;
    sys::io::stderr() << test(N_VERTS, mat4(), vec4(1.f)) << " seconds"_k
                      << "\n"_k;
    sys::io::stderr() << "\n"_k;

    sys::io::stderr() << "multiplying mat4 with "_k << N_MATS << " mat4's"_k
                      << "\n"_k;
    sys::io::stderr() << test(N_MATS, mat4(), mat4()) << " seconds"_k
                      << "\n"_k;
    sys::io::stderr() << "\n"_k;

    mat3_t out;
    rodriguez(out, vec3(1, 2, 3));
    sys::io::stderr() << "rodriguez: "_k << out << "\n"_k;

    mat3_t out2;
    rot2(out2, vec3(1, 2, 3));
    sys::io::stderr() << "rot2: "_k << out2 << "\n"_k;

    auto delta = out - out2;
    sys::io::stderr() << "delta: "_k << delta << "\n"_k;

    return 0;
}
