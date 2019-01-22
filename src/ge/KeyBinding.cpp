#include "ge/KeyBinding.hpp"

#include "bl/hashtable.hpp"
#include "bl/range.hpp"
#include "err/err.hpp"

#include <algorithm>
#include <cstring>

namespace ge {

PP_DEF_ENUM_IMPL(GE_KEY_CODE_ENUM_DEF)

namespace {
bl::hashtable<bl::string_view, KeyCode>
make_table()
{
    bl::hashtable<bl::string_view, KeyCode> table;
    for (auto i : bl::irange(KeyCode::count)) {
        auto val = KeyCode(i);
        auto str = to_string(val);
        table.insert(bl::string_view(str, strlen(str)), val);
    }
    return table;
}
} // namespace

bl::optional<KeyCode>
parseKeyCode(const bl::string_view &str)
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static auto table = make_table();
    END_NO_WARN_GLOBAL_DESTRUCTOR
    auto it = table.find(str);
    if (it != table.end())
        return { it->value };
    return bl::nullopt;
}

int
compareKeyBinding(const KeyBinding &x, const KeyBinding &y)
{
    for (const auto i : bl::irange(std::min(x.size(), y.size()))) {
        auto a = y[i].code;
        auto b = x[i].code;
        if (a != b)
            return compare(a, b);
    }
    return x.size() < y.size() ? -1 : x.size() > y.size() ? 1 : 0;
}

} // namespace ge
