#include "glt/GLPerfCounter.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"

namespace glt {

using namespace defs;

GLPerfCounter::GLPerfCounter() :
    _last_query(-1.0)
{}

GLPerfCounter::GLPerfCounter(size s) :
    _last_query(-1.0)
{
    init(s);
}

void GLPerfCounter::init(size s) {
    ASSERT(s > 0);
    _last_query = -1.0;
    _queries.unsafeResize(s);
    _active_query = 0;
}

void GLPerfCounter::begin() {
    if (!_queries[_active_query].valid()) {
        _queries[_active_query].ensure();
        _last_query = -1.0;
    } else {
        GLuint64 ns_elapsed;
        GL_CALL(glGetQueryObjectui64v, *_queries[_active_query], GL_QUERY_RESULT, &ns_elapsed);
        _last_query = double(ns_elapsed) * 1e-9;
    }

    GL_CALL(glBeginQuery, GL_TIME_ELAPSED, *_queries[_active_query]);
}

void GLPerfCounter::end() {
    GL_CALL(glEndQuery, GL_TIME_ELAPSED);
    _active_query = (_active_query + 1) % UNSIZE(_queries.size());
}

double GLPerfCounter::query() {
    return _last_query;
}

} // namespace glt
