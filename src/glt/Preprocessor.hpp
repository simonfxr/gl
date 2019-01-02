#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

#include <map>
#include <memory>

namespace glt {

using namespace defs;

struct GLT_API Preprocessor
{

    struct ContentContext
    {
        Preprocessor &processor;
        const std::string name;
        const char *data;
        size_t size;

        ContentContext(Preprocessor &proc, const std::string &nam)
          : processor(proc), name(nam)
        {}
    };

    struct DirectiveContext
    {
        ContentContext content;

        defs::size lineLength;
        defs::index lineOffset;
        defs::index beginDirective; // index of first char in directive
        defs::index
          endDirective; // index of first char behind directive, so length of
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
    void process(const char *begin, uint32 size);
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
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;
};

} // namespace glt

#endif
