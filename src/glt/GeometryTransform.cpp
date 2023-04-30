#include "glt/GeometryTransform.hpp"

#include "err/err.hpp"
#include "glt/Transformations.hpp"
#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include <bl/array.hpp>

namespace glt {

using namespace math;

inline constexpr uint16_t FLAG_MV = 1;
inline constexpr uint16_t FLAG_MVP = 2;
inline constexpr uint16_t FLAG_VP = 4;
inline constexpr uint16_t FLAG_NORMAL = 8;
inline constexpr uint16_t FLAG_INVPROJ = 16;
inline constexpr uint16_t FLAG_ALL = 0x1F;

struct GeometryTransform::Impl : GeometryTransform
{
    mat4_t viewMatrix;
    mat4_t projectionMatrix;

    mutable mat4_t mvMatrix{};
    mutable mat4_t mvpMatrix{};
    mutable mat4_t vpMatrix{};
    mutable mat3_t normalMatrix{};

    uint16_t depth{};
    mutable uint16_t dirty_flags;

    mutable mat4_t inverseProjectionMatrix;

    bl::array<mat4_t, GEOMETRY_TRANSFORM_MAX_DEPTH> modelMatrices{};
    bl::array<uint64_t, GEOMETRY_TRANSFORM_MAX_DEPTH> mods{};

    Impl()
      : viewMatrix(mat4())
      , projectionMatrix(mat4())
      , dirty_flags(FLAG_ALL)
      , inverseProjectionMatrix(mat4())
    {
        modelMatrices[0] = mat4();
        mods[0] = 0;
    }

    bool flag(uint16_t flg) const
    {
        if (unlikely((dirty_flags & flg) != 0)) {
            dirty_flags = static_cast<uint16_t>(dirty_flags & ~flg);
            return true;
        }
        return false;
    }

    void modelUpdated()
    {
        dirty_flags = FLAG_MV | FLAG_MVP | FLAG_NORMAL;
        ++mods[depth];
    }
};

BL_DEFINE_RUNTIME_SIZED(GeometryTransform, GeometryTransform::Impl);

GeometryTransform &
GeometryTransform::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

const mat4_t &
GeometryTransform::modelMatrix() const
{
    return m().modelMatrices[m().depth];
}

const mat4_t &
GeometryTransform::viewMatrix() const
{
    return m().viewMatrix;
}

const mat4_t &
GeometryTransform::projectionMatrix() const
{
    return m().projectionMatrix;
}

const mat4_t &
GeometryTransform::mvpMatrix() const
{
    if (m().flag(FLAG_MVP))
        m().mvpMatrix = vpMatrix() * modelMatrix();
    return m().mvpMatrix;
}

const mat4_t &
GeometryTransform::mvMatrix() const
{
    if (m().flag(FLAG_MV))
        m().mvMatrix = viewMatrix() * modelMatrix();
    return m().mvMatrix;
}

const mat4_t &
GeometryTransform::vpMatrix() const
{
    if (m().flag(FLAG_VP))
        m().vpMatrix = projectionMatrix() * viewMatrix();
    return m().vpMatrix;
}

const mat3_t &
GeometryTransform::normalMatrix() const
{
    if (m().flag(FLAG_NORMAL)) {
        // transpose of inverse modelView
        m().normalMatrix = transpose(inverse(mat3(mvMatrix())));
    }
    return m().normalMatrix;
}

const mat4_t &
GeometryTransform::inverseProjectionMatrix() const
{
    if (m().flag(FLAG_INVPROJ))
        m().inverseProjectionMatrix = inverse(m().projectionMatrix);
    return m().inverseProjectionMatrix;
}

void
GeometryTransform::loadModelMatrix(const mat4_t &M)
{
    m().modelMatrices[m().depth] = M;
    m().modelUpdated();
}

void
GeometryTransform::loadViewMatrix(const mat4_t &M)
{
    m().dirty_flags = FLAG_MVP | FLAG_VP | FLAG_MV | FLAG_NORMAL;
    m().viewMatrix = M;
    ++m().mods[m().depth];
}

void
GeometryTransform::loadProjectionMatrix(const mat4_t &M)
{
    m().dirty_flags = FLAG_MVP | FLAG_VP | FLAG_INVPROJ;
    m().projectionMatrix = M;
    ++m().mods[m().depth];
}

void
GeometryTransform::dup()
{
    if (unlikely(m().depth >= GEOMETRY_TRANSFORM_MAX_DEPTH - 1))
        FATAL_ERR("GeometryTransform: stack overflow");
    m().modelMatrices[m().depth + 1] = m().modelMatrices[m().depth];
    m().mods[m().depth + 1] = m().mods[m().depth];
    ++m().depth;
}

void
GeometryTransform::pop()
{
    if (unlikely(m().depth == 0))
        FATAL_ERR("GeometryTransform: stack underflow");
    --m().depth;
    if (m().mods[m().depth] != m().mods[m().depth + 1])
        m().dirty_flags |= FLAG_MV | FLAG_MVP | FLAG_NORMAL;
}

SavePoint
GeometryTransform::save()
{
    auto cookie = m().mods[m().depth];
    auto depth = m().depth;
    dup();
    return { *this, cookie, depth };
}

void
GeometryTransform::restore(SavePoint &sp)
{
    auto gt = bl::exchange(sp.gt, nullptr);
    auto depth = bl::exchange(sp.depth, 0);
    auto cookie = bl::exchange(sp.cookie, 0);
    if (unlikely(this != gt || m().mods[depth] != cookie || m().depth < depth))
        FATAL_ERR(
          "cannot restore GeometryTransform: stack modified below safepoint");

    m().depth = depth;
    if (m().mods[depth] != m().mods[m().depth])
        m().dirty_flags |= FLAG_MV | FLAG_MVP | FLAG_NORMAL;
}

void
GeometryTransform::scale(const vec3 &dim)
{
    m().modelMatrices[m().depth][0] *= dim[0];
    m().modelMatrices[m().depth][1] *= dim[1];
    m().modelMatrices[m().depth][2] *= dim[2];
    m().modelUpdated();
}

void
GeometryTransform::translate(const vec3 &origin)
{
    m().modelMatrices[m().depth][3] += vec4(transformPoint(origin), 0.f);
    m().modelUpdated();
}

void
GeometryTransform::rotate(real phi, const direction3 &axis)
{
    concat(rotationMatrix(phi, axis));
}

void
GeometryTransform::concat(const mat3_t &M)
{
    concat(mat4(M));
}

void
GeometryTransform::concat(const mat4_t &M)
{
    m().modelMatrices[m().depth] *= M;
    m().modelUpdated();
}

vec4
GeometryTransform::transform(const vec4 &v) const
{
    return m().modelMatrices[m().depth](v);
}

vec3
GeometryTransform::transformPoint(const vec3 &p) const
{
    return vec3(transform(vec4(p, 1.f)));
}

vec3
GeometryTransform::transformVector(const vec3 &v) const
{
    return vec3(transform(vec4(v, 0.f)));
}

size_t
GeometryTransform::depth() const
{
    return m().depth + 1;
}

} // namespace glt
