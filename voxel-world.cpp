#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <limits.h>
#include <memory>
#include <bitset>
#include <algorithm>

#include <SFML/Graphics/Image.hpp>

#include "defs.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/math.hpp"
#include "math/ivec3.hpp"

#include "ge/GameWindow.hpp"

#include "glt/utils.hpp"
#include "glt/RenderManager.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"
#include "glt/Frame.hpp"
#include "glt/color.hpp"
#include "glt/Transformations.hpp"

#ifdef MESH_GENBATCH
#include "glt/GenBatch.hpp"
#else
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#endif

using namespace math;

static const float FPS_UPDATE_INTERVAL = 3.f;
static const vec3_t BLOCK_DIM = vec3(0.25f);
static const vec3_t LIGHT_DIR = vec3(+0.21661215f, +0.81229556f, +0.5415304f);

static const int32 N = 196;
static const int32 SPHERE_POINTS_FACE = 32;
static const int32 SPHERE_POINTS = SPHERE_POINTS_FACE * 6;
static const std::string WORLD_MODEL_FILE = "voxel-world.mdl";

#define time(op) do {                                                   \
        float T0 = now();                                               \
        (op);                                                           \
        float diff = now() - T0;                                        \
        std::cerr << #op << " took " << diff << " seconds." << std::endl; \
    } while (0)

struct MaterialProperties {
    float ambientContribution;
    float diffuseContribution;
    float specularContribution;
    float shininess;

    MaterialProperties() {}
    MaterialProperties(float amb, float diff, float spec, float sh) :
        ambientContribution(amb), diffuseContribution(diff), specularContribution(spec),
        shininess(sh) {}
};

static const MaterialProperties BLOCK_MAT(0, 1.f, 0.f, 0.f);

static float noise3D(const vec3_t& p);
static float sumNoise3D(const vec3_t& p, uint32 octaves, float fmin, float phi, bool abs = false);
static void pointsOnSphere(uint32 n, vec3_t *points);

struct Vertex {
    vec3_t position;
    uint32 color;
    vec4_t normal;  // last component is diffuse factor
};

#ifdef MESH_GENBATCH

static const glt::Attr vertexAttrVals[] = {
    glt::attr::vec3(offsetof(Vertex, position)),
    glt::attr::color(offsetof(Vertex, color)),
    glt::attr::vec4(offsetof(Vertex, normal))
};

static const glt::Attrs<Vertex> vertexAttrs(
    ARRAY_LENGTH(vertexAttrVals), vertexAttrVals
);

typedef glt::GenBatch<Vertex> CubeMesh;
#define FREEZE_MESH(m) m.freeze()
#define CUBE_MESH(m) m.primType(GL_QUADS)
#define KEEP_MESH_DATA(m) m.deleteAfterFreeze(false)

#else

DEFINE_VERTEX_ATTRS(vertexAttrs, Vertex,
    VERTEX_ATTR(Vertex, position),
    VERTEX_ATTR_AS(Vertex, color, glt::color),
    VERTEX_ATTR(Vertex, normal)
);

typedef glt::CubeMesh<Vertex> CubeMesh;

#define FREEZE_MESH(m) m.send()
#define CUBE_MESH(m) UNUSED(m)
#define KEEP_MESH_DATA(m) UNUSED(m)

#endif

struct Timer {
private:
    const ge::GameWindow* win;
    bool repeat;
    float countdown;
    float timestamp;

public:
    Timer() : win(0), repeat(false), countdown(0.f), timestamp(0.f) {}

    void start(const ge::GameWindow& _win, float _countdown, bool _repeat = false);
        
    bool fire();
};

void Timer::start(const ge::GameWindow& _win, float _countdown, bool _repeat) {
    win = &_win;
    repeat = _repeat;
    countdown = _countdown;
    timestamp = win->realTime();
}

bool Timer::fire() {
    float now = win->realTime();
    if (now < timestamp + countdown)
        return false;
    if (repeat)
        timestamp = now;
    return true;
}

struct Anim EXPLICIT : public ge::GameWindow {

    glt::RenderManager rm;
    glt::ShaderManager sm;
    glt::Frame         camera;
    CubeMesh cubeModel;
    glt::ShaderProgram voxelShader;
    CubeMesh worldModel;
    vec3_t sphere_points[SPHERE_POINTS];

    float gamma_correction;

    vec3_t ecLightDir;
    
    Timer fpsTimer;

    bool write_model;
    bool read_model;

    Anim() :
        cubeModel(vertexAttrs),
        voxelShader(sm),
        worldModel(vertexAttrs)
        {}
    
    bool onInit() OVERRIDE;
    void initWorld();
    bool loadShaders();
    void animate() OVERRIDE;
    void renderScene(float interpolation) OVERRIDE;
    void renderBlocks();
    void windowResized(uint32 width, uint32 height) OVERRIDE;
    void mouseMoved(int32 dx, int32 dy) OVERRIDE;
    void keyStateChanged(const sf::Event::KeyEvent& key, bool pressed) OVERRIDE;
    void handleInternalEvents() OVERRIDE;

    bool readModel(const std::string& file, CubeMesh& model);
    bool writeModel(const std::string& file, const CubeMesh& model);
};

static void makeUnitCube(CubeMesh& cube);

std::ostream& operator <<(std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

bool Anim::onInit() {
    sm.verbosity(glt::ShaderManager::Info);
    configureShaderVersion(sm);

    GLuint vao;
    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));


    if (GLEW_ARB_multisample) {
        std::cerr << "multisampling support available" << std::endl;

        GL_CHECK(glEnable(GL_MULTISAMPLE_ARB));
    }

    rm.setDefaultRenderTarget(&this->renderTarget());

    sm.addPath("shaders");

    ticksPerSecond(100);
    synchronizeDrawing(true);

    gamma_correction = 1.8f;
    
    grabMouse(true);

    fpsTimer.start(*this, FPS_UPDATE_INTERVAL, true);

    KEEP_MESH_DATA(cubeModel);
    makeUnitCube(cubeModel);

    if (!loadShaders())
        return false;

    srand(0xDEADBEEF);

    CUBE_MESH(worldModel);
    pointsOnSphere(SPHERE_POINTS, sphere_points);

    if (read_model) {
        if (!readModel(WORLD_MODEL_FILE, worldModel)) {
            ERR("couldnt read model file!");
            return false;
        }
    } else {
        initWorld();
    }

    if (write_model) {
        if (!writeModel(WORLD_MODEL_FILE, worldModel))
            ERR("couldnt write model file");
    }

    FREEZE_MESH(worldModel);

    return true;
}

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

static vec4_t randColor() {
    return vec4(vec3(rand1(), rand1(), rand1()), 1.f);
}

// struct World {

//     typedef std::bitset<N * N * N> Bitmap;
//     Bitmap bits;

//     bool operator()(uint32 i, uint32 j, uint32 k) const {
//         return bits[i * N * N + j * N + k];
//     }

//     Bitmap::reference operator()(uint32 i, uint32 j, uint32 k) {
//         return bits[i * N * N + j * N + k];
//     }

//     void zero() { bits.reset(); }
// };

struct World {
    typedef unsigned long Word;
    
    static const uint32 WORD_BITS = sizeof (Word) * 8;
    static const uint32 NBYTES = (N * N * N + WORD_BITS - 1) / WORD_BITS;
    
    Word bits[NBYTES];
    
    bool  operator()(uint32 i, uint32 j, uint32 k) const {
        uint32 bitidx = i * N * N + j * N + k;
        uint32 idx = bitidx / WORD_BITS;
        uint32 bit = bitidx % WORD_BITS;
        return (bits[idx] & (Word(1) << bit)) != 0;
    }

    struct Ref {
        Word *w;
        Word bit;
        Ref(Word *_w, Word _b) : w(_w), bit(_b) {}
        Ref& operator =(bool x) {
            Word mask = Word(1) << bit;
            *w = x ? *w | mask : *w & ~mask;

            // branching seems to be faster than blending
            // *w |= Word(x) << bit;
            // *w &= ~(Word(!x) << bit);
            
            return *this;
        }

        operator bool() { return (*w & (Word(1) << bit)) != 0; }
    };

    typedef Ref reference;
    
    reference operator()(uint32 i, uint32 j, uint32 k) {
        uint32 bitidx = i * N * N + j * N + k;
        uint32 idx = bitidx / WORD_BITS;
        uint32 bit = bitidx % WORD_BITS;
        return Ref(&bits[idx], bit);
    }
    
    void zero() { memset(bits, 0, sizeof bits); }

    static void bitwiseAnd(World& r, const World& a, const World& b) {
        for (int32 i = 0; i < N; ++i)
            for (int32 j = 0; j < N; ++j)
                for (int32 k = 0; k < N; ++k)
                    r(i, j, k) = a(i, j, k) && b(i, j, k);
    }
};

bool foo_get(World& w, uint32 i, uint32 j, uint32 k) {
    return w(i, j, k);
}

void foo_set(World& w, uint32 i, uint32 j, uint32 k, bool val) {
    w(i, j, k) = val;
}

struct Faces {
    vec3_t f[2]; // front, up, right, bot, down, left
};

static const uint32 RAY_SAMPLES = 1.9 * (N + 1); // about sqrt(3) * N, maximum ray length

struct Ray {
    Faces lightContrib;
    ivec3_t offset[RAY_SAMPLES];
};

struct Rays {
    Faces sumcos;
    Ray rays[SPHERE_POINTS];
    uint32 faceOffsets[6];
    uint32 faceLengths[6];
    Faces lightContribFace[6];
};

struct GlobalOcc {
    Faces lambertFactor[N][N][N];
};

int32 faceIdx(const vec3_t& dir) {
    direction3_t as = abs(dir);
    int32 maxi;
    if (as.x >= as.y && as.x >= as.z)
        maxi = 0;
    else if (as.y >= as.x && as.y >= as.z)
        maxi = 1;
    else
        maxi = 2;

    int32 side = dir[maxi] < 0 ? 1 : 0;
    return side * 3 + maxi;
}

Faces cosfaces(const direction3_t& dir, Faces& sumcos) {
    direction3_t as = abs(dir);
    int32 i = faceIdx(dir);
    int32 comp = i % 3;
    int32 side = i / 3;
    
    sumcos.f[side][comp]++;
    Faces f = { { vec3(0.f), vec3(0.f) } };
    f.f[side][comp] = as[comp];
    return f;
}

void initRay(Ray& r, const direction3_t& d, const vec3_t& step, Faces& sumcos) {
    vec3_t p = vec3(0.f);
    ivec3_t lastidx = ivec3(p);

    uint32 i = 0;
    while (i < RAY_SAMPLES) {
        ivec3_t idx = ivec3(p);
        if (idx != lastidx)
            lastidx = idx, r.offset[i++] = idx;
        p += step;
    }

    r.lightContrib = cosfaces(d, sumcos);
}


bool faceIdxLessThan(const vec3_t& a, const vec3_t& b) {
    return faceIdx(a) < faceIdx(b);
}

void initRays(Rays &rays, vec3_t dirs[]) {

    for (int32 i = 0; i < SPHERE_POINTS; ++i)
        dirs[i] = normalize(dirs[i]);

    std::sort(dirs, dirs + SPHERE_POINTS, faceIdxLessThan);

    uint32 off = 0;
    int face = -1;
    for (int32 i = 0; i < SPHERE_POINTS; ++i) {
        int32 face1 = faceIdx(dirs[i]);
        if (face1 != face) {
            
            if (face >= 0) {
                rays.faceOffsets[face] = off;
                rays.faceLengths[face] = i - off;
            }
            
            face = face1;
            off = i;
        }
    }

    rays.faceOffsets[5] = off;
    rays.faceLengths[5] = SPHERE_POINTS - off;

    for (uint32 i = 0; i < 6; ++i) {
        std::cerr << "side " << i << ": " << "number of rays: " << rays.faceLengths[i] << " beginning at " << rays.faceOffsets[i] << std::endl;
    }
    
    const vec3_t center = vec3(N * 0.5f);

    float cos0 = 0.f, cos1 = 0.f, cos2 = 0.f;
    float cos3 = 0.f, cos4 = 0.f, cos5 = 0.f;

#pragma omp parallel for reduction(+:cos0, cos1, cos2, cos3, cos4, cos5)
    for (int32 i = 0; i < SPHERE_POINTS; ++i) {
        const direction3_t& d = dirs[i];
        vec3_t p = N * (d * 0.5f + vec3(0.5f));
        vec3_t step = (p - center) / (10.f * N);
        Faces sum1 = { { vec3(0.f), vec3(0.f) } };
        initRay(rays.rays[i], d, step, sum1);
        cos0 += sum1.f[0][0];
        cos1 += sum1.f[0][1];
        cos2 += sum1.f[0][2];
        cos3 += sum1.f[1][0];
        cos4 += sum1.f[1][1];
        cos5 += sum1.f[1][2];
    }

    rays.sumcos.f[0][0] = cos0;
    rays.sumcos.f[0][1] = cos1;
    rays.sumcos.f[0][2] = cos2;
    rays.sumcos.f[1][0] = cos3;
    rays.sumcos.f[1][1] = cos4;
    rays.sumcos.f[1][2] = cos5;

    for (uint32 face = 0; face < 6; ++face) {
        uint32 off = rays.faceOffsets[face];
        uint32 len = rays.faceLengths[face];
        Faces sum = { { vec3(0.f), vec3(0.f) } };
        
        for (uint32 i = off; i < off + len; ++i) {
            sum.f[0] += rays.rays[i].lightContrib.f[0];
            sum.f[1] += rays.rays[i].lightContrib.f[1];
        }
        
        rays.lightContribFace[face] = sum;
    }
}

void testray(const World& w, World& vis, int32 i, int32 j, int32 k, int32 di, int32 dj, int32 dk) {
    for (;;) {
        
        if (i < 0 || i >= N) break;
        if (j < 0 || j >= N) break;
        if (k < 0 || k >= N) break;

        vis(i, j, k) = true;

        if (w(i, j, k)) break;

        i += di;
        j += dj;
        k += dk;
    }
}

void sendRays(const World& w, World& vis, int32 i, int32 j, int32 k) {
    testray(w, vis, i, j, k, +1, 0, 0);    
    testray(w, vis, i, j, k, -1, 0, 0);
    testray(w, vis, i, j, k, 0, +1, 0);     
    testray(w, vis, i, j, k, 0, -1, 0);
    testray(w, vis, i, j, k, 0, 0, +1); 
    testray(w, vis, i, j, k, 0, 0, -1);
}

bool visible(const World& w, const World& vis, int32 i, int32 j, int32 k) {
    return i < 0 || i >= N ||
           j < 0 || j >= N ||
           k < 0 || k >= N ||
        (!w(i, j, k) && vis(i, j, k));
}

World *filterOccluded(const World& w) {

    World *ret = new World;
    World& vis = *ret;

    memset(&vis, 0, sizeof vis);
    
    bool fixpoint = false;
    while (!fixpoint) {
        fixpoint = true;
#pragma omp parallel for
        for (int32 i = 0; i < N; ++i) {
            for (int32 j = 0; j < N; ++j) {
                for (int32 k = 0; k < N; ++k) {
                    if (!vis(i, j, k) &&
                        (visible(w, vis, i + 1, j, k) ||
                         visible(w, vis, i - 1, j, k) ||
                         visible(w, vis, i, j + 1, k) ||
                         visible(w, vis, i, j - 1, k) ||
                         visible(w, vis, i, j, k + 1) ||
                         visible(w, vis, i, j, k - 1)))
                    {
                        fixpoint = false;
                        vis(i, j, k) = true;
                        if (!w(i, j, k))
                            sendRays(w, vis, i, j, k);
                    }
                }
            }
        }
    }

    return ret;                
}

Faces trace(const World& w, const Rays& rs, const ivec3_t& p0) {
    Faces diffContr = { { vec3(0.f), vec3(0.f) } };

    // // front, up, right, bot, down, left
    // ivec3_t faceNorms[6] = {
    //     ivec3(0, 0, +1), ivec3(0, +1, 0), ivec3(+1, 0, 0),
    //     ivec3(0, 0, -1), ivec3(0, -1, 0), ivec3(-1, 0, 0)
    // };

    for (int32 side = 0; side < 6; ++side) {

        uint32 off = rs.faceOffsets[side];
        uint32 len = rs.faceLengths[side];

        // const ivec3_t& norm = faceNorms[side];
        // const ivec3_t p1 = p0 + norm;

        // bool onCubeSurface =  p1.x < 0 || p1.x >= N ||
        //     0 > p1.y || p1.y >= N ||
        //     0 > p1.z || p1.z >= N;
        
        // if (onCubeSurface) { // all rays pass
        //     diffContr.f[0] += rs.lightContribFace[side].f[0];
        //     diffContr.f[1] += rs.lightContribFace[side].f[1];
        //     continue;
        // }

        // if (w(p1.x, p1.y, p1.z)) { // all fail
        //     continue;
        // }

        for (uint32 i = off; i < off + len; ++i) {
            const Ray& ray = rs.rays[i];
            ivec3_t p = p0 + ray.offset[0];
            uint32 j = 0;
            bool escaped = true;
            while (j + 1 < RAY_SAMPLES &&
                   p.x >= 0 && p.x < N &&
                   p.y >= 0 && p.y < N &&
                   p.z >= 0 && p.z < N) {
                
                if (w(p.x, p.y, p.z)) {
                    escaped = false;
                    break;
                }
                
                ++j;
                p = p0 + ray.offset[j];
            }
            
            if (escaped) {
                diffContr.f[0] += ray.lightContrib.f[0];
                diffContr.f[1] += ray.lightContrib.f[1];
            }
        }
    }

    diffContr.f[0] /= rs.sumcos.f[0];
    diffContr.f[1] /= rs.sumcos.f[1];
        
    return diffContr;
}

bool visibleFace(const World& w, const World& vis, int32 i, int32 j, int32 k, const vec3_t& normal) {
    i += (int32) normal.x;
    j += (int32) normal.y;
    k += (int32) normal.z;
    return visible(w, vis, i, j, k);
}

bool blockAt(const vec3_t& p) {
    float r = sumNoise3D(p, 4, 1.2, 0.5, true);
    float distC = length(p - vec3(0.5f)) / length(vec3(0.5f));
    return (2 * p.y * p.y + 0.3) * (1 - distC) * r > 0.2f;
}

void createGeometry(World& world) {
#pragma omp parallel for
    for (int32 i = 0; i < N; ++i)
        for (int32 j = 0; j < N; ++j)
            for (int32 k = 0; k < N; ++k)
                world(i, j, k) = blockAt(vec3(i, j, k) * (1.f / N));
}

void createOcclusionMap(GlobalOcc& occ, const World& hull, const World& world, const Rays& rays) {
#pragma omp parallel for
    for (int32 i = 0; i < N; ++i)
        for (int32 j = 0; j < N; ++j)
            for (int32 k = 0; k < N; ++k)
                if (hull(i, j, k))
                    occ.lambertFactor[i][j][k] = trace(world, rays, ivec3(i, j, k));
}

void andWorld(World& r, const World& a, const World& b) {
    World::bitwiseAnd(r, a, b);
}

void orWorldNot(World& r, const World& a, const World& b) {
    for (int32 i = 0; i < N; ++i)
        for (int32 j = 0; j < N; ++j)
            for (int32 k = 0; k < N; ++k)
                r(i, j, k) = a(i, j, k) || !b(i, j, k);
}

struct Vertices {
    uint32 size;
    Vertex *verts;
    Vertices(uint32 s) : size(s), verts(new Vertex[s]) {}
    ~Vertices() { delete[] verts; }
};

const ivec3_t neighOffs[6] = {
    ivec3(0, 0, +1), ivec3(0, +1, 0), ivec3(+1, 0, 0),
    ivec3(0, 0, -1), ivec3(0, -1, 0), ivec3(-1, 0, 0)
};

bool hasNeighbours(const World& w, const World& vis, const ivec3_t& p) {
    if (!w(p.x, p.y, p.z))
        return false;
    for (uint32 i = 0; i < 6; ++i) {
        const ivec3_t p1 = p + neighOffs[i];
        bool onCubeSurf = 0 > p1.x || p1.x >= N ||
            0 > p1.y || p1.y >= N ||
            0 > p1.z || p1.z >= N;
        if (onCubeSurf) return true;
        if (!w(p1.x, p1.y, p1.z) && vis(p1.x, p1.y, p1.z)) return true;
    }
    return false;
}

void Anim::initWorld() {

    std::auto_ptr<World> worldp(new World);
    World& world = *worldp;

    world.zero();
    time(createGeometry(world));
    
    std::auto_ptr<Rays> raysp(new Rays);
    Rays& rays = *raysp;
    time(initRays(rays, sphere_points));
    std::cerr << "rays crossing planes: " << rays.sumcos.f[0] << ", " << rays.sumcos.f[1] << std::endl;

    World *visptmp;
    time(visptmp = filterOccluded(world));
    std::auto_ptr<World> visp(visptmp);
    World& vis = *visp;

    uint32 permut[N];
    for (int32 i = 0; i < N; ++i)
        permut[i] = i;

    for (int32 i = 0; i < N - 1; ++i) {
        uint32 j = rand() % (N - i);
        uint32 t = permut[i];
        permut[i] = permut[i + j];
        permut[i + j] = t;
    }

    std::auto_ptr<World> worldAndVisp(new World);
    World& worldAndVis = *worldAndVisp;
    andWorld(worldAndVis, world, vis);

    std::auto_ptr<World> worldOrVisp(new World);
    World& worldOrVis = *worldOrVisp;
    orWorldNot(worldOrVis, world, vis);

    std::auto_ptr<World> hullp(new World);
    World& hull = *hullp;

    for (int32 i = 0; i < N; ++i)
        for (int32 j = 0; j < N; ++j)
            for (int32 k = 0; k < N; ++k)
                hull(i, j, k) = hasNeighbours(world, vis, ivec3(i, j, k));


    std::auto_ptr<GlobalOcc> occmapp(new GlobalOcc);
    GlobalOcc& occmap = *occmapp;
    time(createOcclusionMap(occmap, hull, worldOrVis, rays));

    // time({
    //         for (uint32 i = 0; i < 15; ++i)
    //             andWorld(worldAndVis, world, vis);
    //         });

    uint32 blocks = 0;
    uint32 visBlocks = 0;
    uint32 faces = 0;
    uint32 visFaces = 0;


    std::auto_ptr<Vertices> vertsp(new Vertices(cubeModel.size()));
    Vertices& verts = *vertsp;

    for (uint32 i = 0; i < verts.size; ++i)
        verts.verts[i] = cubeModel.at(i);

    for (int32 i = 0; i < N; ++i) {
        for (int32 j = 0; j < N; ++j) {
            for (int32 k = 0; k < N; ++k) {
                uint32 ii = permut[i];
                uint32 jj = permut[permut[j]];
                uint32 kk = permut[permut[permut[k]]];

                if (world(ii, jj, kk))
                    ++blocks;

                if (vis(ii, jj, kk) && world(ii, jj, kk)) {
                    ++visBlocks;

                    const Faces& diffContr = occmap.lambertFactor[ii][jj][kk];

                    float height = (float) jj / N;
                    float hh = height * height * height;
                        
                    glt::color col = glt::color(vec4(hh , (float) ii / N, 1 - hh, 1));
//                    uint32 col = glt::color(randColor()).rgba();
                    vec3_t offset = BLOCK_DIM * vec3(ii, jj, kk);
                    for (uint32 r = 0; r < verts.size; ++r) {
                        Vertex v = verts.verts[r];
                        ++faces;
                        if (visibleFace(world, vis, ii, jj, kk, vec3(v.normal))) {
                            ++visFaces;
                            ivec3_t n = ivec3(vec3(v.normal));
//                            std::cerr << "n: " << vec3(n) << std::endl;
                            int32 face = n.x + n.y * 2 + n.z * 3;
                            int32 side = face < 0 ? 1 : 0;
                            face = abs(face) - 1;
                            v.position *= BLOCK_DIM;
                            v.position += offset;
                            v.normal.w = diffContr.f[side][face];
                            v.color = col.rgba;
                            worldModel.add(v);
                        }
                    }
                }
            }
        }
    }

    std::cerr << "blocks: " << blocks << ", visible blocks: " << visBlocks << std::endl;
    std::cerr << "faces: " << faces << ", visible faces: " << visFaces << std::endl;
}

bool Anim::loadShaders() {
    bool ok = true;

    glt::ShaderProgram vs(sm);

    vs.addShaderFilePair("voxel");
    vs.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    vs.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    vs.bindAttribute("color", vertexAttrs.index(offsetof(Vertex, color)));
    vs.tryLink();
    ok = ok && voxelShader.replaceWith(vs);

    return ok;
}

void Anim::animate() {
    
}

void Anim::renderScene(float interpolation) {
    UNUSED(interpolation);
    
    GL_CHECK(glClearColor(1.f, 1.f, 1.f, 1.f));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    rm.setCameraMatrix(glt::transformationWorldToLocal(camera));

    rm.beginScene();

    ecLightDir = rm.geometryTransform().transformVector(LIGHT_DIR);

    renderBlocks();

    rm.endScene();

    if (fpsTimer.fire()) {
        glt::FrameStatistics fs = rm.frameStatistics();
        std::cerr << "Timings (FPS/Render Avg/Render Min/Render Max): " << fs.avg_fps << "; " << fs.rt_avg << "; " << fs.rt_min << "; " << fs.rt_max << std::endl;
    }
}

void Anim::renderBlocks() {
    glt::SavePoint sp(rm.geometryTransform().save());

    // float phi = gameTime() * 0.1f;

    // vec3_t center = BLOCK_DIM * N * 0.5f;

    // rm.geometryTransform().translate(center);
    // rm.geometryTransform().concat(mat3(vec3(cos(phi), sin(phi), 0.f),
    //                                    vec3(-sin(phi), cos(phi), 0.f),
    //                                    vec3(0.f, 0.f, 1.f)));
    // rm.geometryTransform().translate(-center);

    voxelShader.use();    
    glt::Uniforms us(voxelShader);
    us.optional("mvpMatrix", rm.geometryTransform().mvpMatrix());
    us.optional("mvMatrix", rm.geometryTransform().mvMatrix());
    us.optional("normalMatrix", rm.geometryTransform().normalMatrix());
    us.optional("ecLightDir", ecLightDir);
    vec4_t mat = vec4(BLOCK_MAT.ambientContribution, BLOCK_MAT.diffuseContribution,
                      BLOCK_MAT.specularContribution, BLOCK_MAT.shininess);
    us.optional("materialProperties", mat);
    us.optional("gammaCorrection", gamma_correction);
    us.optional("sin_time", sin(gameTime()));
    worldModel.draw();
}

void Anim::windowResized(uint32 width, uint32 height) {
    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;
    GL_CHECK(glViewport(0, 0, width, height));

    float fov = degToRad(17.5f);
    float aspect = float(width) / float(height);

    rm.setPerspectiveProjection(fov, aspect, 0.25f, 200.f);
}

void Anim::mouseMoved(int32 dx, int32 dy) {
    float rotX = dx * 0.001f;
    float rotY = dy * 0.001f;
    
    camera.rotateLocal(rotY, vec3(1.f, 0.f, 0.f));
    camera.rotateWorld(rotX, vec3(0.f, 1.f, 0.f));
}

void Anim::handleInternalEvents() {
    vec3_t step = vec3(0.f);

    using namespace sf::Key;

    if (isKeyDown(W))
        step += vec3(0.f, 0.f, 1.f);
    if (isKeyDown(S))
        step += vec3(0.f, 0.f, -1.f);
    if (isKeyDown(D))
        step -= vec3(1.f, 0.f, 0.f);
    if (isKeyDown(A))
        step -= vec3(-1.f, 0.f, 0.f);

    static const float STEP = 0.1f;

    if (lengthSq(step) != 0.f)
        camera.translateLocal(STEP * normalize(step));
}

void Anim::keyStateChanged(const sf::Event::KeyEvent& key, bool pressed) {
    if (!pressed) return;

    using namespace sf::Key;
    bool gamma_changed = false;

    switch (key.Code) {
    case C: loadShaders(); break;
    case B: pause(!paused()); break;
    case O: gamma_correction -= 0.05f; gamma_changed = true; break;
    case P: gamma_correction += 0.05f; gamma_changed = true; break;
    }

    if (gamma_changed)
        std::cerr << "gamma_correction: " << gamma_correction << std::endl;
}

bool Anim::readModel(const std::string& file, CubeMesh& mdl) {
//    return mdl.read(file.c_str());
    return false;
}

bool Anim::writeModel(const std::string& file, const CubeMesh& mdl) {
//    return mdl.write(file.c_str());
    return false;
}

int main(int argc, char *argv[]) {

    bool read_mdl = true;
    bool write_mdl = false;

    for (int32 i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--read") == 0)
            read_mdl = true;
        else if (strcmp(argv[i], "--no-read") == 0)
            read_mdl = false;
        else if (strcmp(argv[i], "--write") == 0)
            write_mdl = true;
        else if (strcmp(argv[i], "--no-write") == 0)
            write_mdl = false;
    }

    Anim *anim = new Anim;
    
    anim->read_model = read_mdl;
    anim->write_model = write_mdl;
    
    if (!anim->init("voxel-world"))
        return 1;
    int ret = anim->run();
    delete anim;
    return ret;
}

static void makeUnitCube(CubeMesh& cube) {
    Vertex v;
    CUBE_MESH(cube);

    v.normal = vec4(0.0f, 0.0f, 1.0f, 0.f);					
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);

    v.normal = vec4( 0.0f, 0.0f, -1.f, 0.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);

    v.normal = vec4( 0.0f, 1.0f, 0.0f, 0.f);					
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);

    v.normal = vec4( 0.0f, -1.f, 0.0f, 0.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);

    v.normal = vec4( 1.0f, 0.0f, 0.0f, 0.f);					
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);

    v.normal = vec4(-1.f, 0.0f, 0.0f, 0.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);

    FREEZE_MESH(cube);
}

namespace {

// Perlin Noise

const uint16 permutation[512] = {
   151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
   140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
   247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
   57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68,
   175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111,
   229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
   102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208,
   89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109,
   198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147,
   118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
   189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70,
   221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108,
   110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251,
   34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235,
   249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204,
   176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114,
   67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
   151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
   140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
   247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
   57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68,
   175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111,
   229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
   102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208,
   89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109,
   198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147,
   118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
   189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70,
   221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108,
   110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251,
   34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235,
   249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204,
   176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114,
   67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

}

static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

static float lerp(float t, float a, float b) { return a + t * (b - a); }

static float grad(int hash, float x, float y, float z) {
    int h = hash & 15;                      
    float u = h<8 ? x : y;                 
    float v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

static float sumNoise3D(const vec3_t& p, uint32 octaves, float fmin, float phi, bool absval) {
    float a = 1.f;
    float r = 0.f;
    float freq = fmin;
    
    for (uint32 i = 0; i < octaves; ++i) {
        float x = noise3D(p * freq);
        if (absval) x = abs(x);
        r += a * x;
        freq *= 2;
        a *= phi;
    }

    return r;
}

static float noise3D(const vec3_t& pnt) {

    const uint16 *p = permutation;

    float x = pnt.x;
    float y = pnt.y;
    float z = pnt.z;
    
    int X = (int) floor(x) & 255,                  
        Y = (int) floor(y) & 255,                  
        Z = (int) floor(z) & 255;
    
    x -= floor(x);                                
    y -= floor(y);                                
    z -= floor(z);
    
    float u = fade(x);                                
    float v = fade(y);                                
    float w = fade(z);
    
    int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      
        B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      

    return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z  ),  
                                   grad(p[BA  ], x-1, y  , z  )), 
                           lerp(u, grad(p[AB  ], x  , y-1, z  ),  
                                   grad(p[BB  ], x-1, y-1, z  ))),
                   lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1),  
                                   grad(p[BA+1], x-1, y  , z-1)), 
                           lerp(u, grad(p[AB+1], x  , y-1, z-1),
                                   grad(p[BB+1], x-1, y-1, z-1))));
}

void pointsOnSphere(uint32 n, vec3_t *ps) {
    float inc = PI * (3.f - sqrt(5.f));
    float off = 2.f / n;
    for (uint32 k = 0; k < n; ++k) {
        float y = k * off - 1 + (off / 2.f);
        float r = sqrt(1.f - y * y);
        float phi = k * inc;
        float s, c;
        sincos(phi, s, c);
        ps[k] = vec3(c * r, y, s * r);
    }
}

