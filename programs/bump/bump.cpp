#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"

using namespace defs;
using namespace math;

struct Vertex {
    vec3_t position;
    vec3_t tangent;
    vec3_t binormal;
    vec2_t uv; // if uv.x < 0 then tangent space is left handed, else
               // right handed
    vec3_t normal;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position),
                   VERTEX_ATTR(Vertex, tangent),
                   VERTEX_ATTR(Vertex, binormal),
                   VERTEX_ATTR(Vertex, uv),
                   VERTEX_ATTR(Vertex, normal)
);

template <typename Vertex>
void sphere(glt::Mesh<Vertex>& mesh, real radius, int slices, int stacks);

struct Anim {
    ge::Engine engine;
    glt::Mesh<Vertex> sphere_model;
    ge::Camera camera;
    vec3_t light_position;
        
    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent>&);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    engine.window().showMouseCursor(false);
    engine.window().grabMouse(true);

//    glt::primitives::
    sphere(sphere_model, 1.f, 200, 100);
    sphere_model.send();

    light_position = vec3(0.f, 0.f, -100.f);

    camera.frame.origin = vec3(0.f);
    camera.registerWith(engine);
    camera.registerCommands(engine.commandProcessor());
    
    ev.info.success = true;
}

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>&) {
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    Ref<glt::ShaderProgram> program = engine.shaderManager().program("sphere");
    ASSERT_MSG(program, "sphere program not found");
    glt::RenderManager& rm = engine.renderManager();

    vec3_t ecLight = transformPoint(rm.geometryTransform().viewMatrix(), light_position);


    real phi = engine.gameLoop().gameTime() * 0.1;

    rm.geometryTransform().dup();
    rm.geometryTransform().scale(vec3(10.f)); // scale radius of sphere
    rm.geometryTransform().concat(glt::rotationMatrix(phi, vec3(0, 1, 0)));
    program->use();
    glt::Uniforms(*program)
        .optional("mvpMatrix", rm.geometryTransform().mvpMatrix())
        .optional("mvMatrix", rm.geometryTransform().mvMatrix())
        .optional("normalMatrix", rm.geometryTransform().normalMatrix())
        .optional("ecLight", ecLight);
    sphere_model.draw();
    rm.geometryTransform().pop();
}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}


// based on the GLTools library (OpenGL Superbible gltMakeSphere())
template <typename Vertex>
void sphere(glt::Mesh<Vertex>& mesh, real rad, int slices, int stacks) {

    real dtheta = PI / stacks;
    real dphi = real(2) * PI / slices;

    for (int i = 0; i < stacks; ++i) {
        real theta = real(i) * dtheta;
        real stheta, ctheta;
        sincos(theta, stheta, ctheta);
        real stheta2, ctheta2;
        sincos(theta + dtheta, stheta2, ctheta2);

        vec3_t position[4];
        vec3_t normal[4];
        vec3_t tangent[4];
        vec3_t binormal[4];
        vec2_t uv[4];

        for (int j = 0; j < slices; ++j) {
            real phi = real(j) * dphi;
            real sphi, cphi;
            sincos(phi, sphi, cphi);
            sphi = - sphi;

            real sphi2, cphi2;
            sincos(phi + dphi, sphi2, cphi2);
            sphi2 = - sphi2;

#define CALC(II, sa, ca, sb, cb)                                        \
            normal[II] = vec3(sa * cb, sa * sb, ca);                    \
            position[II] = rad * normal[II];                            \
            tangent[II] = vec3(ca * cb, ca * sb, - sa);                 \
            binormal[II] = cross(tangent[II], normal[II]);              \
            binormal[II] = normalize(binormal[II] - dot(binormal[II], tangent[II]) * tangent[II]); \
            uv[II][0] = real(.5) + atan2(normal[II][2], normal[II][0]) * inverse(real(2) * PI); \
            uv[II][1] = real(.5) - asin(normal[II][1]) * inverse(PI)
            
            CALC(0, stheta, ctheta, sphi, cphi);
            CALC(1, stheta2, ctheta2, sphi, cphi);
            CALC(2, stheta, ctheta, sphi2, cphi2);
            CALC(3, stheta2, ctheta2, sphi2, cphi2);

#undef CALC
            
#define ADD_MESH                                                        \
            for (int k = 0; k < 3; ++k) {                               \
                Vertex v;                                               \
                glt::primitives::setPoint(v.position, position[k]);     \
                glt::primitives::setVec(v.tangent, tangent[k]);         \
                glt::primitives::setVec(v.binormal, binormal[k]);       \
                uv[k][0] *= dot(cross(binormal[k], tangent[k]), normal[k]) < real(0) ? real(-1) : real(1); \
                v.uv = uv[k];                                           \
                mesh.addVertex(v);                                      \
            }

            ADD_MESH;

#define MOV(i, j) position[i] = position[j], normal[i] = normal[j], tangent[i] = tangent[j], binormal[i] = binormal[j], uv[i] = uv[j]
            MOV(0, 1);
            MOV(1, 3);
#undef MOV

            ADD_MESH;
#undef ADD_MESH

        }
    }

    mesh.drawType(glt::DrawArrays);
    mesh.primType(GL_TRIANGLES);
}
