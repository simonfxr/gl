#ifndef GLT_RENDER_MANAGER_HPP
#define GLT_RENDER_MANAGER_HPP

#include "glt/GeometryTransform.hpp"
#include "glt/RenderTarget.hpp"
#include "glt/ViewFrustum.hpp"

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/real.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace glt {

struct GLT_API FrameStatistics
{
    double avg;
    double min, max;
    double last;

    FrameStatistics() : avg(0), min(0), max(0), last(0) {}
};

struct GLT_API Projection
{
    enum Type : uint8_t
    {
        Identity,
        Perspective
    } type;

    union
    {
        struct
        {
            math::real fieldOfViewRad;
            math::real z_near;
            math::real z_far;
        } perspective;
    };

    Projection(Type ty = Identity) : type(ty)
    {
        switch (type) {
        case Identity:
            break;
        case Perspective:
            perspective.fieldOfViewRad = math::PI / math::real(10);
            perspective.z_near = math::real(0.5);
            perspective.z_far = 100;
            break;
            // default:
            //     FATAL_ERR(ERROR_DEFAULT_STREAM, "invalid Projection::Type
            //     enum");
        }
    }

    static Projection mkPerspective(math::real fov,
                                    math::real zn,
                                    math::real zf)
    {
        Projection proj;
        proj.type = Perspective;
        proj.perspective.fieldOfViewRad = fov;
        proj.perspective.z_near = zn;
        proj.perspective.z_far = zf;
        return proj;
    }

    static Projection identity() { return Projection(Identity); }
};

struct GLT_API RenderManager
{
    RenderManager();
    ~RenderManager();

    const ViewFrustum &viewFrustum() const;

    const GeometryTransform &geometryTransform() const;

    GeometryTransform &geometryTransform();

    void setDefaultProjection(const Projection &proj);

    void updateProjection(math::real aspectRatio);

    void setDefaultRenderTarget(RenderTarget *rt, bool delete_after = false);

    RenderTarget *defaultRenderTarget() const;

    void setActiveRenderTarget(RenderTarget *rt);

    void setDefaultRenderTarget();

    RenderTarget *activeRenderTarget() const;

    void beginScene();

    void endScene();

    void shutdown();

    FrameStatistics frameStatistics();

private:
    DECLARE_PIMPL(GLT_API, self);
};

inline math::vec4_t
project(const RenderManager &rm, const math::point3_t &localCoord)
{
    using math::transform;
    return transform(rm.geometryTransform().mvpMatrix(),
                     math::vec4(localCoord, 1.f));
}

inline math::vec4_t
projectWorld(const RenderManager &rm, const math::point3_t &worldCoord)
{
    using math::transform;
    return transform(rm.geometryTransform().vpMatrix(),
                     math::vec4(worldCoord, 1.f));
}

inline math::vec4_t
projectView(const RenderManager &rm, const math::point3_t &eyeCoord)
{
    using math::transform;
    return transform(rm.geometryTransform().projectionMatrix(),
                     math::vec4(eyeCoord, 1.f));
}

inline Outcode
testSphere(const RenderManager &rm,
           const math::point3_t &center,
           math::real rad)
{
    return testSphere(rm.viewFrustum(), center, rad);
}

inline Outcode
testPoint(const RenderManager &rm, const math::point3_t &p)
{
    return testPoint(rm.viewFrustum(), p);
}

} // namespace glt

#endif
