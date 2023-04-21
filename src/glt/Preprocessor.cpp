#include "glt/Preprocessor.hpp"

#include "err/err.hpp"

#include <cstring>

namespace glt {

Preprocessor::DirectiveHandler::~DirectiveHandler() = default;

void
Preprocessor::DirectiveHandler::beginProcessing(
  const Preprocessor::ContentContext & /*unused*/)
{}

void
Preprocessor::DirectiveHandler::endProcessing(
  const Preprocessor::ContentContext & /*unused*/)
{}

void
Preprocessor::DirectiveHandler::directiveEncountered(
  const Preprocessor::DirectiveContext & /*unused*/)
{}

struct Preprocessor::Impl : Preprocessor
{
    Preprocessor::DirectiveHandler nullHandler;
    Preprocessor::DirectiveHandler *defaultHandler;
    sys::io::OutStream *out;
    Preprocessor::Handlers handlers;
    bl::string sourceName;
    bool errorState{ false };

    Impl() : defaultHandler(&nullHandler), out(&sys::io::stdout()) {}
};

BL_DEFINE_RUNTIME_SIZED(Preprocessor, Preprocessor::Impl);

Preprocessor &
Preprocessor::construct_at(void *p) noexcept
{
    return construct_impl(p);
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

    ctx.content.data = begin;
    ctx.content.size = size;

    for (auto &handler : m().handlers) {
        handler.second->beginProcessing(ctx.content);
    }

    m().defaultHandler->beginProcessing(ctx.content);

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
            it->second->directiveEncountered(ctx);
        else
            m().defaultHandler->directiveEncountered(ctx);
    }

    for (auto &handler : m().handlers)
        handler.second->endProcessing(ctx.content);

    m().defaultHandler->endProcessing(ctx.content);
}

Preprocessor::DirectiveHandler &
Preprocessor::defaultHandler(Preprocessor::DirectiveHandler &handler)
{
    DirectiveHandler &old = *m().defaultHandler;
    m().defaultHandler = &handler;
    return old;
}

Preprocessor::DirectiveHandler &
Preprocessor::defaultHandler() const
{
    return *m().defaultHandler;
}

Preprocessor::Handlers &
Preprocessor::handlers()
{
    return m().handlers;
}

Preprocessor::DirectiveHandler *
Preprocessor::installHandler(const bl::string &directive,
                             DirectiveHandler &handler)
{
    DirectiveHandler *old = nullptr;
    auto it = m().handlers.find(directive);
    if (it != m().handlers.end()) {
        old = it->second;
    }

    m().handlers.insert(HandlerEntry(directive, &handler));
    return old;
}

sys::io::OutStream &
Preprocessor::out()
{
    return *m().out;
}

void
Preprocessor::out(sys::io::OutStream &out)
{
    m().out = &out;
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
