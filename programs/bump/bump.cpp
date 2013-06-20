#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"
#include "glt/utils.hpp"

#include <vector>
#include <map>
#include <algorithm>

using namespace defs;
using namespace math;

struct Vertex {
    vec3_t position;
    vec3_t normal;
    real tri_id;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position),
                   VERTEX_ATTR(Vertex, normal),
                   VERTEX_ATTR(Vertex, tri_id)
);

template <typename Vertex>
void sphere(glt::Mesh<Vertex>& mesh, real radius, int slices, int stacks);

template <typename V>
void icoSphere(glt::Mesh<V>& mesh, size subdivs);

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
    icoSphere(sphere_model, 0);
    sphere_model.send();

//    GL_CALL(glDisable, GL_CULL_FACE);

//    GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);

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


    real phi = real(engine.gameLoop().animationFrameID()) * inverse(real(100)) * 0.1f;

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

template <typename V>
void icoSphere(glt::Mesh<V>& mesh, size subdivs) {

    struct Tri {
        index32 a, b, c;
        real tri_id;
        Tri(index32 _a, index32 _b, index32 _c) :
            a(_a), b(_b), c(_c), tri_id(0) {}
        Tri(index32 _a, index32 _b, index32 _c, real _tri_id) :
            a(_a), b(_b), c(_c), tri_id(_tri_id) {}
    };
    
    std::vector<vec3_t> vertices;
    std::vector<Tri> tris;

    typedef std::map<index64, index32> VertexCache;
    VertexCache vertex_cache;

#define X .525731112119133606 
#define Z .850650808352039932

    const GLfloat vertex_data[12][3] = {    
        {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
        {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
        {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
    };

    const GLuint elements[20][3] = { 
        {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
        {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
        {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
        {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

    for (index i = 0; i < 12; ++i)
        vertices.push_back(vec3(vertex_data[i]));
    for (index i = 0; i < 20; ++i)
        tris.push_back(Tri(elements[i][0], elements[i][1], elements[i][2], i));

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
            
#define SUBDIV(ab, a, b) do {                                           \
                index64 key = a < b ? (index64(a) << 32 | b) : (index64(b) << 32 | a); \
                VertexCache::const_iterator it = vertex_cache.find(key); \
                if (it != vertex_cache.end()) {                         \
                    ab = it->second;                                    \
                } else {                                                \
                    vec3_t pa = vertices[a];                            \
                    vec3_t pb = vertices[b];                            \
                    vec3_t pab = normalize(real(.5) * (pa + pb));       \
                    vertices.push_back(pab);                            \
                    ab = next_vert++;                                   \
                    vertex_cache[key] = ab;                             \
                }                                                       \
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

    for (index i = 0; i < SIZE(from->size()); ++i) {
        Tri tri = (*from)[i];
        
        vec3_t ps[3] = { vertices[tri.c], vertices[tri.b], vertices[tri.a] };
        for (index k = 0; k < 3; ++k) {
            V v;
            vec3_t p = ps[k];
            glt::primitives::setPoint(v.position, p);
            glt::primitives::setVec(v.normal, p);
            v.tri_id = tri.tri_id;
            mesh.addVertex(v);
        }
    }

    mesh.primType(GL_TRIANGLES);
    mesh.drawType(glt::DrawArrays);
}
