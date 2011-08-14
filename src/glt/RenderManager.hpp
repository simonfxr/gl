#ifndef GLT_RENDER_MANAGER_HPP
#define GLT_RENDER_MANAGER_HPP

#include "glt/ViewFrustum.hpp"
#include "glt/GeometryTransform.hpp"
#include "glt/RenderTarget.hpp"

#include "math/mat4/type.hpp"
#include "math/real.hpp"

namespace glt {

struct FrameStatistics {
    uint32 avg_fps;
    uint32 rt_avg;
    uint32 rt_current;
    uint32 rt_max;
    uint32 rt_min;
};

struct Projection {
    enum Type {
        Identity,
        Perspective
    } type;

    union {
        struct {
            float fieldOfViewRad;
            float z_near;
            float z_far;
        } perspective;
    };

    Projection(Type ty = Identity) :
        type(ty)
    {
        switch (type) {
        case Identity: break;
        case Perspective:
            perspective.fieldOfViewRad = math::PI / 10.f;
            perspective.z_near = 0.5f;
            perspective.z_far = 100.f;
            break;
        default:
            FATAL_ERR("invalid Projection::Type enum");
        }
    }

    static Projection mkPerspective(float fov, float zn, float zf) {
        Projection proj;
        proj.type = Perspective;
        proj.perspective.fieldOfViewRad = fov;
        proj.perspective.z_near = zn;
        proj.perspective.z_far = zf;
        return proj;
    }

    static Projection identity() { return Projection(Identity); }
};

struct RenderManager {
    RenderManager();
    ~RenderManager();
    
    const ViewFrustum& viewFrustum() const;
    
    const GeometryTransform& geometryTransform() const;

    GeometryTransform& geometryTransform();

    void setDefaultProjection(const Projection& proj);

    void updateProjection(float aspectRatio);
    
    void setDefaultRenderTarget(RenderTarget* rt, bool delete_after = false);
    
    RenderTarget* defaultRenderTarget() const;

    void setActiveRenderTarget(RenderTarget *rt);

    void setDefaultRenderTarget();

    RenderTarget *activeRenderTarget() const;

    void beginScene();

    void endScene();

    void shutdown();

    FrameStatistics frameStatistics();
    
private:

    struct Data;
    friend struct Data;
    Data * const self;
    
    RenderManager(const RenderManager& _);
    RenderManager& operator =(const RenderManager& _);
};

math::vec4_t project(const RenderManager& rm, const math::point3_t& localCoord);

math::vec4_t projectWorld(const RenderManager& rm, const math::point3_t& worldCoord);

math::vec4_t projectView(const RenderManager& rm, const math::point3_t& eyeCoord);

Outcode testSphere(const RenderManager& rm, const math::point3_t& center, float rad);

Outcode testPoint(const RenderManager& rm, const math::point3_t& p);

} // namespace glt

#endif
