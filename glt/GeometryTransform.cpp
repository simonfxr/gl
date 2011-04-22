#include "glt/GeometryTransform.hpp"
#include "glt/utils.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

using namespace math;

struct GeometryTransform::Data {

    aligned_mat4_t projectionMatrix;

    aligned_mat4_t mvpMatrix;
    aligned_mat3_t normalMatrix;

    uint64 depth;

    bool dirty;

    aligned_mat4_t mvMats[GEOMETRY_TRANSFORM_MAX_DEPTH];
    uint64 mods[GEOMETRY_TRANSFORM_MAX_DEPTH];

    Data() :
        projectionMatrix(mat4()),
        mvpMatrix(mat4()),
        normalMatrix(mat3()),
        depth(0),
        dirty(false)
    {
        mvMats[0] = mat4();
        mods[0] = 0;
    }

    void update() {
        if (unlikely(dirty)) {
            dirty = false;
            mvpMatrix = projectionMatrix * mvMats[depth];
            normalMatrix = mat3(normalize(vec3(mvMats[depth][0])),
                                normalize(vec3(mvMats[depth][1])),
                                normalize(vec3(mvMats[depth][2])));
        }
    }
};

GeometryTransform::GeometryTransform() :
    self(new Data)
{}

GeometryTransform::~GeometryTransform() {
    delete self;
}

const aligned_mat4_t& GeometryTransform::mvpMatrix() const {
    self->update();
    return self->mvpMatrix;
}

const aligned_mat4_t& GeometryTransform::mvMatrix() const {
    return self->mvMats[self->depth];
}

const aligned_mat3_t& GeometryTransform::normalMatrix() const {
    self->update();
    return self->normalMatrix;
}

const aligned_mat4_t& GeometryTransform::projectionMatrix() const {
    return self->projectionMatrix;
}

void GeometryTransform::loadMVMatrix(const mat4_t& m) {
    ++self->mods[self->depth];
    self->dirty = true;
    self->mvMats[self->depth] = m;
}

void GeometryTransform::loadProjectionMatrix(const mat4_t& m) {
    ++self->mods[self->depth];
    self->dirty = true;
    self->projectionMatrix = m;
}

void GeometryTransform::dup() {
    if (unlikely(self->depth + 1 >= GEOMETRY_TRANSFORM_MAX_DEPTH))
        FATAL_ERR("GeometryTransform: stack overflow");
    self->mvMats[self->depth + 1] = self->mvMats[self->depth];
    self->mods[self->depth + 1] = self->mods[self->depth];
    ++self->depth;
}
    
void GeometryTransform::pop() {
    if (unlikely(self->depth == 0))
        FATAL_ERR("GeometryTransform: stack underflow");
    --self->depth;
    self->dirty = self->mods[self->depth] != self->mods[self->depth + 1];
}

SavePointArgs GeometryTransform::save() {
    uint64 cookie = self->mods[self->depth];
    uint32 depth  = self->depth;
    dup();
    return SavePointArgs(*this, cookie, depth);
}

void GeometryTransform::restore(const SavePointArgs& sp) {
    if (unlikely(self->mods[sp.depth] != sp.cookie || self->depth < sp.depth))
        FATAL_ERR("cannot restore GeometryTransform: stack modified below safepoint");
    self->dirty = self->mods[sp.depth] != self->mods[self->depth];
    self->depth = sp.depth;
}

void GeometryTransform::scale(const vec3_t& dim) {
    concat(mat3(vec3(dim.x, 0.f, 0.f),
                vec3(0.f, dim.y, 0.f),
                vec3(0.f, 0.f, dim.z)));
}

void GeometryTransform::translate(const vec3_t& origin) {
    mat4_t trans = mat4();
    trans[3] = vec4(origin, 1.f);
    concat(trans);
}

void GeometryTransform::concat(const mat3_t& m) {
    concat(mat4(m));
}

void GeometryTransform::concat(const mat4_t& m) {
    self->dirty = true;
    ++self->mods[self->depth];
    self->mvMats[self->depth] *= m;
    self->update();
}

vec4_t GeometryTransform::transform(const vec4_t& v) const {
    return self->mvMats[self->depth] * v;
}

vec3_t GeometryTransform::transformPoint(const vec3_t& p) const {
    return vec3(transform(vec4(p, 1.f)));
}

vec3_t GeometryTransform::transformVector(const vec3_t& v) const {
    return vec3(transform(vec4(v, 0.f)));
}

uint32 GeometryTransform::depth() const {
    return self->depth + 1;
}
   
} // namespace glt
