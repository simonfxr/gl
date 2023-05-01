#ifndef BL_ANY_UNIQUE_HPP
#define BL_ANY_UNIQUE_HPP

#include <bl/tag_invoke.hpp>

/// Code adapted from the tag_invoke paper (P1895R0)
/// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
/// godbolt link from paper: https://godbolt.org/z/3TvO4f

namespace bl {

BL_DIAG_PUSH
BL_DIAG_IGN_CLANG("-Wold-style-cast")
// NOLINTBEGIN(google-readability-casting)

///////////////////////////////////////////
// Type-erasing Adapter

struct self
{};

template<typename ArgType>
struct replace_self
{
    template<typename Arg, typename T>
    using apply = Arg;

    template<typename Arg, typename T>
    static Arg &&get(Arg &&arg, T &)
    {
        return (Arg &&) arg;
    }
};

template<>
struct replace_self<self>
{
    template<typename Arg, typename T>
    using apply = T;

    template<typename Arg, typename T>
    static T &&get(Arg &&, T &obj)
    {
        return (T &&) obj;
    }
};

template<>
struct replace_self<self &>
{
    template<typename Arg, typename T>
    using apply = T &;

    template<typename Arg, typename T>
    static T &get(Arg &, T &obj) noexcept
    {
        return obj;
    }
};

template<>
struct replace_self<self &&>
{
    template<typename Arg, typename T>
    using apply = T &&;

    template<typename Arg, typename T>
    static T &&get(Arg &&, T &obj) noexcept
    {
        return (T &&) obj;
    }
};

template<>
struct replace_self<const self &>
{
    template<typename Arg, typename T>
    using apply = const T &;

    template<typename Arg, typename T>
    static const T &get(const Arg &, T &obj) noexcept
    {
        return obj;
    }
};

template<>
struct replace_self<const self &&>
{
    template<typename Arg, typename T>
    using type = const T &&;

    template<typename Arg, typename T>
    const T &&operator()(const Arg &&, T &obj) const
    {
        return (const T &&) obj;
    }
};

template<typename Arg, typename T>
using replace_self_t = typename replace_self<Arg>::template apply<Arg, T>;

template<typename Arg>
using replace_self_with_void_ptr_t =
  std::conditional_t<std::same_as<std::remove_cvref_t<Arg>, self>, void *, Arg>;

template<typename... ArgTypes>
struct extract_self;

template<typename FirstType, typename... RestTypes>
struct extract_self<FirstType, RestTypes...>
{
    template<typename TFirst, typename... TRest>
    decltype(auto) operator()(TFirst &&first, TRest &&...rest) const
    {
        if constexpr (std::same_as<std::remove_cvref_t<FirstType>, self>) {
            return first;
        } else {
            static_assert(sizeof...(TRest) > 0, "Arguments to extract_self");
            return extract_self<RestTypes...>{}((TRest &&) rest...);
        }
    }
};

namespace detail {

template<typename CPO, typename Sig>
struct overloaded_cpo : CPO
{
    constexpr overloaded_cpo() = default;
    constexpr overloaded_cpo(CPO) noexcept {}

    using type_erased_signature_t = Sig;
};

template<typename CPO>
struct base_cpo
{
    using type = CPO;
};

template<typename CPO, typename Sig>
struct base_cpo<overloaded_cpo<CPO, Sig>>
{
    using type = CPO;
};

template<typename CPO>
using base_cpo_t = typename base_cpo<CPO>::type;

template<typename CPO, typename Sig>
inline constexpr overloaded_cpo<CPO, Sig> overload_{};

} // namespace detail

template<typename Sig, typename CPO>
constexpr auto &
overload(CPO)
{
    return detail::overload_<CPO, Sig>;
}

namespace detail {

template<typename CPO, typename Sig = typename CPO::type_erased_signature_t>
struct vtable_entry;

template<typename CPO, typename Ret, typename... Args, bool NoExcept>
struct vtable_entry<CPO, Ret(Args...) noexcept(NoExcept)>
{
    using fn_t = Ret(detail::base_cpo_t<CPO>,
                     replace_self_with_void_ptr_t<Args>...) noexcept(NoExcept);

    constexpr fn_t *get() const noexcept { return fn_; }

    template<typename T>
    static constexpr vtable_entry create() noexcept
    {
        // auto& f = vtable_entry::concrete_impl<T>;
        // constexpr fn_t* f = &vtable_entry::concrete_impl<T>;
        // return vtable_entry{&f};
        return vtable_entry{
            [](detail::base_cpo_t<CPO> cpo,
               replace_self_with_void_ptr_t<Args>... args) noexcept(NoExcept)
              -> Ret {
                void *thisPointer = extract_self<Args...>{}(args...);
                T &obj = *static_cast<T *>(thisPointer);
                return bl::move(cpo)(
                  replace_self<Args>::get((Args &&) args, obj)...);
            }
        };
    }

private:
    explicit constexpr vtable_entry(fn_t *fn) noexcept : fn_(fn) {}

    fn_t *fn_;
};

template<typename... CPOs>
struct vtable : private vtable_entry<CPOs>...
{
    template<typename T>
    static constexpr vtable create() noexcept
    {
        return vtable{ vtable_entry<CPOs>::template create<T>()... };
    }

    template<typename CPO>
    constexpr auto get() const noexcept -> typename vtable_entry<CPO>::fn_t *
    {
        const vtable_entry<CPO> &entry = *this;
        return entry.get();
    }

private:
    explicit constexpr vtable(vtable_entry<CPOs>... entries) noexcept
      : vtable_entry<CPOs>{ entries }...
    {}
};

template<typename... CPOs>
struct indirect_vtable_holder
{
    template<typename T>
    static indirect_vtable_holder create()
    {
        static constexpr vtable<CPOs...> v =
          vtable<CPOs...>::template create<T>();
        return indirect_vtable_holder{ v };
    }

    const vtable<CPOs...> &operator*() const noexcept { return vtable_; }

    const vtable<CPOs...> *operator->() const noexcept
    {
        return bl::addressof(vtable_);
    }

private:
    constexpr indirect_vtable_holder(const vtable<CPOs...> &vtable)
      : vtable_(vtable)
    {}

    const vtable<CPOs...> &vtable_;
};

template<typename... CPOs>
struct inline_vtable_holder
{
    template<typename T>
    static constexpr inline_vtable_holder create()
    {
        return inline_vtable_holder{ vtable<CPOs...>::template create<T>() };
    }

    const vtable<CPOs...> &operator*() const noexcept { return vtable_; }

    const vtable<CPOs...> *operator->() const noexcept
    {
        return bl::addressof(vtable_);
    }

private:
    constexpr inline_vtable_holder(const vtable<CPOs...> &vtable)
      : vtable_(vtable)
    {}

    vtable<CPOs...> vtable_;
};

template<typename... CPOs>
using vtable_holder = std::conditional_t<(sizeof...(CPOs) <= 2),
                                         inline_vtable_holder<CPOs...>,
                                         indirect_vtable_holder<CPOs...>>;

template<typename Derived,
         typename CPO,
         typename Sig = typename CPO::type_erased_signature_t>
struct with_type_erased_tag_invoke;

template<typename Derived,
         typename CPO,
         typename Ret,
         typename... Args,
         bool NoExcept>
struct with_type_erased_tag_invoke<Derived,
                                   CPO,
                                   Ret(Args...) noexcept(NoExcept)>
{
    friend Ret tag_invoke(
      base_cpo_t<CPO> cpo,
      replace_self_t<Args, Derived>... args) noexcept(NoExcept)
    {
        auto &t = extract_self<Args...>{}(args...);
        void *objPtr = t.get_object_address();
        auto *fnPtr = t.get_vtable()->template get<CPO>();
        return fnPtr(bl::move(cpo),
                     replace_self<Args>::get((Args &&) args, objPtr)...);
    }
};

template<typename Derived,
         typename CPO,
         typename Sig = typename CPO::type_erased_signature_t>
struct with_forwarding_tag_invoke;

template<typename Derived,
         typename CPO,
         typename Ret,
         typename... Args,
         bool NoExcept>
struct with_forwarding_tag_invoke<Derived, CPO, Ret(Args...) noexcept(NoExcept)>
{
    friend Ret tag_invoke(
      detail::base_cpo_t<CPO> cpo,
      replace_self_t<Args, Derived>... args) noexcept(NoExcept)
    {
        auto &wrapper = extract_self<Args...>{}(args...);
        auto &wrapped = wrapper.value_ref();
        return bl::move(cpo)(
          replace_self<Args>::get((Args &&) args, wrapped)...);
    }
};

inline constexpr struct deallocate_cpo
{
    using type_erased_signature_t = void(self &&) noexcept;

    template<typename T>
        requires bl::tag_invocable<deallocate_cpo, T>
    void operator()(T &&obj) const noexcept
    {
        tag_invoke(deallocate_cpo{}, (T &&) obj);
    }
} deallocate;

template<template<typename...> class T, typename U>
constexpr bool is_instance_of_v = false;

template<template<typename...> class T, typename... Ts>
constexpr bool is_instance_of_v<T, T<Ts...>> = true;

template<typename T,
         typename CPO,
         typename Sig = typename CPO::type_erased_signature_t>
struct is_cpo_invocable : std::false_type
{};

template<typename T,
         typename CPO,
         typename Ret,
         typename... Args,
         bool NoExcept>
    requires std::invocable<CPO, replace_self_t<Args, T>...> &&
             std::convertible_to<
               std::invoke_result_t<CPO, replace_self_t<Args, T>...>,
               Ret>
struct is_cpo_invocable<T, CPO, Ret(Args...) noexcept(NoExcept)>
  : std::true_type
{};

template<typename T, typename... RequiredCPOs>
concept is_type_erasable = (is_cpo_invocable<T, RequiredCPOs>::value && ...);

} // namespace detail

template<typename... CPOs>
class any_unique
  : private detail::with_type_erased_tag_invoke<any_unique<CPOs...>, CPOs>...
{
public:
    template<typename Concrete, typename Allocator, typename... Args>
        requires std::constructible_from<Concrete, Args...> &&
                 detail::is_type_erasable<Concrete, CPOs...>
    explicit any_unique(std::allocator_arg_t,
                        Allocator alloc,
                        std::in_place_type_t<Concrete>,
                        Args &&...args)
      : vtable_(vtable_holder_t::template create<
                concrete_impl<Concrete, Allocator>>())
    {
        using concrete_type = concrete_impl<Concrete, Allocator>;
        using allocator_type = typename concrete_type::allocator_type;
        using allocator_traits = std::allocator_traits<allocator_type>;
        allocator_type typedAllocator{ bl::move(alloc) };
        auto ptr = allocator_traits::allocate(typedAllocator, 1);
        // FIXME:        try {

        // TODO: Ideally we'd use allocator_traits::construct() here but
        // that makes it difficult to provide consistent behaviour across
        // std::allocator and std::pmr::polymorphic_allocator as the latter
        // automatically injects the extra allocator_arg/alloc params which
        // ends up duplicating them. But std::allocator doesn't do the same
        // injection of the parameters.
        ::new ((void *) ptr) concrete_type{ std::allocator_arg,
                                            typedAllocator,
                                            (Args &&) args... };
        // } catch (...) {
        //     allocator_traits::deallocate(typedAllocator, ptr, 1);
        //     throw;
        // }
        impl_ = static_cast<void *>(ptr);
    }

    template<typename Concrete, typename Allocator>
        requires std::constructible_from<std::remove_cvref_t<Concrete>,
                                         Concrete> &&
                 (!std::same_as<std::allocator_arg_t, std::decay_t<Concrete>> &&
                  !detail::is_instance_of_v<std::in_place_type_t,
                                            std::decay_t<Concrete>>) &&
                 detail::is_type_erasable<Concrete, CPOs...>
    any_unique(Concrete &&concrete, Allocator alloc)
      : any_unique(std::allocator_arg,
                   bl::move(alloc),
                   std::in_place_type<std::remove_cvref_t<Concrete>>,
                   (Concrete &&) concrete)
    {}

    template<typename Concrete, typename... Args>
        requires std::constructible_from<Concrete, Args...> &&
                 detail::is_type_erasable<Concrete, CPOs...>
    explicit any_unique(std::in_place_type_t<Concrete> tag, Args &&...args)
      : any_unique(std::allocator_arg,
                   std::allocator<unsigned char>{},
                   tag,
                   (Args &&) args...)
    {}

    template<typename Concrete>
        requires(!detail::is_instance_of_v<std::in_place_type_t, Concrete>) &&
                (!std::same_as<std::remove_cvref_t<Concrete>, any_unique>) &&
                std::constructible_from<std::remove_cvref_t<Concrete>,
                                        Concrete> &&
                detail::is_type_erasable<Concrete, CPOs...>
    any_unique(Concrete &&concrete)
      : any_unique(std::in_place_type<std::remove_cvref_t<Concrete>>,
                   (Concrete &&) concrete)
    {}

    any_unique(any_unique &&other) noexcept
      : impl_(bl::exchange(other.impl_, nullptr)), vtable_(other.vtable_)
    {}

    ~any_unique()
    {
        if (impl_ != nullptr) {
            auto *deallocateFn =
              vtable_->template get<detail::deallocate_cpo>();
            deallocateFn(detail::deallocate_cpo{}, impl_);
        }
    }

private:
    using vtable_holder_t =
      detail::vtable_holder<detail::deallocate_cpo, CPOs...>;

    template<typename Concrete, typename Allocator>
    struct concrete_impl final
      : private detail::with_forwarding_tag_invoke<
          concrete_impl<Concrete, Allocator>,
          CPOs>...
    {
        using allocator_type = typename std::allocator_traits<
          Allocator>::template rebind_alloc<concrete_impl>;

        template<typename... Args>
        explicit concrete_impl(
          std::allocator_arg_t,
          allocator_type alloc,
          Args &&...args) noexcept(std::
                                     is_nothrow_move_constructible_v<
                                       allocator_type> &&
                                       std::is_nothrow_constructible_v<Concrete,
                                                                       Args...>)
          : value_((Args &&) args...), alloc_(bl::move(alloc))
        {}

        friend void tag_invoke(detail::deallocate_cpo,
                               concrete_impl &&impl) noexcept
        {
            allocator_type allocCopy = bl::move(impl.alloc_);
            impl.~concrete_impl();
            std::allocator_traits<allocator_type>::deallocate(
              allocCopy, bl::addressof(impl), 1);
        }

        Concrete value_;
        allocator_type alloc_;

        BL_inline constexpr const Concrete &value_ref() const noexcept
        {
            return value_;
        }

        BL_inline constexpr Concrete &value_ref() noexcept { return value_; }
    };

    template<typename Derived, typename CPO, typename Sig>
    friend struct detail::with_type_erased_tag_invoke;

    const vtable_holder_t &get_vtable() const noexcept { return vtable_; }

    [[nodiscard]] void *get_object_address() const noexcept { return impl_; }

    void *impl_;
    vtable_holder_t vtable_;
};

template<auto &...CPOs>
using any_unique_t = any_unique<bl::tag_t<CPOs>...>;

template<typename... CPOs>
class any_ref
  : private detail::with_type_erased_tag_invoke<any_ref<CPOs...>, CPOs>...
{
public:
    template<typename Concrete>
        requires detail::is_type_erasable<Concrete, CPOs...>
    explicit any_ref(std::in_place_type_t<Concrete>, Concrete &concrete)
      : vtable_(vtable_holder_t::template create<concrete_impl<Concrete>>())
    {
        ::new (&impl_data_) concrete_impl<Concrete>{ concrete };
    }

    template<typename Concrete>
        requires(!detail::is_instance_of_v<std::in_place_type_t, Concrete>) &&
                (!std::same_as<std::remove_cvref_t<Concrete>, any_ref>) &&
                detail::is_type_erasable<Concrete, CPOs...>
    any_ref(Concrete &concrete)
      : any_ref(std::in_place_type<std::remove_cvref_t<Concrete>>, concrete)
    {}

    any_ref(any_ref &&other) noexcept
      : impl_data_(
          bl::exchange(other.impl_data_, concrete_impl_base{ nullptr }))
      , vtable_(other.vtable_)
    {}

    any_ref(const any_ref &rhs) = default;

    any_ref &operator=(const any_ref &rhs) = default;

    any_ref &operator=(any_ref &&rhs) noexcept
    {
        impl_data_ =
          bl::exchange(rhs.impl_data_, concrete_impl_base{ nullptr });
        vtable_ = rhs.vtable_;
        return *this;
    }

    ~any_ref() = default;

private:
    using vtable_holder_t = detail::vtable_holder<CPOs...>;

    struct concrete_impl_base
    {
        void *value_;
    };

    template<typename Concrete>
    struct concrete_impl final
      : public concrete_impl_base
      , private detail::with_forwarding_tag_invoke<concrete_impl<Concrete>,
                                                   CPOs>...
    {
        explicit concrete_impl(Concrete &value) noexcept
          : concrete_impl_base{ static_cast<void *>(bl::addressof(value)) }
        {}

        BL_inline constexpr const Concrete &value_ref() const noexcept
        {
            return *static_cast<const Concrete *>(concrete_impl_base::value_);
        }
        BL_inline constexpr Concrete &value_ref() noexcept
        {
            return *static_cast<Concrete *>(concrete_impl_base::value_);
        }
    };

    template<typename Derived, typename CPO, typename Sig>
    friend struct detail::with_type_erased_tag_invoke;

    const vtable_holder_t &get_vtable() const noexcept { return vtable_; }

    [[nodiscard]] void *get_object_address() const noexcept
    {
        return static_cast<void *>(
          const_cast<concrete_impl_base *>(bl::addressof(impl_data_)));
    }

    concrete_impl_base impl_data_;
    vtable_holder_t vtable_;
};

template<auto &...CPOs>
using any_ref_t = any_ref<bl::tag_t<CPOs>...>;

// NOLINTEND(google-readability-casting)
BL_DIAG_POP

} // namespace bl

#endif // BL_ANY_UNIQUE_HPP
