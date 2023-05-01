#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

#include <bl/new.hpp>

#include <memory>
#include <unordered_map>

namespace glt {

struct GLT_API Preprocessor : bl::runtime_sized_base<Preprocessor>
{

    struct ContentContext
    {
        Preprocessor &processor;
        const bl::string name;
        const char *data;
        size_t size;

        ContentContext(Preprocessor &proc, bl::string &&nam)
          : processor(proc), name(bl::move(nam))
        {}
    };

    struct GLT_API DirectiveContext
    {
        ContentContext content;

        size_t lineLength;
        size_t lineOffset;
        size_t beginDirective; // size_t of first char in directive
        size_t endDirective; // size_t of first char behind directive, so length
                             // of directive = endDirective - beginDirective

        DirectiveContext(Preprocessor &proc, bl::string &&name)
          : content(proc, bl::move(name))
        {}
    };

    struct GLT_API DirectiveHandler
    {
        virtual ~DirectiveHandler();
        virtual void beginProcessing(const ContentContext &ctx);
        virtual void endProcessing(const ContentContext &ctx);
        virtual void directiveEncountered(const DirectiveContext &ctx);
    };

    using Handlers = std::unordered_map<bl::string, DirectiveHandler *>;
    using HandlerEntry = std::pair<bl::string, DirectiveHandler *>;

    static Preprocessor &construct_at(void *) noexcept;

    const bl::string &name() const;
    void name(bl::string &&name);

    void process(bl::string_view);

    DirectiveHandler &defaultHandler(DirectiveHandler &handler);
    DirectiveHandler &defaultHandler() const;

    Handlers &handlers();

    DirectiveHandler *installHandler(const bl::string &directive,
                                     DirectiveHandler &handler);

    sys::io::OutStream out();
    void out(sys::io::OutStream out);

    bool setError();
    bool wasError() const;
    bool clearError();

    BL_DECLARE_RUNTIME_SIZED(Preprocessor, Impl);
};

} // namespace glt

#endif
