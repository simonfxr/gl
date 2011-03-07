#include "glt/RenderManager.hpp"

#include "glt/Transform.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4.hpp"

namespace glt {

using namespace math;

struct GeometryTransform::Data {
    aligned_mat4_t cameraMatrix;
    ViewFrustum frustum;
    GeometryTransform transform;
    bool inScene;
    SavePointArgs transformStateBOS; // transform state in begin of scene

    Data() :
        cameraMatrix(mat4()),
        inScene(false)
        {}
};

RenderManager::RenderManager() :
    self(new Data)
{}

RenderManager::~RenderManager() {
    delete self;
}

void RenderManager::setPerspectiveProjection(float theta, float aspectRatio, float z_near, float z_far) {
    aligned_mat4_t m = perspectiveProjection(theta, aspectRatio, z_near, z_far);
    self->transform.loadProjectionMatrix(m);
    self->frustum.updateView(self->cameraMatrix, m);
}

void RenderManager::setCameraMatrix(const mat4_t& m) {
    self->cameraMatrix = m;
}

void RenderManager::beginScene() {
    ASSERT(!self->inScene, "nested beginScene()!");
    self->inScene = true;
    self->transformStateBOS = self->transform.save();
    self->transform.concat(self->cameraMatrix());
}

void RenderManager::endScene() {
    ASSERT(self->inScene, "cannot endScene() without beginScene()!");
    self->inScene = false;
    self->transform.restore(self->transformStateBOS);
}

vec4_t project(const RenderManager& rm, const point3_t& localCoord) {
    return transform(rm.geomtryTransform().mvpMatrix(), localCoord);
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
