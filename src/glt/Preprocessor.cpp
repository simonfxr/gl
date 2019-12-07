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

struct Preprocessor::Data
{
    Preprocessor::DirectiveHandler nullHandler;
    Preprocessor::DirectiveHandler *defaultHandler;
    bool errorState{ false };
    sys::io::OutStream *out;
    Preprocessor::Handlers handlers;
    std::string sourceName;

    Data() : defaultHandler(&nullHandler), out(&sys::io::stdout()) {}
};

DECLARE_PIMPL_DEL(Preprocessor)

Preprocessor::Preprocessor() : self(new Data) {}

namespace {
bool
is_newline(char c)
{
    return c == '\n' || c == '\r';
}
} // namespace

void
Preprocessor::process(std::string_view data)
{
    if (wasError())
        return;

    const char *const begin = data.data();
    const size_t size = data.size();
    DirectiveContext ctx(*this, std::string(self->sourceName));

    ctx.content.data = begin;
    ctx.content.size = size;

    for (auto &handler : self->handlers) {
        handler.second->beginProcessing(ctx.content);
    }

    self->defaultHandler->beginProcessing(ctx.content);

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

        std::string key(directiveBegin, size_t(directiveEnd - directiveBegin));

        auto it = self->handlers.find(key);
        if (it != self->handlers.end())
            it->second->directiveEncountered(ctx);
        else
            self->defaultHandler->directiveEncountered(ctx);
    }

    for (auto &handler : self->handlers)
        handler.second->endProcessing(ctx.content);

    self->defaultHandler->endProcessing(ctx.content);
}

Preprocessor::DirectiveHandler &
Preprocessor::defaultHandler(Preprocessor::DirectiveHandler &handler)
{
    DirectiveHandler &old = *self->defaultHandler;
    self->defaultHandler = &handler;
    return old;
}

Preprocessor::DirectiveHandler &
Preprocessor::defaultHandler() const
{
    return *self->defaultHandler;
}

Preprocessor::Handlers &
Preprocessor::handlers() const
{
    return self->handlers;
}

Preprocessor::DirectiveHandler *
Preprocessor::installHandler(const std::string &directive,
                             DirectiveHandler &handler)
{
    DirectiveHandler *old = nullptr;
    auto it = self->handlers.find(directive);
    if (it != self->handlers.end()) {
        old = it->second;
    }

    self->handlers.insert(HandlerEntry(directive, &handler));
    return old;
}

sys::io::OutStream &
Preprocessor::out()
{
    return *self->out;
}

void
Preprocessor::out(sys::io::OutStream &out)
{
    self->out = &out;
}

bool
Preprocessor::setError()
{
    WARN("setting error");
    return std::exchange(self->errorState, true);
}

bool
Preprocessor::wasError() const
{
    if (self->errorState)
        WARN("in Error state");
    return self->errorState;
}

bool
Preprocessor::clearError()
{
    bool s = self->errorState;
    self->errorState = false;
    return s;
}

const std::string &
Preprocessor::name() const
{
    return self->sourceName;
}

void
Preprocessor::name(std::string &&name)
{
    self->sourceName = std::move(name);
}

// Preprocessor::DirectiveHandler &Preprocessor::nullHandler() {
//     return nullHandler;
// }

} // namespace glt
