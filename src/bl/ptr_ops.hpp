#ifndef BL_PTR_OPS_HPP
#define BL_PTR_OPS_HPP

#include <bl/ptr_traits.hpp>

namespace bl {

template<typename P, typename T, bool is_nullable_, bool is_owned_>
struct ptr_ops
{
    static inline constexpr bool is_nullable = is_nullable_;
    static inline constexpr bool is_owned = is_owned_;
    using element_type = T;

    BL_wrapper ptr_ops() noexcept = default;
    BL_wrapper ptr_ops(const ptr_ops &) noexcept = default;
    BL_wrapper ptr_ops(ptr_ops &&) noexcept = default;
    BL_wrapper ptr_ops &operator=(const ptr_ops &) noexcept = default;
    BL_wrapper ptr_ops &operator=(ptr_ops &&) noexcept = default;
    BL_wrapper ~ptr_ops() = default;

    // NOLINTNEXTLINE(misc-unconventional-assign-operator,cppcoreguidelines-c-copy-assignment-signature)
    BL_wrapper constexpr P &operator=(element_type *p) noexcept
        requires(is_nullable)
    {
        return self() = P{ p };
    }

    // NOLINTNEXTLINE(misc-unconventional-assign-operator,cppcoreguidelines-c-copy-assignment-signature)
    HU_NONNULL_PARAMS()
    BL_wrapper constexpr P &operator=(element_type *p) noexcept
        requires(!is_nullable)
    {
        return self() = P{ p };
    }

    // NOLINTNEXTLINE(misc-unconventional-assign-operator,cppcoreguidelines-c-copy-assignment-signature)
    BL_wrapper P &operator=(std::nullptr_t)
        requires(is_nullable)
    {
        self() = (static_cast<element_type *>(nullptr));
        return self();
    }

    P &operator=(std::nullptr_t)
        requires(!is_nullable)
    = delete;

    BL_wrapper void reset(element_type *p) noexcept
        requires(is_nullable)
    {
        self() = p;
    }

    HU_NONNULL_PARAMS()
    BL_wrapper void reset(element_type *p) noexcept
        requires(!is_nullable)
    {
        self() = p;
    }

    BL_wrapper void reset(std::nullptr_t = nullptr) noexcept
        requires(is_nullable)
    {
        reset();
    }

    void reset(std::nullptr_t)
        requires(!is_nullable)
    = delete;

    BL_wrapper element_type *get() const noexcept
        requires(is_nullable)
    {
        return self().get_or_null();
    }

    BL_wrapper element_type *get() const noexcept
        requires(!is_nullable)
    {
        auto p = self().get_or_null();
        hu_assume(p);
        return p;
    }

    BL_wrapper element_type &operator*() const noexcept { return *get(); }

    BL_wrapper element_type *operator->() const noexcept { return get(); }

    BL_wrapper explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

    BL_wrapper explicit operator element_type &() const noexcept
        requires(!is_nullable)
    {
        return *get();
    }

    BL_wrapper explicit operator const element_type &() const noexcept
        requires(!is_nullable)
    {
        return *get();
    }

private:
    BL_wrapper const P &self() const { return static_cast<const P &>(*this); }
    BL_wrapper P &self() { return static_cast<P &>(*this); }
};

} // namespace bl

#endif // BL_PTR_OPS_HPP
