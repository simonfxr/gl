#include "glt/Preprocessor.hpp"

#include "err/err.hpp"

#include <cstring>
#include <utility>

namespace glt {

struct null_handler
{
    template<typename F, typename... Args>
    friend void tag_invoke(F, null_handler &, Args &&...) noexcept
    {}
};

struct Preprocessor::Impl : Preprocessor
{
    [[no_unique_address]] null_handler null_handler_instance{};
    Preprocessor::DirectiveHandler defaultHandler = null_handler_instance;
    sys::io::OutStream out;
    Preprocessor::Handlers handlers;
    bl::string sourceName;
    bool errorState{ false };

    Impl() : out(sys::io::stdout()) {}
    Impl(sys::io::OutStream out) : out(bl::move(out)) {}
};

BL_DEF_RUNTIME_SIZED(Preprocessor, Preprocessor::Impl);

Preprocessor &
Preprocessor::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

Preprocessor &
Preprocessor::construct_at(void *p, sys::io::OutStream out) noexcept
{
    return construct_impl(p, out);
}

namespace {
bool
is_newline(char c)
{
    return c == '\n' || c == '\r';
}
} // namespace

void
Preprocessor::process(bl::string_view data)
{
    if (wasError())
        return;

    const char *const begin = data.data();
    const size_t size = data.size();
    DirectiveContext ctx(*this, bl::string(m().sourceName));

    ctx.content.data = { begin, size };

    for (auto &handler : m().handlers) {
        directiveScopeChanged(handler.second, beginDirective, ctx.content);
    }

    directiveScopeChanged(m().defaultHandler, beginDirective, ctx.content);

    const char *const end = begin + size;
    const char *str = begin;
    const char *match;

    while (!wasError() &&
           (match = static_cast<const char *>(memchr(str, '#', end - str))) <
             end &&
           match != nullptr) {

        const char *lineBegin = match;
        while (lineBegin > begin && !is_newline(lineBegin[-1]) &&
               (isspace(lineBegin[-1]) != 0))
            --lineBegin;

        auto eol = match;
        while (eol < end && !is_newline(*eol))
            ++eol;

        str = eol;

        if (lineBegin > begin && !is_newline(lineBegin[-1]))
            continue;

        const char *directiveBegin = match + 1;
        while (directiveBegin < eol && isspace(*directiveBegin))
            ++directiveBegin;

        const char *directiveEnd = directiveBegin;
        while (directiveEnd < eol && !isspace(*directiveEnd))
            ++directiveEnd;

        ctx.lineLength = eol - lineBegin;
        ctx.lineOffset = lineBegin - begin;
        ctx.beginDirective = directiveBegin - begin;
        ctx.endDirective = directiveEnd - begin;

        auto key = bl::string(
          directiveBegin, static_cast<size_t>(directiveEnd - directiveBegin));

        auto it = m().handlers.find(key);
        if (it != m().handlers.end())
            directiveEncountered(it->second, ctx);
        else
            directiveEncountered(m().defaultHandler, ctx);
    }

    for (auto &handler : m().handlers)
        directiveScopeChanged(handler.second, endDirective, ctx.content);

    directiveScopeChanged(m().defaultHandler, endDirective, ctx.content);
}

Preprocessor::DirectiveHandler
Preprocessor::defaultHandler(Preprocessor::DirectiveHandler handler)
{
    return bl::exchange(m().defaultHandler, handler);
}

Preprocessor::DirectiveHandler
Preprocessor::defaultHandler() const
{
    return m().defaultHandler;
}

Preprocessor::Handlers &
Preprocessor::handlers()
{
    return m().handlers;
}

bl::optional<Preprocessor::DirectiveHandler>
Preprocessor::installHandler(bl::string_view directive,
                             DirectiveHandler handler)
{
    if (auto it = m().handlers.find(directive); it != m().handlers.end())
        return { bl::exchange(it->second, handler) };

    m().handlers.emplace(directive, handler);
    return bl::nullopt;
}

sys::io::OutStream
Preprocessor::out()
{
    return m().out;
}

void
Preprocessor::out(sys::io::OutStream out)
{
    m().out = out;
}

bool
Preprocessor::setError()
{
    WARN("setting error");
    return bl::exchange(m().errorState, true);
}

bool
Preprocessor::wasError() const
{
    if (m().errorState)
        WARN("in Error state");
    return m().errorState;
}

bool
Preprocessor::clearError()
{
    return bl::exchange(m().errorState, false);
}

const bl::string &
Preprocessor::name() const
{
    return m().sourceName;
}

void
Preprocessor::name(bl::string &&name)
{
    m().sourceName = bl::move(name);
}

// Preprocessor::DirectiveHandler &Preprocessor::nullHandler() {
//     return nullHandler;
// }

} // namespace glt
