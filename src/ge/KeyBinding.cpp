#include "ge/KeyBinding.hpp"

#include "data/range.hpp"
#include "ge/module.hpp"

#include <algorithm>
#include <unordered_map>

namespace ge {

struct KeyBindingState::Data
{
    const char *table[keycode::Count]{};
    std::unordered_map<std::string, KeyCode> revtable;

    Data();
};

DECLARE_PIMPL_DEL(KeyBindingState)

KeyBindingState::KeyBindingState() : self(new Data) {}

#define global (*module->key_binding.self)

const char *
keycodeStrings(KeyCode code)
{
#define K(k)                                                                   \
    case keycode::k:                                                           \
        return #k
    switch (code) {
        K(A);         ///< The A key
        K(B);         ///< The B key
        K(C);         ///< The C key
        K(D);         ///< The D key
        K(E);         ///< The E key
        K(F);         ///< The F key
        K(G);         ///< The G key
        K(H);         ///< The H key
        K(I);         ///< The I key
        K(J);         ///< The J key
        K(K);         ///< The K key
        K(L);         ///< The L key
        K(M);         ///< The M key
        K(N);         ///< The N key
        K(O);         ///< The O key
        K(P);         ///< The P key
        K(Q);         ///< The Q key
        K(R);         ///< The R key
        K(S);         ///< The S key
        K(T);         ///< The T key
        K(U);         ///< The U key
        K(V);         ///< The V key
        K(W);         ///< The W key
        K(X);         ///< The X key
        K(Y);         ///< The Y key
        K(Z);         ///< The Z key
        K(Num0);      ///< The 0 key
        K(Num1);      ///< The 1 key
        K(Num2);      ///< The 2 key
        K(Num3);      ///< The 3 key
        K(Num4);      ///< The 4 key
        K(Num5);      ///< The 5 key
        K(Num6);      ///< The 6 key
        K(Num7);      ///< The 7 key
        K(Num8);      ///< The 8 key
        K(Num9);      ///< The 9 key
        K(Escape);    ///< The Escape key
        K(LControl);  ///< The left Control key
        K(LShift);    ///< The left Shift key
        K(LAlt);      ///< The left Alt key
        K(LSystem);   ///< The left OS specific key : windows (Windows and
                      ///< Linux)); apple (MacOS X)); ...
        K(RControl);  ///< The right Control key
        K(RShift);    ///< The right Shift key
        K(RAlt);      ///< The right Alt key
        K(RSystem);   ///< The right OS specific key : windows (Windows and
                      ///< Linux)); apple (MacOS X)); ...
        K(Menu);      ///< The Menu key
        K(LBracket);  ///< The [ key
        K(RBracket);  ///< The ] key
        K(SemiColon); ///< The ; key
        K(Comma);     ///< The ); key
        K(Period);    ///< The . key
        K(Quote);     ///< The ' key
        K(Slash);     ///< The / key
        K(BackSlash); ///< The \ key
        K(Tilde);     ///< The ~ key
        K(Equal);     ///< The = key
        K(Dash);      ///< The - key
        K(Space);     ///< The Space key
        K(Return);    ///< The Return key
        K(Back);      ///< The Backspace key
        K(Tab);       ///< The Tabulation key
        K(PageUp);    ///< The Page up key
        K(PageDown);  ///< The Page down key
        K(End);       ///< The End key
        K(Home);      ///< The Home key
        K(Insert);    ///< The Insert key
        K(Delete);    ///< The Delete key
        K(Add);       ///< +
        K(Subtract);  ///< -
        K(Multiply);  ///< *
        K(Divide);    ///< /
        K(Left);      ///< Left arrow
        K(Right);     ///< Right arrow
        K(Up);        ///< Up arrow
        K(Down);      ///< Down arrow
        K(Numpad0);   ///< The numpad 0 key
        K(Numpad1);   ///< The numpad 1 key
        K(Numpad2);   ///< The numpad 2 key
        K(Numpad3);   ///< The numpad 3 key
        K(Numpad4);   ///< The numpad 4 key
        K(Numpad5);   ///< The numpad 5 key
        K(Numpad6);   ///< The numpad 6 key
        K(Numpad7);   ///< The numpad 7 key
        K(Numpad8);   ///< The numpad 8 key
        K(Numpad9);   ///< The numpad 9 key
        K(F1);        ///< The F1 key
        K(F2);        ///< The F2 key
        K(F3);        ///< The F3 key
        K(F4);        ///< The F4 key
        K(F5);        ///< The F5 key
        K(F6);        ///< The F6 key
        K(F7);        ///< The F7 key
        K(F8);        ///< The F8 key
        K(F9);        ///< The F8 key
        K(F10);       ///< The F10 key
        K(F11);       ///< The F11 key
        K(F12);       ///< The F12 key
        K(F13);       ///< The F13 key
        K(F14);       ///< The F14 key
        K(F15);       ///< The F15 key
        K(Pause);     ///< The Pause key
        K(KeyCount);

        K(MLeft);     ///< The left mouse button
        K(MRight);    ///< The right mouse button
        K(MMiddle);   ///< The middle (wheel) mouse button
        K(MXButton1); ///< The first extra mouse button
        K(MXButton2); ///< The second extra mouse button

        K(Count); ///< Keep last -- the total number of keyboard keys
    }
    return nullptr;
#undef K
}

KeyBindingState::Data::Data()
{
    for (const auto i : irange(int(keycode::Count)))
        table[i] = keycodeStrings(KeyCode(i));

    for (const auto i : irange(int(keycode::Count)))
        if (table[i] != nullptr)
            revtable[table[i]] = KeyCode(i);
}

std::optional<KeyCode>
parseKeyCode(const std::string &str)
{
    auto it = global.revtable.find(str);
    if (it != global.revtable.end())
        return { it->second };
    return std::nullopt;
}

const char *
to_string(KeyCode code)
{
    int idx = int(code);
    if (idx < 0 || idx >= KeyCode::Count)
        return nullptr;
    return global.table[idx];
}

int
compareKeyBinding(const KeyBinding &x, const KeyBinding &y)
{
    for (const auto i : irange(std::min(x.size(), y.size()))) {
        auto a = int32_t(y[i].code);
        auto b = int32_t(x[i].code);
        if (a != b) {
            if (a < b)
                return -1;
            return 1;
        }
    }

    return x.size() < y.size() ? -1 : x.size() > y.size() ? 1 : 0;
}

} // namespace ge
