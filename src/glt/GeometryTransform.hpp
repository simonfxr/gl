#ifndef GEOMETRY_TRANSFORM_HPP
#define GEOMETRY_TRANSFORM_HPP

#include "defs.hpp"

#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"
#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

#include "err/err.hpp"

#include <iostream>

namespace glt {

using namespace defs;

static const uint32 GEOMETRY_TRANSFORM_MAX_DEPTH = 16;

struct SavePoint;
struct SavePointArgs;

struct GeometryTransform {

    GeometryTransform();
    ~GeometryTransform();

    const math::aligned_mat4_t& modelMatrix() const;
    const math::aligned_mat4_t& viewMatrix() const;
    const math::aligned_mat4_t& projectionMatrix() const;
    const math::aligned_mat4_t& inverseProjectionMatrix() const;
    
    const math::aligned_mat4_t& mvpMatrix() const;
    const math::aligned_mat4_t& mvMatrix() const;
    const math::aligned_mat4_t& vpMatrix() const;
    const math::aligned_mat3_t& normalMatrix() const;

    void loadModelMatrix(const math::mat4_t& m);

    void loadViewMatrix(const math::mat4_t& m);

    void loadProjectionMatrix(const math::mat4_t& m);

    void dup();
    
    void pop();

    SavePointArgs save() ATTRS(ATTR_WARN_UNUSED);

    void restore(const SavePointArgs& sp);

    void scale(const math::vec3_t& dim);

    void translate(const math::vec3_t& origin);

    void rotate(float phi, const math::direction3_t& axis);

    void concat(const math::mat3_t& m);

    void concat(const math::mat4_t& m);

    math::vec4_t transform(const math::vec4_t& v) const;

    math::vec3_t transformPoint(const math::vec3_t& p) const;

    math::vec3_t transformVector(const math::vec3_t& v) const;

    size depth() const;

private:

    GeometryTransform(const GeometryTransform& _);
    GeometryTransform& operator =(const GeometryTransform& _);
    
    struct Data;
    Data * const self;
};

struct SavePointArgs {    
protected:
    GeometryTransform* g;
    uint64 cookie;
    uint16 depth;

    friend struct GeometryTransform;
    friend struct SavePoint;

public:
    SavePointArgs(GeometryTransform& _g, uint64 _cookie, uint16 _depth) :
        g(&_g), cookie(_cookie), depth(_depth) {}
};

struct SavePoint {
    const SavePointArgs args;
    SavePoint(const SavePointArgs& _args) : args(_args)  {}
    
    ~SavePoint() { args.g->restore(args); }

private:
    SavePoint(const SavePoint& _);
    SavePoint& operator =(const SavePoint& _);
};

} // namespace glt

#endif
