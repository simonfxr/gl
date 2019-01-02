#ifndef GLT_GL_PERF_COUNTER
#define GLT_GL_PERF_COUNTER

#include "data/Array.hpp"
#include "glt/GLObject.hpp"

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

        Counter() : begin(), end() {}

    private:
        Counter(const Counter &) = delete;
        Counter &operator=(const Counter &) = delete;
    };

    size_t _nqueries;
    Counter *_queries;
    size_t _active_query;
    double _last_query;

    GLPerfCounter();
    GLPerfCounter(size_t);
    ~GLPerfCounter();

    void init(size_t);
    void begin();
    void end();
    double query();

    void shutdown();
};

} // namespace glt

#endif
