#include "glt/RenderManager.hpp"

#include "opengl.hpp"
#include "glt/utils.hpp"
#include "glt/Transformations.hpp"
#include "glt/GLObject.hpp"
#include "glt/GLPerfCounter.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include "sys/clock.hpp"

#include <algorithm>

namespace glt {

using namespace math;

static const size FS_UPDATE_INTERVAL = 100;


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
    bool projection_outdated;

    uint64 frame_id_current;
    uint64 frame_id_next;

    bool perf_initialized;
    GLPerfCounter perf_counter;
    
    double sum_elapsed;
    double min_elapsed;
    double max_elapsed;
    FrameStatistics stats;

    Data() :
        inScene(false),
        transformStateBOS(transform, 0, 0),
        owning_def_rt(false),
        def_rt(0),
        current_rt(0),
        projection(),
        projection_outdated(true),
        frame_id_current(0),
        frame_id_next(0),
        perf_initialized(false),
        stats()
        {}

    void beginStats();
    void endStats();
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
    self->projection_outdated = true;
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
    self->projection_outdated = false;
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

    self->beginStats();
    
    self->inScene = true;

    setActiveRenderTarget(self->def_rt);
    self->current_rt->beginScene();
    if (self->projection_outdated) {
        size w = self->current_rt->width();
        size h = self->current_rt->height();
        float aspect_ratio = float(w) / float(h);
        updateProjection(aspect_ratio);
    }

    self->transformStateBOS = self->transform.save();
    self->frustum.update(self->transform.vpMatrix());
}

void RenderManager::endScene() {
    ASSERT_MSG(self->inScene, "cannot endScene() without beginScene()");
    self->inScene = false;
    self->transform.restore(self->transformStateBOS);
    self->endStats(); // dont count swap buffers
    if (self->current_rt != 0)
        self->current_rt->draw();
}

FrameStatistics RenderManager::frameStatistics() {
    return self->stats;
}

void RenderManager::Data::beginStats() {
    if (!perf_initialized) {
        perf_initialized = true;
        perf_counter.init(2);
    }

    perf_counter.begin();
    stats.last = perf_counter.query();
    if (stats.last > 0) {
        sum_elapsed += stats.last;
        if (stats.last > max_elapsed)
            max_elapsed = stats.last;
        if (stats.last < min_elapsed)
        min_elapsed = stats.last;
        
        if (frame_id_current >= frame_id_next) {
            stats.min = min_elapsed;
            stats.max = max_elapsed;
            stats.avg = sum_elapsed / double(FS_UPDATE_INTERVAL + (frame_id_current - frame_id_next));
            
            frame_id_next = frame_id_current + FS_UPDATE_INTERVAL;
            sum_elapsed = 0.0;
            min_elapsed = std::numeric_limits<double>::infinity();
            max_elapsed = 0.0;
        }
    }
}

void RenderManager::Data::endStats() {
    perf_counter.end();
    ++frame_id_current;
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
