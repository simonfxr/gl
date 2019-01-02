#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

#include <map>
#include <memory>

namespace glt {

struct GLT_API Preprocessor
{

    struct ContentContext
    {
        Preprocessor &processor;
        const std::string name;
        const char *data;
        defs::size_t size_t;

        ContentContext(Preprocessor &proc, const std::string &nam)
          : processor(proc), name(nam)
        {}
    };

    struct DirectiveContext
    {
        ContentContext content;

        defs::size_t lineLength;
        defs::index_t lineOffset;
        defs::index_t beginDirective; // index_t of first char in directive
        defs::index_t
          endDirective; // index_t of first char behind directive, so length of
                        // directive = endDirective - beginDirective

        DirectiveContext(Preprocessor &proc, const std::string &name)
          : content(proc, name)
        {}
    };

    struct DirectiveHandler
    {
        virtual ~DirectiveHandler();
        virtual void beginProcessing(const ContentContext &ctx);
        virtual void endProcessing(const ContentContext &ctx);
        virtual void directiveEncountered(const DirectiveContext &ctx);
    };

    typedef std::map<std::string, DirectiveHandler *> Handlers;
    typedef std::pair<std::string, DirectiveHandler *> HandlerEntry;

    Preprocessor();

    const std::string &name() const;
    void name(const std::string &name);

    void process(const std::string &);
    void process(const char *begin, defs::size_t size);
    void process(const char *contents);

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
    DECLARE_PIMPL(self);
};

} // namespace glt

#endif
