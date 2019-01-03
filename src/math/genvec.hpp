#ifndef GL_MATH_GENVEC_HPP
#define GL_MATH_GENVEC_HPP

#include "math/mdefs.hpp"

#include <type_traits>

#define DEF_GENVEC_SCALAR_OP(op)                                               \
    template<typename U>                                                       \
    constexpr auto operator op(const genvec<U, N> &rhs) const                  \
    {                                                                          \
                                                                               \
        using V = std::decay_t<decltype(components[0] op rhs[0])>;             \
        genvec<V, N> ret;                                                      \
        for (size_t i = 0; i < N; ++i)                                         \
            ret[i] = components[i] op rhs[i];                                  \
        return ret;                                                            \
    }

#define DEF_GENVEC_SCALAR_MUT_OP(op)                                           \
    template<typename U>                                                       \
    constexpr genvec &operator CONCAT(op, =)(const genvec<U, N> &rhs)          \
    {                                                                          \
        return *this = *this op rhs;                                           \
    }

#define DEF_GENVEC_SCALAR_OP_BOTH(op)                                          \
    DEF_GENVEC_SCALAR_OP(op)                                                   \
    DEF_GENVEC_SCALAR_MUT_OP(op)

namespace math {

template<typename T, size_t N>
struct genvec
{
    static const size_t size = N;
    static const size_t padded_size = N;
    using component_type = T;
    using buffer = T[N];

    T components[N]{};

    constexpr T &operator[](size_t i) { return components[i]; }

    constexpr T operator[](size_t i) const { return components[i]; }

    DEF_GENVEC_SCALAR_OP_BOTH(+)
    DEF_GENVEC_SCALAR_OP_BOTH(-)
    DEF_GENVEC_SCALAR_OP_BOTH(*)
    DEF_GENVEC_SCALAR_OP_BOTH(/)
};

#undef DEF_GENVEC_SCALAR_OP
#undef DEF_GENVEC_SCALAR_MUT_OP
#undef DEF_GENVEC_SCALAR_OP_BOTH

} // namespace math

#endif
