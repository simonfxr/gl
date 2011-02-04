#include <iostream>
#include <ctime>
#include <cstdlib>
#include <climits>
#include <cstdio>

#include "defs.h"
#include "mat4.hpp"
#include "vec4.hpp"

double now() {
    // return clock() * (1.0 / CLOCKS_PER_SEC);
    struct timespec t;
    
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t) == -1) {
        perror("now: clock_gettime failed");
        exit(1);
    }
        
    return t.tv_sec + t.tv_nsec * (1 / 1e9);
}

#define time(op) do {                                                   \
        double t0 = now();                                              \
        (op);                                                           \
        double t1 = now();                                              \
        std::cout << #op << " took "                                    \
                  << (t1 - t0)                                          \
                  << " seconds" << std::endl;                           \
    } while (0)

using namespace math;

void __attribute__((noinline)) transform_mesh(const mat4_t& A, vec4_t * vertices, uint32 n) {
    const mat4_t AT = transpose(A);
    
    for (uint32 i = 0; i < n; ++i) {
        vertices[i] = transposedMult(AT, vertices[i]);
    }
}

float rand1() {
    return rand() * (1.f / (RAND_MAX + 1.f));
}

vec4_t rand_vec(float scale) {
    return vec4(rand1() * scale, rand1() * scale, rand1() * scale, rand1() * scale);
}

vec4_t vec4_mul1(const mat4_t& A, const vec4_t& v) {
    return A[0] * v[0] + A[1] * v[1] + A[2] * v[2] + A[3] * v[3];
}

vec4_t vec4_mul2(const mat4_t& A, const vec4_t& v) {
    mat4_t AT = transpose(A);
    return vec4(dot(A[0], v), dot(A[1], v), dot(A[2], v), dot(A[3], v));
}

int main(void) {

    FILE *randdev = fopen("/dev/urandom", "rb");
    unsigned seed;
    fread(&seed, sizeof seed, 1, randdev);
    srand(seed);
    fclose(randdev);

    static const uint32 N = 10000000;

    mat4_t A = mat4(rand_vec(1), rand_vec(1), rand_vec(1), rand_vec(1));
    
    vec4_t *vertices = new vec4_t[N];

    for (uint32 i = 0; i < N; ++i)
        vertices[i] = rand_vec(100);

    time(transform_mesh(A, vertices, N));

    return 0;
}
