#ifndef GLT_GL_PERF_COUNTER
#define GLT_GL_PERF_COUNTER

#include "glt/GLObject.hpp"
#include "data/Array.hpp"

namespace glt {

struct GLPerfCounter {
    Array<GLQueryObject> _queries;
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
