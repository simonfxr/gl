
#include "math/real.hpp"
#include "math/vec3.hpp"

#include "glt/utils.hpp"
#include "glt/Mesh.hpp"
#include "glt/primitives.hpp"
#include "glt/color.hpp"

#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

using namespace defs;
using namespace math;

struct Vertex {
    point3_t position;
    direction3_t normal;
    glt::color color;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position),
                   VERTEX_ATTR(Vertex, normal),
                   VERTEX_ATTR(Vertex, color));

static const real PLANE_DIM = 30.f;

static const real SPHERE_RAD = 10.f;

struct Anim {
    ge::Engine engine;
    ge::Camera camera;

    direction3_t plane_orientation;
    glt::Mesh<Vertex> plane;
    glt::CubeMesh<Vertex> cube;
    glt::Mesh<Vertex> tetrahedron;
    
    void link(ge::EngineOptions&);
    void init(const ge::Event<ge::InitEvent>&);
    void render(const ge::Event<ge::RenderEvent>&);

    void renderPlane();
};

void Anim::link(ge::EngineOptions& opts) {
    opts.inits.reg(ge::Init, ge::makeEventHandler(this, &Anim::init));
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::render));
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {

    camera.registerWith(engine);
    camera.registerCommands(engine.commandProcessor());

    camera.frame.origin = vec3(0.f, 30.f, 0.f);
    camera.frame.setXZ(vec3(1, 0, 0), vec3(0, 1, 0));

    GL_CALL(glEnable, GL_CULL_FACE);

    engine.gameLoop().ticksPerSecond(100);
    engine.gameLoop().sync(false);
    
    engine.window().grabMouse(true);
    engine.window().showMouseCursor(false);
    
    plane_orientation = vec3(0, 1.f, 0);
    
    {
        Vertex v;
        v.normal = plane_orientation;
        v.position = vec3(1.f, 0.f, 1.f); plane.addVertex(v);
        v.position = vec3(1.f, 0.f, 0.f); plane.addVertex(v);
        v.position = vec3(0.f, 0.f, 0.f); plane.addVertex(v);
        plane.addVertex(v);
        v.position = vec3(0.f, 0.f, 1.f); plane.addVertex(v);
        v.position = vec3(1.f, 0.f, 1.f); plane.addVertex(v);
    }

    plane.drawType(glt::DrawArrays);
    plane.primType(GL_TRIANGLES);
    plane.send();
    plane.freeHost();

    {
        real s = 1.f;
        real h1 = math::sqrt(3) / 2.f * s;
        real h2 = math::sqrt(6) / 3.f * s;
        point3_t A, B, C, D;
        A = vec3(- h1 * (2/3.f), 0, 0);
        B = vec3(h1 * (1/3.f), 0, s/2);
        C = vec3(h1 * (1/3.f), 0, -s/2);
        D = vec3(0, h2, 0);

#define TRI(a, b, c, col) TRI0(a, b, c, col)
#define TRI0(a, b, c, col)                       \
        v.normal = cross(b - a, c - a);          \
        v.color = (col);                         \
        v.position = a; t.addVertex(v);          \
        v.position = b; t.addVertex(v);          \
        v.position = c; t.addVertex(v);

        
        glt::Mesh<Vertex>& t = tetrahedron;
        Vertex v;
        TRI(A, C, B, glt::color(0xFF, 0, 0));
        TRI(A, B, D, glt::color(0, 0xFF, 0));
        TRI(B, C, D, glt::color(0, 0, 0xFF));
        TRI(C, A, D, glt::color(0xFF, 0xFF, 0));

#undef TRI
#undef TRI0
    }

    tetrahedron.drawType(glt::DrawArrays);
    tetrahedron.primType(GL_TRIANGLES);
    tetrahedron.send();

    glt::primitives::unitCube(cube);
    cube.send();
    
    ev.info.success = true;
}

void Anim::render(const ge::Event<ge::RenderEvent>&) {
    engine.renderManager().activeRenderTarget()->clearColor(glt::color(0xFF, 0xFF, 0xFF));
    engine.renderManager().activeRenderTarget()->clear();

    renderPlane();

    // {
    //     Ref<glt::ShaderProgram> shader = engine.shaderManager().program("plane");
    //     glt::GeometryTransform& gt = engine.renderManager().geometryTransform();
        
    //     glt::SavePoint sp(gt.save());

    //     gt.scale(vec3(10.f));
        
    //     ASSERT(shader);
    //     shader->use();

    //     glt::Uniforms(*shader)
    //         .optional("mvpMatrix", gt.mvpMatrix())
    //         .optional("mvMatrix", gt.mvMatrix())
    //         .optional("normalMatrix", gt.normalMatrix());

    //     cube.draw();
    // }
}

void Anim::renderPlane() {
    glt::GeometryTransform& gt = engine.renderManager().geometryTransform();
    glt::SavePoint sp(gt.save());

    gt.scale(vec3(PLANE_DIM));
    gt.translate(- vec3(0.5f, 0.f, 0.5f));

    Ref<glt::ShaderProgram> shader = engine.shaderManager().program("plane");
    ASSERT(shader);
    shader->use();

    glt::Uniforms(*shader)
        .mandatory("mvpMatrix", gt.mvpMatrix())
//        .mandatory("mvMatrix", gt.mvMatrix())
        .mandatory("normalMatrix", gt.normalMatrix());
//    plane.draw();
    tetrahedron.draw();
}

int main(int argc, char *argv[]) {
    Anim anim;
    ge::EngineOptions opts;
    anim.link(opts);
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
