#ifndef GE_KEY_BINDING_HPP
#define GE_KEY_BINDING_HPP

#include "defs.h"

#include "data/Array.hpp"

#include <SFML/Window/Event.hpp>

#include <istream>

namespace ge {

enum KeyState {
    Pressed,
    Down,
    Up,
    Released
};

namespace keycode {

// stolen from SFML
enum KeyCode {
    A = 'a',      ///< The A key
    B = 'b',      ///< The B key
    C = 'c',      ///< The C key
    D = 'd',      ///< The D key
    E = 'e',      ///< The E key
    F = 'f',      ///< The F key
    G = 'g',      ///< The G key
    H = 'h',      ///< The H key
    I = 'i',      ///< The I key
    J = 'j',      ///< The J key
    K = 'k',      ///< The K key
    L = 'l',      ///< The L key
    M = 'm',      ///< The M key
    N = 'n',      ///< The N key
    O = 'o',      ///< The O key
    P = 'p',      ///< The P key
    Q = 'q',      ///< The Q key
    R = 'r',      ///< The R key
    S = 's',      ///< The S key
    T = 't',      ///< The T key
    U = 'u',      ///< The U key
    V = 'v',      ///< The V key
    W = 'w',      ///< The W key
    X = 'x',      ///< The X key
    Y = 'y',      ///< The Y key
    Z = 'z',      ///< The Z key
    Num0 = '0',   ///< The 0 key
    Num1 = '1',   ///< The 1 key
    Num2 = '2',   ///< The 2 key
    Num3 = '3',   ///< The 3 key
    Num4 = '4',   ///< The 4 key
    Num5 = '5',   ///< The 5 key
    Num6 = '6',   ///< The 6 key
    Num7 = '7',   ///< The 7 key
    Num8 = '8',   ///< The 8 key
    Num9 = '9',   ///< The 9 key
    Escape = 256, ///< The Escape key
    LControl,     ///< The left Control key
    LShift,       ///< The left Shift key
    LAlt,         ///< The left Alt key
    LSystem,      ///< The left OS specific key : windows (Windows and Linux), apple (MacOS X), ...
    RControl,     ///< The right Control key
    RShift,       ///< The right Shift key
    RAlt,         ///< The right Alt key
    RSystem,      ///< The right OS specific key : windows (Windows and Linux), apple (MacOS X), ...
    Menu,         ///< The Menu key
    LBracket,     ///< The [ key
    RBracket,     ///< The ] key
    SemiColon,    ///< The ; key
    Comma,        ///< The , key
    Period,       ///< The . key
    Quote,        ///< The ' key
    Slash,        ///< The / key
    BackSlash,    ///< The \ key
    Tilde,        ///< The ~ key
    Equal,        ///< The = key
    Dash,         ///< The - key
    Space,        ///< The Space key
    Return,       ///< The Return key
    Back,         ///< The Backspace key
    Tab,          ///< The Tabulation key
    PageUp,       ///< The Page up key
    PageDown,     ///< The Page down key
    End,          ///< The End key
    Home,         ///< The Home key
    Insert,       ///< The Insert key
    Delete,       ///< The Delete key
    Add,          ///< +
    Subtract,     ///< -
    Multiply,     ///< *
    Divide,       ///< /
    Left,         ///< Left arrow
    Right,        ///< Right arrow
    Up,           ///< Up arrow
    Down,         ///< Down arrow
    Numpad0,      ///< The numpad 0 key
    Numpad1,      ///< The numpad 1 key
    Numpad2,      ///< The numpad 2 key
    Numpad3,      ///< The numpad 3 key
    Numpad4,      ///< The numpad 4 key
    Numpad5,      ///< The numpad 5 key
    Numpad6,      ///< The numpad 6 key
    Numpad7,      ///< The numpad 7 key
    Numpad8,      ///< The numpad 8 key
    Numpad9,      ///< The numpad 9 key
    F1,           ///< The F1 key
    F2,           ///< The F2 key
    F3,           ///< The F3 key
    F4,           ///< The F4 key
    F5,           ///< The F5 key
    F6,           ///< The F6 key
    F7,           ///< The F7 key
    F8,           ///< The F8 key
    F9,           ///< The F8 key
    F10,          ///< The F10 key
    F11,          ///< The F11 key
    F12,          ///< The F12 key
    F13,          ///< The F13 key
    F14,          ///< The F14 key
    F15,          ///< The F15 key
    Pause,        ///< The Pause key
    KeyCount,

    MLeft,        ///< The left mouse button
    MRight,       ///< The right mouse button
    MMiddle,      ///< The middle (wheel) mouse button
    MXButton1,    ///< The first extra mouse button
    MXButton2,    ///< The second extra mouse button
    
    Count         ///< Keep last -- the total number of keyboard keys
};

} // namespace keycode

typedef keycode::KeyCode KeyCode;

struct Key {
    KeyState state;
    KeyCode code;
};

typedef Array<Key> KeyBinding;

KeyCode fromSFML(sf::Key::Code key);

KeyCode fromSFML(sf::Mouse::Button button);

const char *prettyKeyCode(KeyCode code);

bool parseKeyCode(const std::string& str, KeyCode *code);

int compareKeyBinding(const KeyBinding& x, const KeyBinding& y);

} // namespace ge

#endif
