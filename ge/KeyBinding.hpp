#ifndef GE_KEY_BINDING_HPP
#define GE_KEY_BINDING_HPP

#include <SFML/Window/Event.hpp>

namespace ge {

enum KeyState {
    Pressed,
    Down,
    Up,
    Released
};

struct Key {
    KeyState state;
    sf::Key::Code key;
};

struct KeyBinding {
    Array<Key> keys;
    bool fired;
    bool mayFire;
};

} // namespace ge

#endif
