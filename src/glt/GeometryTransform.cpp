#include "glt/GeometryTransform.hpp"
#include "err/err.hpp"
#include "glt/Transformations.hpp"

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace glt {

using namespace math;

static const uint16 FLAG_MV = 1;
static const uint16 FLAG_MVP = 2;
static const uint16 FLAG_VP = 4;
static const uint16 FLAG_NORMAL = 8;
static const uint16 FLAG_INVPROJ = 16;
static const uint16 FLAG_ALL = 0x1F;

struct GeometryTransform::Data
{

    aligned_mat4_t viewMatrix;
    aligned_mat4_t projectionMatrix;

    aligned_mat4_t mvMatrix{};
    aligned_mat4_t mvpMatrix{};
    aligned_mat4_t vpMatrix{};
    aligned_mat3_t normalMatrix{};

    uint16 depth{ 0 };
    uint16 dirty_flags;

    aligned_mat4_t inverseProjectionMatrix;

    aligned_mat4_t modelMatrices[GEOMETRY_TRANSFORM_MAX_DEPTH]{};
    uint64 mods[GEOMETRY_TRANSFORM_MAX_DEPTH]{};

    Data()
      : viewMatrix(mat4())
      , projectionMatrix(mat4())
      , dirty_flags(FLAG_ALL)
      , inverseProjectionMatrix(mat4())
    {
        modelMatrices[0] = mat4();
        mods[0] = 0;
    }

    bool flag(uint16 flg)
    {
        if (unlikely((dirty_flags & flg) != 0)) {
            dirty_flags = uint16(dirty_flags & ~flg);
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

GeometryTransform::GeometryTransform() : self(new Data) {}

GeometryTransform::~GeometryTransform()
{
    delete self;
}

const aligned_mat4_t &
GeometryTransform::modelMatrix() const
{
    return self->modelMatrices[self->depth];
}

const aligned_mat4_t &
GeometryTransform::viewMatrix() const
{
    return self->viewMatrix;
}

const aligned_mat4_t &
GeometryTransform::projectionMatrix() const
{
    return self->projectionMatrix;
}

const aligned_mat4_t &
GeometryTransform::mvpMatrix() const
{
    if (self->flag(FLAG_MVP))
        self->mvpMatrix = vpMatrix() * modelMatrix();
    return self->mvpMatrix;
}

const aligned_mat4_t &
GeometryTransform::mvMatrix() const
{
    if (self->flag(FLAG_MV))
        self->mvMatrix = viewMatrix() * modelMatrix();
    return self->mvMatrix;
}

const aligned_mat4_t &
GeometryTransform::vpMatrix() const
{
    if (self->flag(FLAG_VP))
        self->vpMatrix = projectionMatrix() * viewMatrix();
    return self->vpMatrix;
}

const aligned_mat3_t &
GeometryTransform::normalMatrix() const
{
    if (self->flag(FLAG_NORMAL)) {
        // transpose of inverse modelView
        self->normalMatrix = transpose(inverse(mat3(mvMatrix())));
    }
    return self->normalMatrix;
}

const aligned_mat4_t &
GeometryTransform::inverseProjectionMatrix() const
{
    if (self->flag(FLAG_INVPROJ))
        self->inverseProjectionMatrix = inverse(self->projectionMatrix);
    return self->inverseProjectionMatrix;
}

void
GeometryTransform::loadModelMatrix(const mat4_t &m)
{
    self->modelMatrices[self->depth] = m;
    self->modelUpdated();
}

void
GeometryTransform::loadViewMatrix(const mat4_t &m)
{
    self->dirty_flags = FLAG_MVP | FLAG_VP | FLAG_MV | FLAG_NORMAL;
    self->viewMatrix = m;
    ++self->mods[self->depth];
}

void
GeometryTransform::loadProjectionMatrix(const mat4_t &m)
{
    self->dirty_flags = FLAG_MVP | FLAG_VP | FLAG_INVPROJ;
    self->projectionMatrix = m;
    ++self->mods[self->depth];
}

void
GeometryTransform::dup()
{
    if (unlikely(self->depth >= GEOMETRY_TRANSFORM_MAX_DEPTH - 1))
        FATAL_ERR("GeometryTransform: stack overflow");
    self->modelMatrices[self->depth + 1] = self->modelMatrices[self->depth];
    self->mods[self->depth + 1] = self->mods[self->depth];
    ++self->depth;
}

void
GeometryTransform::pop()
{
    if (unlikely(self->depth == 0))
        FATAL_ERR("GeometryTransform: stack underflow");
    --self->depth;
    if (self->mods[self->depth] != self->mods[self->depth + 1])
        self->dirty_flags |= FLAG_MV | FLAG_MVP | FLAG_NORMAL;
}

SavePointArgs
GeometryTransform::save()
{
    uint64 cookie = self->mods[self->depth];
    uint16 depth = self->depth;
    dup();
    return SavePointArgs(*this, cookie, depth);
}

void
GeometryTransform::restore(const SavePointArgs &sp)
{
    if (unlikely(self->mods[sp.depth] != sp.cookie || self->depth < sp.depth))
        FATAL_ERR(
          "cannot restore GeometryTransform: stack modified below safepoint");
    self->depth = sp.depth;
    if (self->mods[sp.depth] != self->mods[self->depth])
        self->dirty_flags |= FLAG_MV | FLAG_MVP | FLAG_NORMAL;
}

void
GeometryTransform::scale(const vec3_t &dim)
{
    self->modelMatrices[self->depth][0] *= dim[0];
    self->modelMatrices[self->depth][1] *= dim[1];
    self->modelMatrices[self->depth][2] *= dim[2];
    self->modelUpdated();
}

void
GeometryTransform::translate(const vec3_t &origin)
{
    self->modelMatrices[self->depth][3] += vec4(transformPoint(origin), 0.f);
    self->modelUpdated();
}

void
GeometryTransform::rotate(real phi, const direction3_t &axis)
{
    concat(rotationMatrix(phi, axis));
}

void
GeometryTransform::concat(const mat3_t &m)
{
    concat(mat4(m));
}

void
GeometryTransform::concat(const mat4_t &m)
{
    self->modelMatrices[self->depth] *= m;
    self->modelUpdated();
}

vec4_t
GeometryTransform::transform(const vec4_t &v) const
{
    return self->modelMatrices[self->depth] * v;
}

vec3_t
GeometryTransform::transformPoint(const vec3_t &p) const
{
    return vec3(transform(vec4(p, 1.f)));
}

vec3_t
GeometryTransform::transformVector(const vec3_t &v) const
{
    return vec3(transform(vec4(v, 0.f)));
}

size
GeometryTransform::depth() const
{
    return self->depth + 1;
}

} // namespace glt
