#include "ge/KeyBinding.hpp"

#include "err/err.hpp"
#include "util/range.hpp"

#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace ge {

namespace {
std::unordered_map<bl::string_view, KeyCode>
make_table()
{
    std::unordered_map<bl::string_view, KeyCode> table;
    for (auto i : irange(bl::enum_count_of<KeyCode>())) {
        auto val = static_cast<KeyCode>(i);
        table[to_string(val)] = val;
    }
    return table;
}
} // namespace

std::optional<KeyCode>
parseKeyCode(const bl::string_view &str)
{
    BEGIN_NO_WARN_GLOBAL_DESTRUCTOR
    static auto table = make_table();
    END_NO_WARN_GLOBAL_DESTRUCTOR
    auto it = table.find(str);
    if (it != table.end())
        return { it->second };
    return std::nullopt;
}

std::strong_ordering
operator<=>(const KeyBinding &x, const KeyBinding &y) noexcept
{
    for (const auto i : irange(std::min(x.size(), y.size()))) {
        auto a = y[i].code;
        auto b = x[i].code;
        auto r = a <=> b;
        if (!std::is_eq(r))
            return r;
    }
    return x.size() <=> y.size();
}

} // namespace ge
