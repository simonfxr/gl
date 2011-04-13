#include "glt/RenderManager.hpp"

#include "glt/Transformations.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

using namespace math;

struct RenderManager::Data {
    aligned_mat4_t cameraMatrix;
    ViewFrustum frustum;
    GeometryTransform transform;
    bool inScene;
    bool camera_set;
    SavePointArgs transformStateBOS; // transform state in begin of scene

    Data() :
        cameraMatrix(mat4()),
        inScene(false),
        camera_set(false),
        transformStateBOS(transform, 0, 0)
        {}
};

RenderManager::RenderManager() :
    self(new Data)
{}

RenderManager::~RenderManager() {
    delete self;
}

const ViewFrustum& RenderManager::viewFrustum() const {
    return self->frustum;
}

const GeometryTransform& RenderManager::geometryTransform() const {
    return self->transform;
}

GeometryTransform& RenderManager::geometryTransform() {
    return self->transform;
}

const aligned_mat4_t& RenderManager::cameraMatrix() const {
    return self->cameraMatrix;
}

void RenderManager::setPerspectiveProjection(float theta, float aspectRatio, float z_near, float z_far) {
    aligned_mat4_t m = perspectiveProjection(theta, aspectRatio, z_near, z_far);
    self->transform.loadProjectionMatrix(m);
}

void RenderManager::setCameraMatrix(const mat4_t& m) {
    self->cameraMatrix = m;
    self->camera_set = true;
}

void RenderManager::beginScene() {
    ASSERT_MSG(!self->inScene, "nested beginScene()!");
    ASSERT(self->camera_set);
    self->camera_set = false;
    self->inScene = true;
    self->transformStateBOS = self->transform.save();
    self->transform.concat(self->cameraMatrix);
    self->frustum.update(self->transform.mvpMatrix());
}

void RenderManager::endScene() {
    ASSERT_MSG(self->inScene, "cannot endScene() without beginScene()!");
    self->inScene = false;
    self->transform.restore(self->transformStateBOS);
}

vec4_t transformModelToWorld(const RenderManager& rm, const vec4_t& modelCoord) {
    vec4_t view = transform(rm.geometryTransform().mvMatrix(), modelCoord);
    mat4_t m = rm.cameraMatrix();
    vec3_t trans = vec3(m[3]);
    mat3_t irot = transpose(mat3(vec3(m[0]),
                                 vec3(m[1]),
                                 vec3(m[2])));
    m = mat4(irot);
    m[3] = m * vec4(-trans, 1.f);
    return m * view;
}

vec4_t project(const RenderManager& rm, const point3_t& localCoord) {
    return transform(rm.geometryTransform().mvpMatrix(), vec4(localCoord, 1.f));
}

vec4_t projectView(const RenderManager& rm, const point3_t& eyeCoord) {
    return transform(rm.geometryTransform().projectionMatrix(), vec4(eyeCoord, 1.f));
}

Outcode testSphere(const RenderManager& rm, const point3_t& center, float rad) {
    return testSphere(rm.viewFrustum(), center, rad);
}

Outcode testPoint(const RenderManager& rm, const point3_t& p) {
    return testPoint(rm.viewFrustum(), p);
}

} // namespace glt
