#include "glt/RenderManager.hpp"

#include "opengl.hpp"
#include "glt/utils.hpp"
#include "glt/Transformations.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include "sys/clock.hpp"

#include <algorithm>

namespace glt {

using namespace math;

static const float FS_UPDATE_INTERVAL = 1.f;
static const float FS_UPDATE_INTERVAL_FAST = 0.1f;

struct RenderManager::Data {
    ViewFrustum frustum;
    GeometryTransform transform;
    bool inScene;
    SavePointArgs transformStateBOS; // transform state in begin of scene

    bool owning_def_rt;
    RenderTarget *def_rt;

    // always active
    RenderTarget *current_rt;

    Projection projection;

    float stat_snapshot;
    float stat_fast_snapshot;
    float framet0;
    float renderAcc;
    float renderAccFast;
    uint32 num_frames_fast;
    uint32 num_frames;
    
    FrameStatistics stats;

    Data() :
        inScene(false),
        transformStateBOS(transform, 0, 0),
        owning_def_rt(false),
        def_rt(0),
        current_rt(0),
        stat_snapshot(NEG_INF),
        stat_fast_snapshot(NEG_INF),
        renderAcc(1.f),
        renderAccFast(1.f)
        {}

    float now() {
        return float(sys::queryTimer());
    }

    void statBegin() {
        framet0 = now();
        float diff = framet0 - stat_snapshot;
        if (diff >= FS_UPDATE_INTERVAL) {
            stats.avg_fps = uint32(float(num_frames) / diff);
            if (renderAcc != 0)
                stats.rt_avg = uint32(float(num_frames) / renderAcc);
            else
                stats.rt_avg = 0;
            renderAcc = 0.f;
            num_frames = 0;
            stat_snapshot = framet0;
            stats.rt_min = 0xFFFFFFFF;
            stats.rt_max = 0;
        }

        float diff2 = framet0 - stat_fast_snapshot;
        if (diff2 >= FS_UPDATE_INTERVAL_FAST) {
            if (renderAccFast != 0)
                stats.rt_current = uint32(float(num_frames_fast) / renderAccFast);
            else
                stats.rt_avg = 0;
            renderAccFast = 0.f;
            num_frames_fast = 0;
            stat_fast_snapshot = framet0;
        }
    }

    void statEnd() {
        float frameDur = now() - framet0;
        renderAcc += frameDur;
        renderAccFast += frameDur;
        uint32 fps = uint32(frameDur != 0 ? math::recip(frameDur) : 0);
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
    shutdown();
    delete self;
}

void RenderManager::shutdown() {
    if (self->current_rt != 0)  {
        self->current_rt->deactivate();
        self->current_rt = 0;
    }
    
    if (self->owning_def_rt) {
        delete self->def_rt;
        self->def_rt = 0;
    }
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

void RenderManager::setDefaultProjection(const Projection& proj) {
    self->projection = proj;
}

void RenderManager::updateProjection(float aspectRatio) {
    switch (self->projection.type) {
    case Projection::Identity:
        geometryTransform().loadProjectionMatrix(
            mat4(vec4(1.f, 0.f, 0.f, 0.f),
                 vec4(0.f, 1.f, 0.f, 0.f),
                 vec4(0.f, 0.f, 1.f, 0.f),
                 vec4(0.f, 0.f, 0.f, 1.f)));
        break;
    case Projection::Perspective:
        geometryTransform().loadProjectionMatrix(
            perspectiveProjection(self->projection.perspective.fieldOfViewRad,
                                  aspectRatio,
                                  self->projection.perspective.z_near,
                                  self->projection.perspective.z_far));
        break;
    default:
        ERR("invalid projection type");
    }
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

void RenderManager::setDefaultRenderTarget() {
    ASSERT(self->def_rt);
    setActiveRenderTarget(self->def_rt);
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
    ASSERT_MSG(self->current_rt != 0 || self->def_rt != 0, "no RenderTarget specified");

    self->statBegin();
    
    self->inScene = true;

    setActiveRenderTarget(self->def_rt);
    self->current_rt->beginScene();

    self->transformStateBOS = self->transform.save();
    self->frustum.update(self->transform.vpMatrix());
}

void RenderManager::endScene() {
    ASSERT_MSG(self->inScene, "cannot endScene() without beginScene()");
    self->inScene = false;
    self->transform.restore(self->transformStateBOS);
    GL_CALL(glFinish, );
    if (self->current_rt != 0)
        self->current_rt->draw();
    self->statEnd();
}

FrameStatistics RenderManager::frameStatistics() {
    return self->stats;
}

vec4_t project(const RenderManager& rm, const point3_t& localCoord) {
    return transform(rm.geometryTransform().mvpMatrix(), vec4(localCoord, 1.f));
}

vec4_t projectWorld(const RenderManager& rm, const point3_t& worldCoord) {
    return transform(rm.geometryTransform().vpMatrix(), vec4(worldCoord, 1.f));
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
