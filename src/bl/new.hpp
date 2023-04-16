#ifndef BL_NEW_HPP
#define BL_NEW_HPP

#include <bl/conf.hpp>

#include <bl/concepts.hpp>
#include <bl/invoke.hpp>
#include <bl/mp.hpp>
#include <bl/noncopymove.hpp>

#include <new>

namespace bl {

inline constexpr struct runtime_size_f
{
    template<typename T>
        requires nothrow_invocable<runtime_size_f, type_proxy<T>>
    BL_inline constexpr size_t operator()(type_proxy<T>) const noexcept
    {
        return ::bl::invoke(*this, type_proxy<T>{});
    }
} runtime_size;

struct runtime_sized_base : private non_copy_moveable
{
    BL_DISABLE_COPY_MOVE(runtime_sized_base);
    runtime_sized_base() = delete;
    ~runtime_sized_base() = delete;
};

template<typename T>
concept runtime_sized = !
std::is_reference_v<T> && !std::move_constructible<T> &&
  !std::default_initializable<T> && !std::copy_constructible<T> &&
  !std::destructible<T> && invocable<runtime_size_f, type_proxy<T>>;

template<typename T, typename... Args>
    requires(!runtime_sized<T>)
T *new_obj(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
{
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    return new T(bl_fwd(args)...);
}

template<typename T, typename... Args>
    requires runtime_sized<T>
T *
new_obj(Args &&...args) noexcept(noexcept(T::construct_at(bl_declval(T *),
                                                          bl_fwd(args)...)))
{
    size_t sz = runtime_size(type_proxy<T>{});
    auto p = ::operator new(
      sz, static_cast<std::align_val_t>(alignof(std::max_align_t)));
    decltype(auto) obj = T::construct_at(static_cast<T *>(p), bl_fwd(args)...);
    static_assert(std::is_same_v<T *, decltype(obj)>);
    return obj;
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
delete_obj(T *p) noexcept(noexcept(T::destroy_at(p)))
{
    auto sz = runtime_size(type_proxy<T>{});
    if (!p)
        return;
    T::destroy_at(p);
    ::operator delete(static_cast<void *>(p),
                      sz,
                      static_cast<std::align_val_t>(alignof(std::max_align_t)));
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
