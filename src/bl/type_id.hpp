#ifndef BL_TYPE_ID_HPP
#define BL_TYPE_ID_HPP

#include <bl/string_view.hpp>

namespace bl {

struct type_id
{
    template<typename T>
    BL_consteval static type_id of() noexcept
    {
        static constexpr bl::string_view type_name =
          bl::string_view_literal(__PRETTY_FUNCTION__);
        return type_id{ &type_name };
    }

    type_id() = delete;

    constexpr type_id(const type_id &) noexcept = default;
    constexpr type_id(type_id &&) noexcept = default;
    constexpr type_id &operator=(const type_id &) noexcept = default;
    constexpr type_id &operator=(type_id &&) noexcept = default;

    friend constexpr bool operator==(const type_id &lhs,
                                     const type_id &rhs) noexcept = default;

    friend constexpr auto operator<=>(const type_id &lhs,
                                      const type_id &rhs) noexcept = default;

    constexpr operator bl::string_view() const noexcept { return *_name; }

private:
    BL_consteval explicit type_id(const bl::string_view *name) : _name(name){};
    const bl::string_view *_name;
};

} // namespace bl

#endif // BL_TYPE_ID_HPP
