#ifndef BL_FUNCTOR_HPP
#define BL_FUNCTOR_HPP

#include <bl/core.hpp>
#include <bl/tag_invoke.hpp>

namespace bl {
inline constexpr struct map_f
{
    BL_DEF_TAG_CPO_OPERATOR(map_f)
} map;

inline constexpr struct for_each_f
{
    BL_DEF_TAG_CPO_OPERATOR(for_each_f)
} for_each;

} // namespace bl

#endif // BL_FUNCTOR_HPP
