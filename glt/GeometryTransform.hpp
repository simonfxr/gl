#ifndef GEOMETRY_TRANSFORM_HPP
#define GEOMETRY_TRANSFORM_HPP

#include "defs.h"

#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"
#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

#include "glt/utils.hpp"

#include <iostream>

namespace glt {

static const uint32 GEOMETRY_TRANSFORM_MAX_DEPTH = 32;

struct SavePoint;
struct SavePointArgs;

struct GeometryTransform {

    GeometryTransform();
    ~GeometryTransform();

    const math::aligned_mat4_t& mvpMatrix() const;
    const math::aligned_mat4_t& mvMatrix() const;
    const math::aligned_mat3_t& normalMatrix() const;
    const math::aligned_mat4_t& projectionMatrix() const;

    void loadMVMatrix(const math::mat4_t& m);

    void loadProjectionMatrix(const math::mat4_t& m);

    void dup();
    
    void pop();

    SavePointArgs save() ATTRS(ATTR_WARN_UNUSED);

    void restore(SavePoint& sp);

    void scale(const math::vec3_t& dim);

    void translate(const math::vec3_t& origin);

    void concat(const math::mat3_t& m);

    void concat(const math::mat4_t& m);

    math::vec4_t transform(const math::vec4_t& v) const;

    math::vec3_t transformPoint(const math::vec3_t& p) const;

    math::vec3_t transformVector(const math::vec3_t& v) const;

    uint32 depth() const;

private:

    GeometryTransform(const GeometryTransform& _);
    GeometryTransform& operator =(const GeometryTransform& _);
    
    struct Data;
    Data * const self;
};

struct SavePointArgs {    
protected:

    GeometryTransform& g;
    const uint64 cookie;
    const uint32 depth;

public:
    SavePointArgs(GeometryTransform& _g, uint64 _cookie, uint32 _depth) :
        g(_g), cookie(_cookie), depth(_depth) {}
};

struct SavePoint : public SavePointArgs {

    friend struct GeometryTransform;

    SavePoint(const SavePointArgs& args) : SavePointArgs(args) {}
    
    ~SavePoint() { g.restore(*this); }

private:
    SavePoint(const SavePoint& _);
    SavePoint& operator =(const SavePoint& _);
};

} // namespace glt

#endif
