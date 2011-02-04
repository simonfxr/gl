#ifndef GEOMETRY_TRANSFORM_HPP
#define GEOMETRY_TRANSFORM_HPP

#include "defs.h"

namespace math {

struct mat4_t;
struct mat3_t;
struct vec3_t;
struct vec4_t;

} // namespace math

namespace glt {

struct SavePoint;

struct GeometryTransform {

    GeometryTransform();
    ~GeometryTransform();

    const math::mat4_t& mvpMatrix();
    const math::mat4_t& mvMatrix();
    const math::mat3_t& normalMatrix();

    void dup();
    
    void pop();

    SavePoint save() ATTRS(ATTR_WARN_UNUSED);

    void scale(const math::vec3_t& dim);

    void translate(const math::vec3_t& origin);

    vec4_t transform(const math::vec4_t& v);

    vec3_t transformPoint(const math::vec3_t& p);

    vec3_t transformVector(const math::vec3_t& v);

private:

    GeometryTransform(const GeometryTransform& _);
    GeometryTransform& operator =(const GeometryTransform& _);
    
    struct Data;
    Data * const self;
};

struct SavePoint {
    SavePoint(GeometryTransform& _g) : g(_g) {}
private:
    GeometryTransform& g;
    ~SavePoint() { g.pop(); }
}

} // namespace glt

#endif
