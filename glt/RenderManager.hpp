#ifndef GLT_RENDER_MANAGER_HPP
#define GLT_RENDER_MANAGER_HPP

#include "glt/ViewFrustum.hpp"
#include "glt/GeometryTransform.hpp"

#include "math/mat4/type.hpp"

namespace glt {

struct RenderManager {
    RenderManager();
    ~RenderManager();
    
    const ViewFrustum& viewFrustum() const;
    
    const GeometryTransform& geometryTransform() const;

    const math::aligned_mat4_t& cameraMatrix() const;
    
    GeometryTransform& geometryTransform();

    void setPerspectiveProjection(float theta, float aspectRatio, float z_near, float z_far);
    
    void setCameraMatrix(const math::mat4_t& m);

    void beginScene();

    void endScene();

private:

    struct Data;
    friend struct Data;
    Data * const self;
    
    RenderManager(const RenderManager& _);
    RenderManager& operator =(const RenderManager& _);
};

math::vec4_t transformModelToWorld(const RenderManager& rm, const math::vec4_t& modelCoord);

math::vec4_t project(const RenderManager& rm, const math::point3_t& localCoord);

math::vec4_t projectView(const RenderManager& rm, const math::point3_t& eyeCoord);

Outcode testSphere(const RenderManager& rm, const math::point3_t& center, float rad);

Outcode testPoint(const RenderManager& rm, const math::point3_t& p);

} // namespace glt

#endif
