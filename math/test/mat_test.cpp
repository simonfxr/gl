#include <iostream>

#include "math.hpp"
#include "mat4.hpp"
#include "mat3.hpp"
#include "vec4.hpp"
#include "vec3.hpp"
#include "transform.hpp"

using namespace math;
using namespace math::Transform;

std::ostream& operator <<(std::ostream& out, const vec3_t& v) {
    return out << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

std::ostream& operator <<(std::ostream& out, const vec4_t& v) {
    return out << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
}

std::ostream& operator <<(std::ostream& out, const mat3_t& v) {
    return out << "[" << v[0] << ", " << v[1] << ", " << v[2] <<  "]";
}

std::ostream& operator <<(std::ostream& out, const mat4_t& v) {
    return out << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
}

#define test(sym, op) print_test(#sym, #op, (op))

template <typename T>
void print_test(const char *sym, const char *op, const T& result) {
    std::cout << sym << std::endl
              << "operation: " << op << std::endl
              << "result: " << result << std::endl
              << std::endl;        
}

int main(void) {
    test(identity, mat4());
    test(prop identity, mat4() * vec4(2, 3, 4, 1));
    test(mat add, mat4(1) + mat4(2) - mat4(9));
    test(mat mul identity, mat4() * mat4());
    test(translation, translate(vec3(2, 4, 8)) * mat4() * vec4(vec3(0.f), 1.f));
    test(translation2, (translate(vec3(2, 4, 8)) + translate(vec3(-2, 0, 2))) * vec4(vec3(0.f), 1.f));
    
}

vec4_t foo(const mat4_t& A, const mat4_t& B, const vec4_t v) {
    return A * (B * v);
}

vec4_t bar(const mat4_t& A, const mat4_t& B, const vec4_t v) {
    return (A * B) * v;
}
