#ifndef GEOMETRY_TRANSFORM_HPP
#define GEOMETRY_TRANSFORM_HPP

#include "err/err.hpp"
#include "glt/conf.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "pp/pimpl.hpp"

namespace glt {

inline constexpr uint32_t GEOMETRY_TRANSFORM_MAX_DEPTH = 16;

struct SavePoint;
struct SavePointArgs;

struct GLT_API GeometryTransform
{
    GeometryTransform();

    const math::mat4_t &modelMatrix() const;
    const math::mat4_t &viewMatrix() const;
    const math::mat4_t &projectionMatrix() const;
    const math::mat4_t &inverseProjectionMatrix() const;

    const math::mat4_t &mvpMatrix() const;
    const math::mat4_t &mvMatrix() const;
    const math::mat4_t &vpMatrix() const;
    const math::mat3_t &normalMatrix() const;

    void loadModelMatrix(const math::mat4_t &m);

    void loadViewMatrix(const math::mat4_t &m);

    void loadProjectionMatrix(const math::mat4_t &m);

    void dup();

    void pop();

    HU_NODISCARD SavePointArgs save();

    void restore(const SavePointArgs &sp);

    void scale(const math::vec3_t &dim);

    void translate(const math::vec3_t &origin);

    void rotate(math::real phi, const math::direction3_t &axis);

    void concat(const math::mat3_t &m);

    void concat(const math::mat4_t &m);

    math::vec4_t transform(const math::vec4_t &v) const;

    math::vec3_t transformPoint(const math::vec3_t &p) const;

    math::vec3_t transformVector(const math::vec3_t &v) const;

    size_t depth() const;

private:
    DECLARE_PIMPL(GLT_API, self);
};

struct GLT_API SavePointArgs
{
protected:
    GeometryTransform *g;
    uint64_t cookie;
    uint16_t depth;

    friend struct GeometryTransform;
    friend struct SavePoint;

public:
    SavePointArgs(GeometryTransform &_g, uint64_t _cookie, uint16_t _depth)
      : g(&_g), cookie(_cookie), depth(_depth)
    {}

    SavePointArgs(const SavePoint &) = delete;
    SavePointArgs &operator=(const SavePoint &) = delete;
};

struct GLT_API SavePoint
{
    const SavePointArgs args;
    SavePoint(const SavePointArgs &_args) : args(_args) {}

    ~SavePoint() { args.g->restore(args); }

    SavePoint(const SavePoint &) = delete;
    SavePoint(SavePoint &&) = delete;
    SavePoint &operator=(const SavePoint &) = delete;
    SavePoint &operator=(SavePoint &&) = delete;
};

} // namespace glt

#endif
