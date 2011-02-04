#include "glt/GeometryTransform.hpp"

#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "math/vec4.hpp"

using namespace math

namespace glt {

struct GeometryTransform::Data {

    mat4_t mvpMatrix;
    mat4_t mvMatrix;
    mat3_t normalMatrix;

    bool dirty;

    GeometryTransform() :
        mvpMatrix(mat4()),
        mvMatrix(mat4()),
        normalMatrix(mat3()),
        dirty(false)
    {}
};

GeometryTransform::GeometryTransform() :
    self(new Data);
{}

GeometryTransform::~GeometryTransform() {
    delete self;
} // namespace glt
