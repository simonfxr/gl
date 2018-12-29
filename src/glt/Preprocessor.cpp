#include <cstring>

#include "glt/Preprocessor.hpp"

#include "err/err.hpp"

namespace glt {

Preprocessor::DirectiveHandler::~DirectiveHandler() = default;

void
Preprocessor::DirectiveHandler::beginProcessing(
  const Preprocessor::ContentContext &ctx)
{
    UNUSED(ctx);
}

void
Preprocessor::DirectiveHandler::endProcessing(
  const Preprocessor::ContentContext &ctx)
{
    UNUSED(ctx);
}

void
Preprocessor::DirectiveHandler::directiveEncountered(
  const Preprocessor::DirectiveContext &ctx)
{
    UNUSED(ctx);
}

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

Preprocessor::Preprocessor() : self(new Data) {}

Preprocessor::~Preprocessor()
{
    delete self;
}

void
Preprocessor::process(const std::string &str)
{
    process(str.data(), uint32(str.length()));
}

void
Preprocessor::process(const char *begin, uint32 size)
{

    if (wasError())
        return;

    DirectiveContext ctx(*this, self->sourceName);

    ctx.content.data = begin;
    ctx.content.size = size;

    for (auto it = self->handlers.begin(); it != self->handlers.end(); ++it) {
        it->second->beginProcessing(ctx.content);
    }

    self->defaultHandler->beginProcessing(ctx.content);

    const char *const end = begin + size;
    const char *str = begin;
    const char *match;

    while (!wasError() && (match = strchr(str, '#')) < end &&
           match != nullptr) {

        const char *lineBegin = match;
        while (lineBegin > begin && lineBegin[-1] != '\n' &&
               (isspace(lineBegin[-1]) != 0))
            --lineBegin;

        const char *eol = strchr(match, '\n');
        if (eol == nullptr)
            eol = end;

        str = eol;

        if (lineBegin > begin && lineBegin[-1] != '\n')
            continue;

        const char *directiveBegin = match + 1;
        while (directiveBegin < eol && (isspace(*directiveBegin) != 0))
            ++directiveBegin;

        const char *directiveEnd = directiveBegin;
        while (directiveEnd < eol && (isspace(*directiveEnd) == 0))
            ++directiveEnd;

        ctx.lineLength = SIZE(eol - lineBegin);
        ctx.lineOffset = SIZE(lineBegin - begin);
        ctx.beginDirective = SIZE(directiveBegin - begin);
        ctx.endDirective = SIZE(directiveEnd - begin);

        std::string key(directiveBegin, size_t(directiveEnd - directiveBegin));

        auto it = self->handlers.find(key);
        if (it != self->handlers.end())
            it->second->directiveEncountered(ctx);
        else
            self->defaultHandler->directiveEncountered(ctx);
    }

    for (auto it = self->handlers.begin(); it != self->handlers.end(); ++it) {
        it->second->endProcessing(ctx.content);
    }

    self->defaultHandler->endProcessing(ctx.content);
}

void
Preprocessor::process(const char *contents)
{
    process(contents, uint32(strlen(contents)));
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
    bool isError = self->errorState;
    self->errorState = true;
    return isError;
}

bool
Preprocessor::wasError() const
{
    if (self->errorState) {
        WARN("in Error state");
    }

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
Preprocessor::name(const std::string &name)
{
    self->sourceName = name;
}

// Preprocessor::DirectiveHandler &Preprocessor::nullHandler() {
//     return nullHandler;
// }

} // namespace glt
