#ifndef BL_TYPE_TRAITS_HPP
#define BL_TYPE_TRAITS_HPP

#include "defs.h"

#include <type_traits>

namespace bl {

#define BL_DEF_STD_TYPE_PROP_ALIAS(nm)                                         \
    template<typename T>                                                       \
    using nm = std::nm<T>;                                                     \
    template<typename T>                                                       \
    inline constexpr bool PP_CAT(nm, _v) = std::PP_CAT(nm, _v)<T>

#define BL_DEF_STD_TYPE_PROP_ALIAS2(nm)                                        \
    template<typename T, typename U>                                           \
    using nm = std::nm<T, U>;                                                  \
    template<typename T, typename U>                                           \
    inline constexpr bool PP_CAT(nm, _v) = std::PP_CAT(nm, _v)<T, U>

#define BL_DEF_STD_TYPE_OP_ALIAS(op)                                           \
    BL_DEF_STD_TYPE_PROP_ALIAS(PP_CAT(is_, op));                               \
    BL_DEF_STD_TYPE_PROP_ALIAS(PP_CAT(is_trivially_, op));                     \
    BL_DEF_STD_TYPE_PROP_ALIAS(PP_CAT(is_nothrow_, op));

#define BL_DEF_STD_TYPE_OP_ALIAS2(op)                                          \
    BL_DEF_STD_TYPE_PROP_ALIAS2(PP_CAT(is_, op));                              \
    BL_DEF_STD_TYPE_PROP_ALIAS2(PP_CAT(is_trivially_, op));                    \
    BL_DEF_STD_TYPE_PROP_ALIAS2(PP_CAT(is_nothrow_, op));

#define BL_DEF_STD_TMPL_ALIAS(nm)                                              \
    template<typename T>                                                       \
    using nm = std::nm<T>;                                                     \
    template<typename T>                                                       \
    using PP_CAT(nm, _t) = std::PP_CAT(nm, _t)<T>

template<typename T, T value>
using integral_constant = std::integral_constant<T, value>;

template<bool x>
using bool_constant = std::bool_constant<x>;

using false_type = std::false_type;
using true_type = std::true_type;

template<bool cond, typename... Args>
using enable_if = std::enable_if<cond, Args...>;

template<bool cond, typename... Args>
using enable_if_t = typename bl::enable_if<cond, Args...>::type;

template<bool cond, typename A, typename B>
using conditional = std::conditional<cond, A, B>;

template<bool cond, typename A, typename B>
using conditional_t = std::conditional_t<cond, A, B>;

template<typename F, typename... Args>
using invoke_result = std::invoke_result<F, Args...>;

template<typename F, typename... Args>
using invoke_result_t = std::invoke_result_t<F, Args...>;

BL_DEF_STD_TYPE_PROP_ALIAS(is_void);
BL_DEF_STD_TYPE_PROP_ALIAS(is_null_pointer);
BL_DEF_STD_TYPE_PROP_ALIAS(is_integral);
BL_DEF_STD_TYPE_PROP_ALIAS(is_floating_point);
BL_DEF_STD_TYPE_PROP_ALIAS(is_array);
BL_DEF_STD_TYPE_PROP_ALIAS(is_enum);
BL_DEF_STD_TYPE_PROP_ALIAS(is_union);
BL_DEF_STD_TYPE_PROP_ALIAS(is_class);
BL_DEF_STD_TYPE_PROP_ALIAS(is_function);
BL_DEF_STD_TYPE_PROP_ALIAS(is_pointer);
BL_DEF_STD_TYPE_PROP_ALIAS(is_lvalue_reference);
BL_DEF_STD_TYPE_PROP_ALIAS(is_rvalue_reference);
BL_DEF_STD_TYPE_PROP_ALIAS(is_member_object_pointer);
BL_DEF_STD_TYPE_PROP_ALIAS(is_member_function_pointer);
BL_DEF_STD_TYPE_PROP_ALIAS(is_fundamental);
BL_DEF_STD_TYPE_PROP_ALIAS(is_arithmetic);
BL_DEF_STD_TYPE_PROP_ALIAS(is_scalar);
BL_DEF_STD_TYPE_PROP_ALIAS(is_object);
BL_DEF_STD_TYPE_PROP_ALIAS(is_compound);
BL_DEF_STD_TYPE_PROP_ALIAS(is_reference);
BL_DEF_STD_TYPE_PROP_ALIAS(is_member_pointer);
BL_DEF_STD_TYPE_PROP_ALIAS(is_const);
BL_DEF_STD_TYPE_PROP_ALIAS(is_volatile);
BL_DEF_STD_TYPE_PROP_ALIAS(is_trivial);
BL_DEF_STD_TYPE_PROP_ALIAS(is_trivially_copyable);
BL_DEF_STD_TYPE_PROP_ALIAS(is_standard_layout);
BL_DEF_STD_TYPE_PROP_ALIAS(has_unique_object_representations);
BL_DEF_STD_TYPE_PROP_ALIAS(is_empty);
BL_DEF_STD_TYPE_PROP_ALIAS(is_polymorphic);
BL_DEF_STD_TYPE_PROP_ALIAS(is_abstract);
BL_DEF_STD_TYPE_PROP_ALIAS(is_final);
BL_DEF_STD_TYPE_PROP_ALIAS(is_aggregate);
BL_DEF_STD_TYPE_PROP_ALIAS(is_signed);
BL_DEF_STD_TYPE_PROP_ALIAS(is_unsigned);

BL_DEF_STD_TYPE_OP_ALIAS(constructible);
BL_DEF_STD_TYPE_OP_ALIAS(default_constructible);
BL_DEF_STD_TYPE_OP_ALIAS(copy_constructible);
BL_DEF_STD_TYPE_OP_ALIAS(move_constructible);
BL_DEF_STD_TYPE_OP_ALIAS2(assignable);
BL_DEF_STD_TYPE_OP_ALIAS(copy_assignable);
BL_DEF_STD_TYPE_OP_ALIAS(move_assignable);
BL_DEF_STD_TYPE_OP_ALIAS(destructible);
BL_DEF_STD_TYPE_PROP_ALIAS(has_virtual_destructor);

BL_DEF_STD_TYPE_PROP_ALIAS2(is_same);
BL_DEF_STD_TYPE_PROP_ALIAS2(is_base_of);
BL_DEF_STD_TYPE_PROP_ALIAS2(is_convertible);

BL_DEF_STD_TMPL_ALIAS(remove_cv);
BL_DEF_STD_TMPL_ALIAS(remove_const);
BL_DEF_STD_TMPL_ALIAS(remove_volatile);
BL_DEF_STD_TMPL_ALIAS(add_cv);
BL_DEF_STD_TMPL_ALIAS(add_const);
BL_DEF_STD_TMPL_ALIAS(add_volatile);
BL_DEF_STD_TMPL_ALIAS(remove_reference);
BL_DEF_STD_TMPL_ALIAS(add_lvalue_reference);
BL_DEF_STD_TMPL_ALIAS(add_rvalue_reference);
BL_DEF_STD_TMPL_ALIAS(remove_pointer);
BL_DEF_STD_TMPL_ALIAS(add_pointer);
BL_DEF_STD_TMPL_ALIAS(make_signed);
BL_DEF_STD_TMPL_ALIAS(make_unsigned);
BL_DEF_STD_TMPL_ALIAS(remove_extent);
BL_DEF_STD_TMPL_ALIAS(remove_all_extents);
BL_DEF_STD_TMPL_ALIAS(decay);
BL_DEF_STD_TMPL_ALIAS(underlying_type);

template<typename T>
FORCE_INLINE inline constexpr add_const_t<T> &
as_const(T &x) noexcept
{
    return x;
}

template<class T>
void
as_const(const T &&) = delete;

} // namespace bl

#endif
