#ifndef GEOMETRY_TRANSFORM_HPP
#define GEOMETRY_TRANSFORM_HPP

#include "err/err.hpp"
#include "glt/conf.hpp"
#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include <bl/box.hpp>
#include <bl/noncopymove.hpp>

namespace glt {

inline constexpr uint32_t GEOMETRY_TRANSFORM_MAX_DEPTH = 16;

struct SavePoint;

struct GLT_API GeometryTransform
  : private bl::runtime_sized_base<GeometryTransform>
{
    static GeometryTransform &construct_at(void *) noexcept;

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

    HU_NODISCARD SavePoint save();

    void restore(SavePoint &sp);

    void scale(const math::vec3 &dim);
    void scale(math::real s) { scale(math::vec3(s)); }

    void translate(const math::vec3 &origin);

    void rotate(math::real phi, const math::direction3 &axis);

    void concat(const math::mat3_t &m);

    void concat(const math::mat4_t &m);

    math::vec4 transform(const math::vec4 &v) const;

    math::vec3 transformPoint(const math::vec3 &p) const;

    math::vec3 transformVector(const math::vec3 &v) const;

    size_t depth() const;

    BL_DECLARE_RUNTIME_SIZED(GeometryTransform, Impl);
};

struct GLT_API SavePoint : private bl::non_copyable
{
    friend struct GeometryTransform;

private:
    GeometryTransform *gt;
    uint64_t cookie;
    uint16_t depth;

    SavePoint(GeometryTransform &gt, uint64_t cookie, uint16_t depth)
      : gt{ &gt }, cookie{ cookie }, depth{ depth }
    {}

public:
    ~SavePoint()
    {
        if (gt)
            gt->restore(*this);
    }
};

} // namespace glt

#endif
