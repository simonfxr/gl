#ifndef GLT_GL_PERF_COUNTER
#define GLT_GL_PERF_COUNTER

#include "glt/GLObject.hpp"
#include "data/Array.hpp"

namespace glt {

struct GLT_API GLPerfCounter {
    
    // we use GL_TIMESTAMP
    // instead of GL_TIME_ELAPSED
    // to allow nesting

    struct Counter {
        GLQueryObject begin;
        GLQueryObject end;
    };
    
    OwnedArray<Counter> _queries;
    defs::index _active_query;
    double _last_query;

    GLPerfCounter();
    GLPerfCounter(defs::size);

    void init(defs::size);
    void begin();
    void end();
    double query();
};

} // namespace glt

#endif
