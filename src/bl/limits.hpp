#ifndef BL_LIMITS_HPP
#define BL_LIMITS_HPP

#include "bl/core.hpp"

namespace bl {

template<typename T>
struct numeric_limits;

#define DEF_INT_LIMITS(t, lo, hi)                                              \
    template<>                                                                 \
    struct numeric_limits<t>                                                   \
    {                                                                          \
        FORCE_INLINE static inline constexpr t max() noexcept { return hi; }   \
        FORCE_INLINE static inline constexpr t min() noexcept { return lo; }   \
    }

#define DEF_UINT_LIMITS(t) DEF_INT_LIMITS(t, t(0), t(-1))
#define DEF_SINT_LIMITS(t, ut)                                                 \
    DEF_INT_LIMITS(t, t(ut(-1) >> 1), t(~(ut(-1) >> 1)))

DEF_UINT_LIMITS(uint8_t);
DEF_UINT_LIMITS(uint16_t);
DEF_UINT_LIMITS(uint32_t);
DEF_UINT_LIMITS(uint64_t);

DEF_SINT_LIMITS(int8_t, uint8_t);
DEF_SINT_LIMITS(int16_t, uint16_t);
DEF_SINT_LIMITS(int32_t, uint32_t);
DEF_SINT_LIMITS(int64_t, uint64_t);

template<>
struct numeric_limits<float>
{
    BL_inline static constexpr float max() noexcept
    {
        return float(3.40282346638528859811704183484516925e+38L);
    }

    BL_inline static constexpr float quiet_NaN() noexcept
    {
        return __builtin_nanf("");
    }
};

template<>
struct numeric_limits<double>
{
    BL_inline static constexpr double max() noexcept
    {
        return double(1.79769313486231570814527423731704357e+308L);
    }

    BL_inline static constexpr double quiet_NaN() noexcept
    {
        return __builtin_nan("");
    }
};

} // namespace bl

#endif
