#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "glt/conf.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

#include <memory>
#include <unordered_map>

namespace glt {

struct GLT_API Preprocessor
{

    struct ContentContext
    {
        Preprocessor &processor;
        const std::string name;
        const char *data;
        size_t size;

        ContentContext(Preprocessor &proc, std::string &&nam)
          : processor(proc), name(std::move(nam))
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

        DirectiveContext(Preprocessor &proc, std::string &&name)
          : content(proc, std::move(name))
        {}
    };

    struct GLT_API DirectiveHandler
    {
        virtual ~DirectiveHandler();
        virtual void beginProcessing(const ContentContext &ctx);
        virtual void endProcessing(const ContentContext &ctx);
        virtual void directiveEncountered(const DirectiveContext &ctx);
    };

    using Handlers = std::unordered_map<std::string, DirectiveHandler *>;
    using HandlerEntry = std::pair<std::string, DirectiveHandler *>;

    Preprocessor();

    const std::string &name() const;
    void name(std::string &&name);

    void process(std::string_view);

    void process(const char *contents) { process(std::string_view(contents)); }

    DirectiveHandler &defaultHandler(DirectiveHandler &handler);
    DirectiveHandler &defaultHandler() const;

    Handlers &handlers() const;

    DirectiveHandler *installHandler(const std::string &directive,
                                     DirectiveHandler &handler);

    sys::io::OutStream &out();
    void out(sys::io::OutStream &out);

    bool setError();
    bool wasError() const;
    bool clearError();

    //    static DirectiveHandler& nullHandler();

private:
    DECLARE_PIMPL(GLT_API, self);
};

} // namespace glt

#endif
