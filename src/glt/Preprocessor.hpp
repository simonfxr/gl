#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "bl/hashtable.hpp"
#include "glt/conf.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

namespace glt {

struct GLT_API Preprocessor
{

    struct ContentContext
    {
        Preprocessor &processor;
        const bl::string name;
        const char *data;
        size_t size;

        ContentContext(Preprocessor &proc, bl::string &&nam)
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

        DirectiveContext(Preprocessor &proc, bl::string &&name)
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

    typedef bl::hashtable<bl::string, DirectiveHandler *> Handlers;

    Preprocessor();

    const bl::string &name() const;
    void name(bl::string &&name);

    void process(bl::string_view);

    void process(const char *contents) { process(bl::string_view(contents)); }

    DirectiveHandler &defaultHandler(DirectiveHandler &handler);
    DirectiveHandler &defaultHandler() const;

    Handlers &handlers() const;

    DirectiveHandler *installHandler(const bl::string &directive,
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
