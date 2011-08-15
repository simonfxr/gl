#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <ostream>
#include <map>

#include "defs.hpp"

namespace glt {

using namespace defs;

struct Preprocessor {

    struct ContentContext {
        Preprocessor& processor;
        const std::string name;
        const char *data;
        size_t size;

        ContentContext(Preprocessor& proc, const std::string& nam) :
            processor(proc), name(nam) {}
    };

    struct DirectiveContext {
        ContentContext content;
        
        defs::size lineLength;
        defs::index lineOffset;
        defs::index beginDirective; // index of first char in directive
        defs::index endDirective;   // index of first char behind directive, so length of directive = endDirective - beginDirective

        DirectiveContext(Preprocessor &proc, const std::string& name) :
            content(proc, name) {}
    };

    struct DirectiveHandler {
        virtual ~DirectiveHandler();
        virtual void beginProcessing(const ContentContext& ctx);
        virtual void endProcessing(const ContentContext& ctx);
        virtual void directiveEncountered(const DirectiveContext& context);
    };

    typedef std::map<std::string, DirectiveHandler*> Handlers;
    typedef std::pair<std::string, DirectiveHandler*> HandlerEntry;

    Preprocessor();
    ~Preprocessor();

    const std::string& name() const;
    void name(const std::string& name);

    void process(const std::string&);
    void process(const char *contents, uint32 size);
    void process(const char *contents);

    DirectiveHandler &defaultHandler(DirectiveHandler& handler);
    DirectiveHandler &defaultHandler() const;

    Handlers &handlers() const;
    
    DirectiveHandler *installHandler(const std::string& directive, DirectiveHandler& handler);

    std::ostream& err() const;
    void err(std::ostream& err);
    
    bool setError();
    bool wasError() const;
    bool clearError();

    static DirectiveHandler& nullHandler();

private:

    struct Data;
    Data * const self;

    Preprocessor(const Preprocessor& _);
    Preprocessor& operator =(const Preprocessor& _);
};

} // namespace glt

#endif
