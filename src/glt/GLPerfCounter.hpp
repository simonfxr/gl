#ifndef GLT_GL_PERF_COUNTER
#define GLT_GL_PERF_COUNTER

#include "glt/GLObject.hpp"

#include "bl/vector.hpp"

namespace glt {

struct GLT_API GLPerfCounter
{
    // we use GL_TIMESTAMP
    // instead of GL_TIME_ELAPSED
    // to allow nesting

    struct Counter
    {
        GLQueryObject begin;
        GLQueryObject end;
    };

    bl::vector<Counter> _queries;
    size_t _active_query{};
    double _last_query = -1.0;
    bool _available = false;

    GLPerfCounter();
    GLPerfCounter(size_t);
    ~GLPerfCounter();

    void init(size_t);
    void begin();
    void end();
    double query();

    void clear();
};

} // namespace glt

#endif
