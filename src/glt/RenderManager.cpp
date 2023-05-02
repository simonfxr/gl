#include "glt/RenderManager.hpp"

#include "glt/GLObject.hpp"
#include "glt/GLPerfCounter.hpp"
#include "glt/Transformations.hpp"
#include "glt/utils.hpp"
#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "opengl.hpp"
#include "sys/clock.hpp"

#include <bl/optional.hpp>

#include <algorithm>

namespace glt {

using namespace math;

struct SavePointHolder
{
    SavePoint sp;
    explicit SavePointHolder(GeometryTransform &gt) : sp{ gt.save() } {}
};

struct RenderManager::Impl : public RenderManager
{
    ViewFrustum frustum;
    bl::box<GeometryTransform> transform;
    bl::optional<SavePointHolder>
      transformStateBOS; // transform state in begin of scene

    std::shared_ptr<RenderTarget> def_rt;

    // always active
    std::shared_ptr<RenderTarget> current_rt;

    Projection projection;

    uint64_t frame_id_current = 1;
    uint64_t frame_id_last = 0;

    GLPerfCounter perf_counter;

    double sum_elapsed{};
    double min_elapsed{};
    double max_elapsed{};
    FrameStatistics stats;

    bool inScene{ false };
    bool projection_outdated{ true };
    bool perf_initialized{ false };

    void beginStats();
    void endStats();
};

BL_DEF_RUNTIME_SIZED(RenderManager, RenderManager::Impl);

RenderManager &
RenderManager::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

void
RenderManager::shutdown()
{
    if (m().current_rt) {
        m().current_rt->deactivate();
        m().current_rt.reset();
    }

    m().def_rt.reset();

    m().perf_counter.clear();
}

const ViewFrustum &
RenderManager::viewFrustum() const
{
    return m().frustum;
}

const GeometryTransform &
RenderManager::geometryTransform() const
{
    return *m().transform;
}

GeometryTransform &
RenderManager::geometryTransform()
{
    return *m().transform;
}

void
RenderManager::setDefaultProjection(const Projection &proj)
{
    m().projection = proj;
    m().projection_outdated = true;
}

void
RenderManager::updateProjection(math::real aspectRatio)
{
    switch (m().projection.type) {
    case Projection::Identity:
        geometryTransform().loadProjectionMatrix(
          mat4(vec4(1.f, 0.f, 0.f, 0.f),
               vec4(0.f, 1.f, 0.f, 0.f),
               vec4(0.f, 0.f, 1.f, 0.f),
               vec4(0.f, 0.f, 0.f, 1.f)));
        break;
    case Projection::Perspective:
        geometryTransform().loadProjectionMatrix(
          perspectiveProjection(m().projection.perspective.fieldOfViewRad,
                                aspectRatio,
                                m().projection.perspective.z_near,
                                m().projection.perspective.z_far));
        break;
    }
    m().projection_outdated = false;
}

void
RenderManager::setDefaultRenderTarget(const std::shared_ptr<RenderTarget> &rt)
{
    if (rt != m().def_rt) {

        if (m().current_rt && m().current_rt == m().def_rt) {
            WARN("destroying active render target");
            m().current_rt->deactivate();
            m().current_rt.reset();
        }

        m().def_rt = rt;
    }
}

const std::shared_ptr<RenderTarget> &
RenderManager::defaultRenderTarget() const
{
    return m().def_rt;
}

void
RenderManager::setDefaultRenderTarget()
{
    ASSERT(m().def_rt);
    setActiveRenderTarget(m().def_rt);
}

void
RenderManager::setActiveRenderTarget(const std::shared_ptr<RenderTarget> &rt)
{
    if (rt != m().current_rt) {
        if (m().current_rt) {
            m().current_rt->deactivate();
            m().current_rt.reset();
        }

        m().current_rt = rt;
        if (m().current_rt)
            m().current_rt->activate();
    }
}

const std::shared_ptr<RenderTarget> &
RenderManager::activeRenderTarget() const
{
    return m().current_rt;
}

void
RenderManager::beginScene()
{
    ASSERT(!m().inScene, "nested beginScene()");
    ASSERT(m().current_rt != nullptr || m().def_rt != nullptr,
           "no RenderTarget specified");

    m().beginStats();

    m().inScene = true;

    setActiveRenderTarget(m().def_rt);
    m().current_rt->beginScene();
    if (m().projection_outdated) {
        auto w = m().current_rt->width();
        auto h = m().current_rt->height();
        using math::real;
        auto aspect_ratio = static_cast<real>(w) / static_cast<real>(h);
        updateProjection(aspect_ratio);
    }

    m().transformStateBOS.emplace(*m().transform);
    m().frustum.update(m().transform->vpMatrix());
}

void
RenderManager::endScene()
{
    ASSERT(m().inScene, "cannot endScene() without beginScene()");
    m().inScene = false;
    m().transformStateBOS = bl::nullopt; // restore save point
    m().endStats();                      // dont count swap buffers
    if (m().current_rt != nullptr)
        m().current_rt->draw();
}

FrameStatistics
RenderManager::frameStatistics()
{
    m().stats.min = m().min_elapsed;
    m().stats.max = m().max_elapsed;
    m().stats.avg = m().sum_elapsed / static_cast<double>(m().frame_id_current -
                                                          m().frame_id_last);

    m().frame_id_last = m().frame_id_current;
    m().sum_elapsed = 0.0;
    m().min_elapsed = std::numeric_limits<double>::infinity();
    m().max_elapsed = 0.0;

    return m().stats;
}

void
RenderManager::Impl::beginStats()
{
    if (!perf_initialized) {
        perf_initialized = true;
        perf_counter.init(2);

        sum_elapsed = 0;
        min_elapsed = 0;
        max_elapsed = 0;
        ASSERT(frame_id_current > 0);
        frame_id_last = frame_id_current - 1;
    }

    perf_counter.begin();
    stats.last = perf_counter.query();
    if (stats.last > 0) {
        sum_elapsed += stats.last;
        if (stats.last > max_elapsed)
            max_elapsed = stats.last;
        if (stats.last < min_elapsed)
            min_elapsed = stats.last;
    }
}

void
RenderManager::Impl::endStats()
{
    perf_counter.end();
    ++frame_id_current;
}

} // namespace glt
