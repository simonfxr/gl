#ifndef MATH_GLVEC_HPP
#define MATH_GLVEC_HPP

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "math/mat2.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace math {

#define DEF_GLVEC(gl_T, T)                                                     \
    struct gl_T                                                                \
    {                                                                          \
        float buffer[T::size]{};                                               \
        constexpr gl_T() = default;                                            \
        constexpr inline gl_T(const T &v);                                     \
        constexpr inline operator T() const;                                   \
    };

DEF_GLVEC(glvec2, vec2_t);
DEF_GLVEC(glvec3, vec3_t);
DEF_GLVEC(glvec4, vec4_t);

DEF_GLVEC(glmat2, mat2_t);
DEF_GLVEC(glmat3, mat3_t);
DEF_GLVEC(glmat4, mat4_t);

#define DEF_GLVEC_CONSTR(gl_T, T, constr)                                      \
    constexpr inline gl_T::gl_T(const T &v)                                    \
    {                                                                          \
        T::buffer buf{};                                                       \
        load(buf, v);                                                          \
        for (size_t i = 0; i < T::size; ++i)                                   \
            buffer[i] = float(buf[i]);                                         \
    }                                                                          \
    constexpr inline gl_T::operator T() const                                  \
    {                                                                          \
        T::buffer buf{};                                                       \
        for (size_t i = 0; i < T::size; ++i)                                   \
            buf[i] = T::component_type(buffer[i]);                             \
        return constr(buf);                                                    \
    }

DEF_GLVEC_CONSTR(glvec2, vec2_t, vec2);
DEF_GLVEC_CONSTR(glvec3, vec3_t, vec3);
DEF_GLVEC_CONSTR(glvec4, vec4_t, vec4);

DEF_GLVEC_CONSTR(glmat2, mat2_t, mat2);
DEF_GLVEC_CONSTR(glmat3, mat3_t, mat3);
DEF_GLVEC_CONSTR(glmat4, mat4_t, mat4);

} // namespace math
#endif
