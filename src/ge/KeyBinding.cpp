#include "ge/KeyBinding.hpp"

#include "util/range.hpp"
#include "err/err.hpp"

#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace ge {

PP_DEF_ENUM_IMPL(GE_KEY_CODE_ENUM_DEF)

namespace {
std::unordered_map<std::string_view, KeyCode>
make_table()
{
    std::unordered_map<std::string_view, KeyCode> table;
    for (auto i : irange(KeyCode::count)) {
        auto val = KeyCode(i);
        auto str = to_string(val);
        table[std::string_view(str, strlen(str))] = val;
    }
    return table;
}
} // namespace

std::optional<KeyCode>
parseKeyCode(const std::string_view &str)
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static auto table = make_table();
    END_NO_WARN_GLOBAL_DESTRUCTOR
    auto it = table.find(str);
    if (it != table.end())
        return { it->second };
    return std::nullopt;
}

int
compareKeyBinding(const KeyBinding &x, const KeyBinding &y)
{
    for (const auto i : irange(std::min(x.size(), y.size()))) {
        auto a = y[i].code;
        auto b = x[i].code;
        if (a != b)
            return compare(a, b);
    }
    return x.size() < y.size() ? -1 : x.size() > y.size() ? 1 : 0;
}

} // namespace ge
