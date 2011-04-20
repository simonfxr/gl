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

    bool owning_render_target;
    RenderTarget *render_target;

    Data() :
        cameraMatrix(mat4()),
        inScene(false),
        camera_set(false),
        transformStateBOS(transform, 0, 0),
        owning_render_target(false),
        render_target(0)        
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

void RenderManager::setRenderTarget(RenderTarget& rt, bool delete_after) {

    if (&rt != self->render_target) {
        if (self->render_target != 0)
            self->render_target->deactivate();
        
        if (self->owning_render_target && self->render_target != 0) {
            delete self->render_target;
        }
    }
    
    self->owning_render_target = delete_after;
    self->render_target = &rt;

    if (self->inScene)
        self->render_target->activate();
}

RenderTarget& RenderManager::renderTarget()  {
    ASSERT_MSG(self->render_target != 0, "no RenderTarget specified");
    if (self->render_target == 0)
        return *reinterpret_cast<RenderTarget *>(0);
    else
        return *self->render_target;
}

void RenderManager::beginScene() {
    ASSERT_MSG(!self->inScene, "nested beginScene()");
    ASSERT(self->camera_set);
    ASSERT_MSG(self->render_target != 0, "no RenderTarget specified");
    self->camera_set = false;
    self->inScene = true;
    self->transformStateBOS = self->transform.save();
    self->transform.concat(self->cameraMatrix);
    self->frustum.update(self->transform.mvpMatrix());
    self->render_target->activate();
}

void RenderManager::endScene() {
    ASSERT_MSG(self->inScene, "cannot endScene() without beginScene()");
    self->inScene = false;
    self->transform.restore(self->transformStateBOS);
    self->render_target->draw();
    self->render_target->deactivate();
}

vec4_t transformModelToWorld(const RenderManager& rm, const vec4_t& modelCoord) {
    vec4_t viewCoord = transform(rm.geometryTransform().mvMatrix(), modelCoord);
    mat4_t viewWorld = inverse(rm.cameraMatrix());
    return viewWorld * viewCoord;
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
