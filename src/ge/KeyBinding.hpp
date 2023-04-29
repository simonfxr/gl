#ifndef GE_KEY_BINDING_HPP
#define GE_KEY_BINDING_HPP

#include "ge/conf.hpp"

#include <bl/enum.hpp>
#include <bl/optional.hpp>
#include <bl/string.hpp>
#include <bl/string_view.hpp>
#include <bl/vec.hpp>
#include <pp/enum.hpp>

#include <compare>

namespace ge {

enum class KeyState : uint8_t
{
    Pressed = 3,  // 0011
    Down = 1,     // 0001
    Up = 4,       // 0100
    Released = 12 // 1100
};

// clang-format off
PP_DEF_ENUM(KeyCode, uint8_t,  \
       A         ,
       B         ,
       C         ,
       D         ,
       E         ,
       F         ,
       G         ,
       H         ,
       I         ,
       J         ,
       K         ,
       L         ,
       M         ,
       N         ,
       O         ,
       P         ,
       Q         ,
       R         ,
       S         ,
       T         ,
       U         ,
       V         ,
       W         ,
       X         ,
       Y         ,
       Z         ,
       Num0      ,
       Num1      ,
       Num2      ,
       Num3      ,
       Num4      ,
       Num5      ,
       Num6      ,
       Num7      ,
       Num8      ,
       Num9      ,
       Escape    ,
       LControl  ,
       LShift    ,
       LAlt      ,
       LSystem   ,
       RControl  ,
       RShift    ,
       RAlt      ,
       RSystem   ,
       Menu      ,
       LBracket  ,
       RBracket  ,
       SemiColon ,
       Comma     ,
       Period    ,
       Quote     ,
       Slash     ,
       BackSlash ,
       Tilde     ,
       Equal     ,
       Dash      ,
       Space     ,
       Return    ,
       Back      ,
       Tab       ,
       PageUp    ,
       PageDown  ,
       End       ,
       Home      ,
       Insert    ,
       Delete    ,
       Add       ,
       Subtract  ,
       Multiply  ,
       Divide    ,
       Left      ,
       Right     ,
       Up        ,
       Down      ,
       Numpad0   ,
       Numpad1   ,
       Numpad2   ,
       Numpad3   ,
       Numpad4   ,
       Numpad5   ,
       Numpad6   ,
       Numpad7   ,
       Numpad8   ,
       Numpad9   ,
       F1        ,
       F2        ,
       F3        ,
       F4        ,
       F5        ,
       F6        ,
       F7        ,
       F8        ,
       F9        ,
       F10       ,
       F11       ,
       F12       ,
       F13       ,
       F14       ,
       F15       ,
       Pause     ,
       KeyCount  ,
       MLeft     ,
       MRight    ,
       MMiddle   ,
       MXButton1 ,
       MXButton2 );
// clang-format on

struct Key
{
    KeyCode code{};
    KeyState state{};
    constexpr Key() = default;
    constexpr Key(KeyState st, KeyCode c) : code(c), state(st) {}
    friend constexpr auto operator<=>(const Key &, const Key &) = default;
};

using KeyBinding = bl::vec<Key>;

HU_NODISCARD GE_API bl::optional<KeyCode>
parseKeyCode(const bl::string_view &str);

GE_API std::strong_ordering
operator<=>(const KeyBinding &x, const KeyBinding &y) noexcept;

} // namespace ge

#endif
