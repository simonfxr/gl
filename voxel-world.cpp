#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <limits.h>

#include <GL/glew.h>

#include <SFML/Graphics/Image.hpp>

#include "defs.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/math.hpp"

#include "ge/GameWindow.hpp"

#include "glt/utils.hpp"
#include "glt/RenderManager.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"
#include "glt/Frame.hpp"
#include "glt/GenBatch.hpp"
#include "glt/color.hpp"
#include "glt/Transformations.hpp"

using namespace math;

static const float FPS_UPDATE_INTERVAL = 3.f;
static const vec3_t BLOCK_DIM = vec3(0.25f);

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

static const MaterialProperties BLOCK_MAT(0.4f, 0.6f, 0.f, 0.f);

static const uint32 N = 64;

static float noise3D(const vec3_t& p);

struct Vertex {
    vec3_t position;
    vec3_t normal;
    uint32 color;
};

static const glt::Attr vertexAttrVals[] = {
    glt::attr::vec3(offsetof(Vertex, position)),
    glt::attr::vec3(offsetof(Vertex, normal)),
    glt::attr::color(offsetof(Vertex, color))
};

static const glt::Attrs<Vertex> vertexAttrs(
    ARRAY_LENGTH(vertexAttrVals), vertexAttrVals
);

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
    glt::GenBatch<Vertex> cubeModel;
    glt::ShaderProgram voxelShader;
    glt::GenBatch<Vertex> worldModel;

    float gamma_correction;

    vec3_t light;
    vec3_t ecLight;
    
    Timer fpsTimer;
    uint64 fpsFirstFrame;

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
};

static void makeUnitCube(glt::GenBatch<Vertex>& cube);

std::ostream& operator <<(std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

bool Anim::onInit() {
    sm.verbosity(glt::ShaderManager::Info);

    if (GLEW_ARB_multisample) {
        std::cerr << "multisampling support available" << std::endl;

        GL_CHECK(glEnable(GL_MULTISAMPLE_ARB));
    }

    sm.addPath(".");
    sm.addPath("shaders");

    ticksPerSecond(100);
    maxDrawFramesSkipped(1);
    maxFPS(0);

    gamma_correction = 1.35f;
    
    grabMouse(true);

    fpsFirstFrame = 0;
    fpsTimer.start(*this, FPS_UPDATE_INTERVAL, true);

    cubeModel.primType(GL_QUADS);
    cubeModel.deleteAfterFreeze(false);
    makeUnitCube(cubeModel);

    if (!loadShaders())
        return false;

    srand(0xDEADBEEF);

    worldModel.primType(GL_QUADS);
    initWorld();

    return true;
}

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

static vec4_t randColor() {
    return vec4(vec3(rand1(), rand1(), rand1()), 1.f);
}


struct World {
    bool shell[N][N][N];
};

void testray(const World& w, World& vis, int32 i, int32 j, int32 k, int32 di, int32 dj, int32 dk) {
    for (;;) {
        
        if (i < 0 || i >= N) break;
        if (j < 0 || j >= N) break;
        if (k < 0 || k >= N) break;

        vis.shell[i][j][k] = true;

        if (w.shell[i][j][k]) break;

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
           (!w.shell[i][j][k] && vis.shell[i][j][k]);
}

World *filterOccluded(const World& w) {

    World *ret = new World;
    World& vis = *ret;

    memset(&vis, 0, sizeof vis);
    
    bool fixpoint = false;
    while (!fixpoint) {
        fixpoint = true;
        for (uint32 i = 0; i < N; ++i) {
            for (uint32 j = 0; j < N; ++j) {
                for (uint32 k = 0; k < N; ++k) {
                    if (!vis.shell[i][j][k]) {
                        if (visible(w, vis, i + 1, j, k) ||
                            visible(w, vis, i - 1, j, k) ||
                            visible(w, vis, i, j + 1, k) ||
                            visible(w, vis, i, j - 1, k) ||
                            visible(w, vis, i, j, k + 1) ||
                            visible(w, vis, i, j, k - 1)) {
                            fixpoint = false;
                            vis.shell[i][j][k] = true;
                            if (!w.shell[i][j][k])
                                sendRays(w, vis, i, j, k);
                        }
                    }
                }
            }
        }
    }

    return ret;                
}

bool visibleFace(const World& w, const World& vis, int32 i, int32 j, int32 k, const vec3_t& normal) {
    i += (int32) normal.x;
    j += (int32) normal.y;
    k += (int32) normal.z;
    return visible(w, vis, i, j, k);
}

void Anim::initWorld() {

    World *world = new World;

    for (uint32 i = 0; i < N; ++i)
        for (uint32 j = 0; j < N; ++j)
            for (uint32 k = 0; k < N; ++k)
                world->shell[i][j][k] = noise3D(vec3(i, j, k) * 0.035f) < 0.f;

    World *vis = filterOccluded(*world);

    uint32 permut[N];
    for (uint32 i = 0; i < N; ++i)
        permut[i] = i;

    for (uint32 i = 0; i < N - 1; ++i) {
        uint32 j = rand() % (N - i);
        uint32 t = permut[i];
        permut[i] = permut[i + j];
        permut[i + j] = t;
    }

    uint32 blocks = 0;
    uint32 visBlocks = 0;
    uint32 faces = 0;
    uint32 visFaces = 0;

    for (uint32 i = 0; i < N; ++i) {
        for (uint32 j = 0; j < N; ++j) {
            for (uint32 k = 0; k < N; ++k) {
                uint32 ii = permut[i];
                uint32 jj = permut[permut[j]];
                uint32 kk = permut[permut[permut[k]]];

                if (world->shell[ii][jj][kk])
                    ++blocks;
                
                if (vis->shell[ii][jj][kk] && world->shell[ii][jj][kk]) {
                    ++visBlocks;
                    uint32 col = glt::color(vec4(0.6, 0.6, 0.6, 1.f)).rgba();
                    vec3_t offset = BLOCK_DIM * vec3(ii, jj, kk);
                    for (uint32 r = 0; r < cubeModel.size(); ++r) {
                        Vertex v = cubeModel.at(r);
                        ++faces;
                        if (visibleFace(*world, *vis, ii, jj, kk, v.normal)) {
                            ++visFaces;
                            v.position *= BLOCK_DIM;
                            v.position += offset;
                            v.color = col;
                            worldModel.add(v);
                        }
                    }
                }
            }
        }
    }

    delete vis;
    delete world;

    std::cerr << "blocks: " << blocks << ", visible blocks: " << visBlocks << std::endl;
    std::cerr << "faces: " << faces << ", visible faces: " << visFaces << std::endl;

    worldModel.freeze();
}

bool Anim::loadShaders() {
    bool ok = true;

    glt::ShaderProgram vs(sm);
    
    vs.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/voxel.vert");
    vs.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/voxel.frag");
    vs.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    vs.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    vs.bindAttribute("color", vertexAttrs.index(offsetof(Vertex, color)));
    vs.tryLink();

    ok = ok && !vs.wasError();

    if (!vs.wasError())
        voxelShader.replaceWith(vs);
    else
        vs.printError(std::cerr);

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

    light = camera.origin;

    ecLight = rm.geometryTransform().transformPoint(light);

    renderBlocks();

    // Voxel v;
    // v.color = vec4(1.f, 0.f, 0.f, 1.f);
    // v.mat.diffuseContribution = 0.f;
    // v.mat.ambientContribution = 1.f;
    // v.mat.specularContribution = 0.f;
    // v.mat.shininess = 0.f;
    // renderBlock(v);

    rm.endScene();

    if (fpsTimer.fire()) {
        uint64 id = currentRenderFrameID();
        uint64 frames = id - fpsFirstFrame;
        fpsFirstFrame = id;

        std::cerr << "fps: " << (frames / FPS_UPDATE_INTERVAL) << std::endl;
    }
}

void Anim::renderBlocks() {
    glt::SavePoint sp(rm.geometryTransform().save());

    voxelShader.use();    
    glt::Uniforms us(voxelShader);
    us.optional("mvpMatrix", rm.geometryTransform().mvpMatrix());
    us.optional("mvMatrix", rm.geometryTransform().mvMatrix());
    us.optional("normalMatrix", rm.geometryTransform().normalMatrix());
    us.optional("ecLight", ecLight);
    vec4_t mat = vec4(BLOCK_MAT.ambientContribution, BLOCK_MAT.diffuseContribution,
                      BLOCK_MAT.specularContribution, BLOCK_MAT.shininess);
    us.optional("materialProperties", mat);
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

    switch (key.Code) {
    case C: loadShaders(); break;
    case B: pause(!paused()); break;
    case O: gamma_correction += 0.05f; break;
    case P: gamma_correction -= 0.05f; break;
    }
}

int main(void) {
    Anim *anim = new Anim;
    if (!anim->init("voxel-world"))
        return 1;
    int ret = anim->run();
    delete anim;
    return ret;
}

static void makeUnitCube(glt::GenBatch<Vertex>& cube) {
    Vertex v;
    cube.primType(GL_QUADS);

    v.normal = vec3( 0.0f, 0.0f, 1.0f);					
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);

    v.normal = vec3( 0.0f, 0.0f, -1.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);

    v.normal = vec3( 0.0f, 1.0f, 0.0f);					
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);

    v.normal = vec3( 0.0f, -1.f, 0.0f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);

    v.normal = vec3( 1.0f, 0.0f, 0.0f);					
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);

    v.normal = vec3(-1.f, 0.0f, 0.0f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);

    cube.freeze();
}

namespace {

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
    int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    float u = h<8 ? x : y;                 // INTO 12 GRADIENT DIRECTIONS.
    float v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

static float noise3D(const vec3_t& pnt) {

    const uint16 *p = permutation;

    float x = pnt.x;
    float y = pnt.y;
    float z = pnt.z;
    
    int X = (int) floor(x) & 255,                  // FIND UNIT CUBE THAT
        Y = (int) floor(y) & 255,                  // CONTAINS POINT.
        Z = (int) floor(z) & 255;
    
    x -= floor(x);                                // FIND RELATIVE X,Y,Z
    y -= floor(y);                                // OF POINT IN CUBE.
    z -= floor(z);
    
    float u = fade(x);                                // COMPUTE FADE CURVES
    float v = fade(y);                                // FOR EACH OF X,Y,Z.
    float w = fade(z);
    
    int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
        B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

    return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z  ),  // AND ADD
                                   grad(p[BA  ], x-1, y  , z  )), // BLENDED
                           lerp(u, grad(p[AB  ], x  , y-1, z  ),  // RESULTS
                                   grad(p[BB  ], x-1, y-1, z  ))),// FROM  8
                   lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1),  // CORNERS
                                   grad(p[BA+1], x-1, y  , z-1)), // OF CUBE
                           lerp(u, grad(p[AB+1], x  , y-1, z-1),
                                   grad(p[BB+1], x-1, y-1, z-1))));
}

