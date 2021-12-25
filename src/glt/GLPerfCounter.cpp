#include "glt/GLPerfCounter.hpp"

#include "glt/utils.hpp"
#include "opengl.hpp"

namespace glt {

GLPerfCounter::GLPerfCounter() = default;

GLPerfCounter::GLPerfCounter(size_t s) : GLPerfCounter()
{
    init(s);
}

GLPerfCounter::~GLPerfCounter() = default;

void
GLPerfCounter::init(size_t s)
{
    ASSERT(s > 0);
    _last_query = -1.0;
    _active_query = 0;
    _queries = std::vector<Counter>(s);
}

void
GLPerfCounter::begin()
{
    ASSERT(!_queries.empty());
    if (!_queries[_active_query].begin.valid()) {
        _queries[_active_query].begin.ensure();
        _queries[_active_query].end.ensure();
        _last_query = -1.0;
    } else {
        GLuint64 ns_t0;

        GLuint64 ns_t1;
        GL_CALL(glGetQueryObjectui64v,
                *_queries[_active_query].begin,
                GL_QUERY_RESULT,
                &ns_t0);
        GL_CALL(glGetQueryObjectui64v,
                *_queries[_active_query].end,
                GL_QUERY_RESULT,
                &ns_t1);
        _last_query = double(ns_t1 - ns_t0) * 1e-9;
    }

    GL_CALL(glQueryCounter, *_queries[_active_query].begin, GL_TIMESTAMP);
}

void
GLPerfCounter::end()
{
    GL_CALL(glQueryCounter, *_queries[_active_query].end, GL_TIMESTAMP);
    _active_query = (_active_query + 1) % _queries.size();
}

double
GLPerfCounter::query() const
{
    return _last_query;
}

void
GLPerfCounter::clear()
{
    _queries.clear();
}

} // namespace glt
