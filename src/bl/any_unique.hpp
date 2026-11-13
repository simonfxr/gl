#ifndef BL_ANY_UNIQUE_HPP
#define BL_ANY_UNIQUE_HPP

#include <bl/any_ref.hpp>

#include <memory>

/// Code adapted from the tag_invoke paper (P1895R0)
/// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
/// godbolt link from paper: https://godbolt.org/z/3TvO4f

namespace bl {

BL_DIAG_PUSH
BL_DIAG_IGN_CLANG("-Wold-style-cast")
// NOLINTBEGIN(google-readability-casting)

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
                        bl::in_place_type_t<Concrete>,
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
                  !detail::is_instance_of_v<bl::in_place_type_t,
                                            std::decay_t<Concrete>>) &&
                 detail::is_type_erasable<Concrete, CPOs...>
    any_unique(Concrete &&concrete, Allocator alloc)
      : any_unique(std::allocator_arg,
                   bl::move(alloc),
                   bl::in_place_type<std::remove_cvref_t<Concrete>>,
                   (Concrete &&) concrete)
    {}

    template<typename Concrete, typename... Args>
        requires std::constructible_from<Concrete, Args...> &&
                 detail::is_type_erasable<Concrete, CPOs...>
    explicit any_unique(bl::in_place_type_t<Concrete> tag, Args &&...args)
      : any_unique(std::allocator_arg,
                   std::allocator<unsigned char>{},
                   tag,
                   (Args &&) args...)
    {}

    template<typename Concrete>
        requires(!detail::is_instance_of_v<bl::in_place_type_t, Concrete>) &&
                (!std::same_as<std::remove_cvref_t<Concrete>, any_unique>) &&
                std::constructible_from<std::remove_cvref_t<Concrete>,
                                        Concrete> &&
                detail::is_type_erasable<Concrete, CPOs...>
    any_unique(Concrete &&concrete)
      : any_unique(bl::in_place_type<std::remove_cvref_t<Concrete>>,
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

    any_unique &operator=(any_unique &&rhs) noexcept
    {
        auto old = bl::move(*this);
        bl::swap(impl_, rhs.impl_);
        bl::swap(vtable_, rhs.vtable_);
        return *this;
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

// NOLINTEND(google-readability-casting)
BL_DIAG_POP

} // namespace bl

#endif // BL_ANY_UNIQUE_HPP
