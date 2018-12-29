#include "ge/Camera.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"

#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"

#include "math/glvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include <algorithm>
#include <map>
#include <vector>

using namespace defs;
using namespace math;

#define VERTEX(V, F, Z)                                                        \
    V(Vertex,                                                                  \
      F(vec3_t,                                                                \
        position,                                                              \
        F(vec3_t, tangent, F(vec3_t, binormal, Z(vec2_t, uv)))))
DEFINE_VERTEX(VERTEX);
#undef VERTEX

template<typename Vertex>
void
sphere(glt::Mesh<Vertex> &mesh, real radius, int slices, int stacks);

template<typename V>
void
icoSphere(glt::Mesh<V> &mesh, size subdivs);

struct Anim
{
    ge::Engine engine;
    glt::Mesh<Vertex> sphere_model;
    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;

    vec3_t light_position{};

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent> & /*unused*/);
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{

    //    icoSphere(sphere_model, 3);
    sphere(sphere_model, 1.f, 200, 100);
    sphere_model.send();

    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glEnable, GL_MULTISAMPLE);
    light_position = vec3(0.f, 0.f, -100.f);

    ev.info.engine.enablePlugin(mouse_look);
    mouse_look.camera(&camera);
    ev.info.engine.enablePlugin(camera);
    camera.frame().origin = vec3(0.f);

    ev.info.success = true;
}

void
Anim::link()
{
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void
Anim::renderScene(const ge::Event<ge::RenderEvent> & /*unused*/)
{
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    auto program = engine.shaderManager().program("sphere");
    ASSERT_MSG(program, "sphere program not found");
    glt::RenderManager &rm = engine.renderManager();

    vec3_t ecLight =
      transformPoint(rm.geometryTransform().viewMatrix(), light_position);

    real phi =
      real(engine.gameLoop().tickID()) * inverse(real(100)) * real(1.3);

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

int
main(int argc, char *argv[])
{

    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}

// based on the GLTools library (OpenGL Superbible gltMakeSphere())
template<typename Vertex>
void
sphere(glt::Mesh<Vertex> &mesh, real rad, int slices, int stacks)
{

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
            sphi = -sphi;

            real sphi2, cphi2;
            sincos(phi + dphi, sphi2, cphi2);
            sphi2 = -sphi2;

#define CALC(II, sa, ca, sb, cb)                                               \
    normal[II] = vec3((sa) * (cb), (sa) * (sb), ca);                           \
    position[II] = rad * normal[II];                                           \
    tangent[II] = vec3((ca) * (cb), (ca) * (sb), -(sa));                       \
    binormal[II] = cross(tangent[II], normal[II]);                             \
    binormal[II] =                                                             \
      normalize(binormal[II] - dot(binormal[II], tangent[II]) * tangent[II]);  \
    uv[II][0] = real(.5) + ::math::atan2(normal[II][2], normal[II][0]) *       \
                             inverse(real(2) * PI);                            \
    uv[II][1] = real(.5) - ::math::asin(normal[II][1]) * inverse(PI);          \
    uv[II][0] *= signum(dot(cross(binormal[II], tangent[II]), normal[II]))

            CALC(0, stheta, ctheta, sphi, cphi);
            CALC(1, stheta2, ctheta2, sphi, cphi);
            CALC(2, stheta, ctheta, sphi2, cphi2);
            CALC(3, stheta2, ctheta2, sphi2, cphi2);

#undef CALC

#define TEST(uv, JJ, v)                                                        \
    ((::math::abs((uv)[0][JJ]) v) + (::math::abs((uv)[1][JJ]) v) +             \
     (::math::abs((uv)[2][JJ]) v))

#define ADD_MESH                                                               \
    do {                                                                       \
        int a[2], b[2];                                                        \
        a[0] = TEST(uv, 0, < real(0.05));                                      \
        a[1] = TEST(uv, 1, < real(0.05));                                      \
        b[0] = TEST(uv, 0, > real(0.95));                                      \
        b[1] = TEST(uv, 1, > real(0.95));                                      \
                                                                               \
        for (int k = 0; k < 3; ++k) {                                          \
            Vertex v;                                                          \
            glt::primitives::setPoint(v.position, position[k]);                \
            glt::primitives::setVec(v.tangent, tangent[k]);                    \
            glt::primitives::setVec(v.binormal, binormal[k]);                  \
            v.uv = uv[k];                                                      \
                                                                               \
            for (int j = 0; j < 2; ++j)                                        \
                if (a[j] > 0 && b[j] > 0 && ::math::abs(v.uv[j]) < 0.5f)       \
                    v.uv[j] = signum(v.uv[j]) * (1.0f - ::math::abs(v.uv[j])); \
                                                                               \
            mesh.addVertex(v);                                                 \
        }                                                                      \
    } while (0)

            ADD_MESH;

#define MOV(i, j)                                                              \
    position[i] = position[j], normal[i] = normal[j], tangent[i] = tangent[j], \
    binormal[i] = binormal[j], uv[i] = uv[j]
            MOV(0, 1);
            MOV(1, 3);
#undef MOV

            ADD_MESH;
#undef ADD_MESH
#undef TEST
        }
    }

    mesh.drawType(glt::DrawArrays);
    mesh.primType(GL_TRIANGLES);
}

template<typename V>
void
icoSphere(glt::Mesh<V> &mesh, size subdivs)
{

    struct Tri
    {
        index32 a, b, c;
        Tri(index32 _a, index32 _b, index32 _c) : a(_a), b(_b), c(_c) {}
    };

    std::vector<vec3_t> vertices;
    std::vector<Tri> tris;

    typedef std::map<index64, index32> VertexCache;
    VertexCache vertex_cache;

#define X .525731112119133606
#define Z .850650808352039932

    const real vertex_data[12][3] = {
        { -X, 0.0, Z }, { X, 0.0, Z },  { -X, 0.0, -Z }, { X, 0.0, -Z },
        { 0.0, Z, X },  { 0.0, Z, -X }, { 0.0, -Z, X },  { 0.0, -Z, -X },
        { Z, X, 0.0 },  { -Z, X, 0.0 }, { Z, -X, 0.0 },  { -Z, -X, 0.0 }
    };

    const GLushort elements[20][3] = {
        { 0, 4, 1 },  { 0, 9, 4 },  { 9, 5, 4 },  { 4, 5, 8 },  { 4, 8, 1 },
        { 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 },  { 5, 2, 3 },  { 2, 7, 3 },
        { 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
        { 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 },  { 7, 2, 11 }
    };

    vertices.reserve(12);
    for (auto i : vertex_data)
        vertices.push_back(vec3(i));
    for (auto element : elements)
        tris.push_back(Tri(element[0], element[1], element[2]));

    std::vector<Tri> tris2;
    std::vector<Tri> *from, *to;
    from = &tris;
    to = &tris2;

    index next_vert = vertices.size();

    for (index k = 0; k < subdivs; ++k) {
        to->clear();

        for (index i = 0; i < SIZE(from->size()); ++i) {
            Tri tri = (*from)[i];
            index ab, bc, ca;

#define SUBDIV(ab, a, b)                                                       \
    do {                                                                       \
        index64 key =                                                          \
          (a) < (b) ? (index64(a) << 32 | (b)) : (index64(b) << 32 | (a));     \
        VertexCache::const_iterator it = vertex_cache.find(key);               \
        if (it != vertex_cache.end()) {                                        \
            (ab) = it->second;                                                 \
        } else {                                                               \
            vec3_t pa = vertices[a];                                           \
            vec3_t pb = vertices[b];                                           \
            vec3_t pab = normalize(real(.5) * (pa + pb));                      \
            vertices.push_back(pab);                                           \
            (ab) = next_vert++;                                                \
            vertex_cache[key] = ab;                                            \
        }                                                                      \
    } while (0)

            SUBDIV(ab, tri.a, tri.b);
            SUBDIV(bc, tri.b, tri.c);
            SUBDIV(ca, tri.c, tri.a);

#undef SUBDIV

            to->push_back(Tri(tri.a, ab, ca));
            to->push_back(Tri(tri.b, bc, ab));
            to->push_back(Tri(tri.c, ca, bc));
            to->push_back(Tri(ab, bc, ca));
        }

        std::swap(from, to);
    }

    to->clear();
    vertex_cache.clear();

    mesh.primType(GL_TRIANGLES);
    mesh.drawType(glt::DrawElements);

    V v;
    for (index i = 0; i < SIZE(vertices.size()); ++i) {
        vec3_t normal = vertices[i];

        real cos_theta = normal[2];
        real sin_theta =
          math::sqrt(normal[0] * normal[0] + normal[1] + normal[1]);

        real cos_phi, sin_phi;
        if (sin_theta == 0) {
            cos_phi = 1;
            sin_phi = 0;
        } else {
            cos_phi = normal[0] / sin_theta;
            sin_phi = normal[1] / sin_theta;
        }

        vec3_t tangent =
          vec3(cos_theta * cos_phi, cos_theta * sin_phi, -sin_theta);
        vec3_t binormal = cross(tangent, normal);
        binormal = normalize(binormal - dot(binormal, tangent) * tangent);
        vec2_t uv{};
        uv[0] = real(.5) +
                ::math::atan2(normal[2], normal[0]) * inverse(real(2) * PI);
        uv[1] = real(.5) - ::math::asin(normal[1]) * inverse(PI);
        uv[0] *= signum(dot(cross(tangent, binormal), normal));

        glt::primitives::setPoint(v.position, normal);
        glt::primitives::setVec(v.tangent, tangent);
        glt::primitives::setVec(v.binormal, binormal);
        v.uv = uv;
        mesh.addVertex(v);
    }

    vertices.clear();

    for (index i = 0; i < SIZE(from->size()); ++i) {
        Tri tri = (*from)[i];

        mesh.addElement(tri.a);
        mesh.addElement(tri.b);
        mesh.addElement(tri.c);
    }
}
