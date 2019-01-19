#ifndef GE_KEY_BINDING_HPP
#define GE_KEY_BINDING_HPP

#include "ge/conf.hpp"

#include "pp/enum.hpp"
#include "util/Comparable.hpp"

#include <optional>
#include <string>
#include <vector>

namespace ge {

enum class KeyState : uint8_t
{
    Pressed = 3,  // 0011
    Down = 1,     // 0001
    Up = 4,       // 0100
    Released = 12 // 1100
};

// clang-format off
#define GE_KEY_CODE_ENUM_DEF(TY, __, _) \
  TY(KeyCode, uint8_t,  \
    __(    A         ) \
     _(    B         ) \
     _(    C         ) \
     _(    D         ) \
     _(    E         ) \
     _(    F         ) \
     _(    G         ) \
     _(    H         ) \
     _(    I         ) \
     _(    J         ) \
     _(    K         ) \
     _(    L         ) \
     _(    M         ) \
     _(    N         ) \
     _(    O         ) \
     _(    P         ) \
     _(    Q         ) \
     _(    R         ) \
     _(    S         ) \
     _(    T         ) \
     _(    U         ) \
     _(    V         ) \
     _(    W         ) \
     _(    X         ) \
     _(    Y         ) \
     _(    Z         ) \
     _(    Num0      ) \
     _(    Num1      ) \
     _(    Num2      ) \
     _(    Num3      ) \
     _(    Num4      ) \
     _(    Num5      ) \
     _(    Num6      ) \
     _(    Num7      ) \
     _(    Num8      ) \
     _(    Num9      ) \
     _(    Escape    ) \
     _(    LControl  ) \
     _(    LShift    ) \
     _(    LAlt      ) \
     _(    LSystem   ) \
     _(    RControl  ) \
     _(    RShift    ) \
     _(    RAlt      ) \
     _(    RSystem   ) \
     _(    Menu      ) \
     _(    LBracket  ) \
     _(    RBracket  ) \
     _(    SemiColon ) \
     _(    Comma     ) \
     _(    Period    ) \
     _(    Quote     ) \
     _(    Slash     ) \
     _(    BackSlash ) \
     _(    Tilde     ) \
     _(    Equal     ) \
     _(    Dash      ) \
     _(    Space     ) \
     _(    Return    ) \
     _(    Back      ) \
     _(    Tab       ) \
     _(    PageUp    ) \
     _(    PageDown  ) \
     _(    End       ) \
     _(    Home      ) \
     _(    Insert    ) \
     _(    Delete    ) \
     _(    Add       ) \
     _(    Subtract  ) \
     _(    Multiply  ) \
     _(    Divide    ) \
     _(    Left      ) \
     _(    Right     ) \
     _(    Up        ) \
     _(    Down      ) \
     _(    Numpad0   ) \
     _(    Numpad1   ) \
     _(    Numpad2   ) \
     _(    Numpad3   ) \
     _(    Numpad4   ) \
     _(    Numpad5   ) \
     _(    Numpad6   ) \
     _(    Numpad7   ) \
     _(    Numpad8   ) \
     _(    Numpad9   ) \
     _(    F1        ) \
     _(    F2        ) \
     _(    F3        ) \
     _(    F4        ) \
     _(    F5        ) \
     _(    F6        ) \
     _(    F7        ) \
     _(    F8        ) \
     _(    F9        ) \
     _(    F10       ) \
     _(    F11       ) \
     _(    F12       ) \
     _(    F13       ) \
     _(    F14       ) \
     _(    F15       ) \
     _(    Pause     ) \
     _(    KeyCount  ) \
     _(    MLeft     ) \
     _(    MRight    ) \
     _(    MMiddle   ) \
     _(    MXButton1 ) \
     _(    MXButton2 ))

// clang-format off

PP_DEF_ENUM_WITH_API(GE_API, GE_KEY_CODE_ENUM_DEF);

struct Key : Comparable<Key>
{
    KeyCode code{};
    KeyState state{};
    constexpr Key() = default;
    constexpr Key(KeyState st, KeyCode c) : code(c), state(st) {}
};

inline int
compare(const Key &a, const Key &b)
{
    using ::compare;
    return chained_compare([&]() { return compare(a.code, b.code); },
                           [&]() { return compare(a.state, b.state); });
}

using KeyBinding = std::vector<Key>;

HU_NODISCARD GE_API  std::optional<KeyCode>
parseKeyCode(const std::string_view &str);

GE_API int
compareKeyBinding(const KeyBinding &x, const KeyBinding &y);

} // namespace ge

#endif
