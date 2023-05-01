#ifndef BL_NEW_HPP
#define BL_NEW_HPP

#include <bl/concepts.hpp>
#include <bl/core.hpp>
#include <bl/mp.hpp>
#include <bl/noncopymove.hpp>
#include <bl/tag_invoke.hpp>

#include <cstddef>
#include <new>

namespace bl {

inline constexpr struct runtime_size_f
{
    template<typename T>
        requires nothrow_tag_invocable<runtime_size_f, type_proxy<T>>
    BL_inline constexpr usize operator()(type_proxy<T>) const noexcept
    {
        return ::bl::tag_invoke(*this, type_proxy<T>{});
    }
} runtime_size;

inline constexpr struct runtime_size_impl_type_f
{
    template<typename T>
        requires nothrow_tag_invocable<runtime_size_impl_type_f, type_proxy<T>>
    BL_inline constexpr auto operator()(type_proxy<T>) const noexcept
    {
        return ::bl::tag_invoke(*this, type_proxy<T>{});
    }
} runtime_size_impl_type;

struct runtime_sized_tag
{
    runtime_sized_tag(const runtime_sized_tag &) = delete;
    runtime_sized_tag(runtime_sized_tag &&) = delete;

    friend bool operator==(const runtime_sized_tag &,
                           const runtime_sized_tag &) = delete;
    friend bool operator<=>(const runtime_sized_tag &,
                            const runtime_sized_tag &) = delete;

protected:
    constexpr runtime_sized_tag() = default;
    ~runtime_sized_tag() = default;
};

template<typename ApiT, typename Defer>
struct runtime_sized_impl_type_defer
{
    using type = typename bl::tag_invoke_result_t<runtime_size_impl_type_f,
                                                  type_proxy<ApiT>>::type;
};

template<typename ApiT, typename Defer = void>
using runtime_sized_impl_t =
  typename runtime_sized_impl_type_defer<ApiT, Defer>::type;

template<typename ApiT>
struct alignas(std::max_align_t) runtime_sized_base : private runtime_sized_tag
{
    BL_DISABLE_COPY_MOVE(runtime_sized_base);
    runtime_sized_base() = delete;

protected:
    constexpr runtime_sized_base(std::in_place_t) noexcept {}
    ~runtime_sized_base() = default;

    using runtime_sized_base_api_t = ApiT;
    using runtime_sized_instantiation_t = runtime_sized_base<ApiT>;

    template<typename Defer = void>
    BL_wrapper static runtime_sized_impl_t<ApiT, Defer>
      &construct_impl(void *p, auto &&...args) noexcept(
        noexcept(*new(p) runtime_sized_impl_t<ApiT, Defer>(bl_fwd(args)...)))
    {
        return *new (p) runtime_sized_impl_t<ApiT, Defer>(bl_fwd(args)...);
    }
};

#define BL_DECLARE_RUNTIME_SIZED(ApiT, ImplT)                                  \
    BL_DIAG_PUSH                                                               \
    BL_DIAG_IGN_CLANG("-Wsuggest-destructor-override")                         \
    BL_DIAG_IGN_CLANG("-Winconsistent-missing-destructor-override")            \
public:                                                                        \
    struct ImplT;                                                              \
                                                                               \
    static const ::bl::usize impl_size;                                        \
                                                                               \
    template<typename Defer = void>                                            \
    BL_wrapper constexpr friend ::bl::usize tag_invoke(                        \
      ::bl::runtime_size_f, ::bl::type_proxy<ApiT>) noexcept                   \
    {                                                                          \
        return ApiT::impl_size;                                                \
    }                                                                          \
                                                                               \
private:                                                                       \
    template<typename Defer = void>                                            \
    BL_wrapper constexpr decltype(auto) m() const noexcept                     \
    {                                                                          \
        static_assert(                                                         \
          std::is_base_of_v<ApiT, bl::runtime_sized_impl_t<ApiT, Defer>>);     \
        return *static_cast<const ::bl::runtime_sized_impl_t<ApiT, Defer> *>(  \
          static_cast<const ApiT *>(this));                                    \
    }                                                                          \
                                                                               \
    template<typename Defer = void>                                            \
    BL_wrapper constexpr decltype(auto) m() noexcept                           \
    {                                                                          \
        static_assert(                                                         \
          std::is_base_of_v<ApiT, bl::runtime_sized_impl_t<ApiT, Defer>>);     \
        return *static_cast<::bl::runtime_sized_impl_t<ApiT, Defer> *>(        \
          static_cast<ApiT *>(this));                                          \
    }                                                                          \
                                                                               \
protected:                                                                     \
    template<typename T = void>                                                \
    ApiT() noexcept : runtime_sized_instantiation_t{ ::std::in_place }         \
    {                                                                          \
        m<T>();                                                                \
    }                                                                          \
                                                                               \
    ~ApiT() = default;                                                         \
                                                                               \
public:                                                                        \
    BL_DIAG_POP                                                                \
    static void destruct_at(ApiT &) noexcept

#define BL_DEFINE_RUNTIME_SIZED(ApiT, ImplT)                                   \
    constexpr ::bl::type_proxy<ImplT> tag_invoke(                              \
      ::bl::runtime_size_impl_type_f, ::bl::type_proxy<ApiT>) noexcept         \
    {                                                                          \
        static_assert(std::is_base_of_v<ApiT, ImplT>);                         \
        return ::bl::type_proxy<ImplT>{};                                      \
    }                                                                          \
    void ApiT::destruct_at(ApiT &p) noexcept                                   \
    {                                                                          \
        ::bl::destroy_at(::bl::addressof(p.m()));                              \
    }                                                                          \
    const ::bl::usize ApiT::impl_size = sizeof(::bl::runtime_sized_impl_t<ApiT>)

template<typename T>
concept runtime_sized = !
std::is_reference_v<T> && !std::move_constructible<T> &&
  !std::copy_constructible<T> && tag_invocable<runtime_size_f, type_proxy<T>>;

template<typename T, typename... Args>
    requires(!runtime_sized<T>)
HU_RETURNS_NONNULL HU_RETURNS_NOALIAS T *new_obj(Args &&...args) noexcept(
  std::is_nothrow_constructible_v<T, Args...>)
{
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return new T(bl_fwd(args)...);
}

template<typename T, typename... Args>
    requires runtime_sized<T>
HU_RETURNS_NONNULL HU_RETURNS_NOALIAS T *
new_obj(Args &&...args) noexcept(noexcept(T::construct_at(bl_declval(T *),
                                                          bl_fwd(args)...)))
{
    usize sz = runtime_size(type_proxy<T>{});
    auto p = ::operator new(
      sz, static_cast<std::align_val_t>(alignof(std::max_align_t)));
    decltype(auto) obj = T::construct_at(static_cast<T *>(p), bl_fwd(args)...);
    static_assert(std::is_same_v<T &, decltype(obj)>);
    return bl::addressof(obj);
}

template<typename T>
    requires(!runtime_sized<T>)
void delete_obj(T *p) noexcept(std::is_nothrow_destructible_v<T>)
{
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete p;
}

template<typename T>
    requires runtime_sized<T>
void
delete_obj(T *p) noexcept(noexcept(T::destruct_at(*p)))
{
    auto sz = runtime_size(type_proxy<T>{});
    if (!p)
        return;
    T::destruct_at(*p);
#if defined(__cpp_sized_deallocation) && __cpp_sized_deallocation
    ::operator delete(static_cast<void *>(p),
                      sz,
                      static_cast<std::align_val_t>(alignof(std::max_align_t)));
#else
    (void) sz;
    ::operator delete(static_cast<void *>(p),
                      static_cast<std::align_val_t>(alignof(std::max_align_t)));
#endif
}

struct default_deleter
{
    template<typename T>
    void operator()(T *p) noexcept(noexcept(delete_obj(p)))
    {
        delete_obj(p);
    }
};

} // namespace bl

#endif // BL_NEW_HPP
