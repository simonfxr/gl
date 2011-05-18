#ifndef GE_KEY_BINDING_HPP
#define GE_KEY_BINDING_HPP

#include <SFML/Window/Event.hpp>

namespace ge {

enum KeyState {
    Pressed,
    Released,
    Down
};

struct Key {
    KeyState state;
    sf::Key::Code key;
};

struct KeyBinding {

};

} // namespace ge

#endif
