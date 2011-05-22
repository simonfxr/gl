#include "glt/RenderManager.hpp"

#include "glt/Transformations.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include <algorithm>
#include <SFML/System/Clock.hpp>

namespace glt {

using namespace math;

static const float FS_UPDATE_INTERVAL = 1.f;
static const float FS_UPDATE_INTERVAL_FAST = 0.1f;

struct RenderManager::Data {
    aligned_mat4_t cameraMatrix;
    ViewFrustum frustum;
    GeometryTransform transform;
    bool inScene;
    bool camera_set;
    SavePointArgs transformStateBOS; // transform state in begin of scene

    bool owning_def_rt;
    RenderTarget *def_rt;

    // always active
    RenderTarget *current_rt;

    sf::Clock clk;
    float stat_snapshot;
    float stat_fast_snapshot;
    float framet0;
    float renderAcc;
    float renderAccFast;
    uint32 num_frames_fast;
    uint32 num_frames;
    
    FrameStatistics stats;

    Data() :
        cameraMatrix(mat4()),
        inScene(false),
        camera_set(false),
        transformStateBOS(transform, 0, 0),
        owning_def_rt(false),
        def_rt(0),
        current_rt(0),
        stat_snapshot(NEG_INF),
        stat_fast_snapshot(NEG_INF),
        renderAcc(1.f),
        renderAccFast(1.f)
        {}

    void statBegin() {
        framet0 = clk.GetElapsedTime();
        float diff = framet0 - stat_snapshot;
        if (diff >= FS_UPDATE_INTERVAL) {
            stats.avg_fps = uint32(num_frames / diff);
            stats.rt_avg = uint32(num_frames / renderAcc);
            renderAcc = 0.f;
            num_frames = 0;
            stat_snapshot = framet0;
            stats.rt_min = 0xFFFFFFFF;
            stats.rt_max = 0;
        }

        float diff2 = framet0 - stat_fast_snapshot;
        if (diff2 >= FS_UPDATE_INTERVAL_FAST) {
            stats.rt_current = uint32(num_frames_fast / renderAccFast);
            renderAccFast = 0.f;
            num_frames_fast = 0;
            stat_fast_snapshot = framet0;
        }
    }

    void statEnd() {
        float frameDur = clk.GetElapsedTime() - framet0;
        renderAcc += frameDur;
        renderAccFast += frameDur;
        uint32 fps = uint32(math::recip(frameDur));
        stats.rt_max = std::max(stats.rt_max, fps);
        stats.rt_min = std::min(stats.rt_min, fps);
        ++num_frames;
        ++num_frames_fast;
    }
};

RenderManager::RenderManager() :
    self(new Data)
{}

RenderManager::~RenderManager() {
    if (self->current_rt != 0)
        self->current_rt->deactivate();

    if (self->owning_def_rt)
        delete self->def_rt;
    
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

void RenderManager::setDefaultRenderTarget(RenderTarget *rt, bool delete_after) {

    if (rt != self->def_rt) {

        if (self->owning_def_rt && self->def_rt != 0) {

            if (self->current_rt == self->def_rt) {
                WARN("destroying active render target");
                self->current_rt->deactivate();
                self->current_rt = 0;
            }
            
            delete self->def_rt;
        }

        self->def_rt = rt;
    }

    self->owning_def_rt = delete_after;
}

RenderTarget *RenderManager::defaultRenderTarget() const {
    return self->def_rt;
}

void RenderManager::setActiveRenderTarget(RenderTarget* rt) {
    if (rt != self->current_rt) {
        
        if (self->current_rt != 0)
            self->current_rt->deactivate();

        self->current_rt = rt;
        if (self->current_rt != 0)
            self->current_rt->activate();
    }
}

RenderTarget *RenderManager::activeRenderTarget() const {
    return self->current_rt;
}

void RenderManager::beginScene() {
    ASSERT_MSG(!self->inScene, "nested beginScene()");
    ASSERT_MSG(self->camera_set, "camera matrix not set");
    ASSERT_MSG(self->current_rt != 0 || self->def_rt != 0, "no RenderTarget specified");

    self->statBegin();
    
    self->camera_set = false;
    self->inScene = true;

    setActiveRenderTarget(self->def_rt);
    self->current_rt->beginScene();

    self->transformStateBOS = self->transform.save();
    self->transform.concat(self->cameraMatrix);
    self->frustum.update(self->transform.mvpMatrix());
}

void RenderManager::endScene() {
    ASSERT_MSG(self->inScene, "cannot endScene() without beginScene()");
    self->inScene = false;
    self->transform.restore(self->transformStateBOS);
    self->current_rt->draw();
    self->statEnd();
}

FrameStatistics RenderManager::frameStatistics() {
    return self->stats;
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
