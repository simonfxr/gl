#include "math/vec3.hpp"
#include "math/mat4.hpp"
#include "math/vec4.hpp"
#include "sys/clock.hpp"

#include "math/io.hpp"

#include <iostream>

using namespace math;

#define NOINLINE ATTRS(ATTR_NOINLINE)

static void NOINLINE mul(vec4_t * RESTRICT rr, const mat4_t * RESTRICT m, const vec4_t * RESTRICT v, uptr n) {
    aligned_mat4_t M = *m;
    aligned_vec4_t r = vec4(0.f);
    for (uptr i = 0; i < n; ++i)
        r += M * v[i];
    *rr = r;
}

static void NOINLINE mul(mat4_t * RESTRICT c, const mat4_t * RESTRICT a, const mat4_t * RESTRICT b, uptr n) {
    aligned_mat4_t M = *a;
    aligned_mat4_t R = mat4(0.f);
    for (uptr i = 0; i < n; ++i)
        R += M * b[i];
    *c = R;
}

template <typename T>
float test(uptr n, const mat4_t& m_init, const T& init) {

    T *in = new T[n];
    T out;

    for (uptr i = 0; i < n; ++i)
        in[i] = init;

    float T0 = sys::queryTimer();
    mul(&out, &m_init, in, n);
    float delta = sys::queryTimer() - T0;
    
    delete[] in;
    return delta;
}

int main(void) {

    const int N_VERTS = 50000000;
    const int N_MATS  = 10000000;

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

    // std::cerr << "Orientation_World->Local: " << mat4(rotationWorldToLocal(frame));
    // std::cerr << std::endl;
    // std::cerr << std::endl;

    // std::cerr << "Frame_World->Local: " << transformationWorldToLocal(frame);
    // std::cerr << std::endl;
    // std::cerr << std::endl;

    // std::cerr << "Frame_Local->World: " << transformationLocalToWorld(frame);
    // std::cerr << std::endl;
    // std::cerr << std::endl;
    

    // std::cerr << "Frame_Local->Local: " << transformationLocalToWorld(frame) * transformationWorldToLocal(frame);
    // std::cerr << std::endl;
    // std::cerr << std::endl;
    
    std::cerr << "multiplying mat4 with " << N_VERTS << " vec4's" << std::endl;
    std::cerr << test(N_VERTS, mat4(), vec4(1.f)) << " seconds" << std::endl;
    std::cerr << std::endl;

    std::cerr << "multiplying mat4 with " << N_MATS << " mat4's" << std::endl;
    std::cerr << test(N_MATS, mat4(), mat4()) << " seconds" << std::endl;
    std::cerr << std::endl;

    return 0;
}
