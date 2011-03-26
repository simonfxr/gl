#include <iostream>
#include <cstdio>
#include <cstring>

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

struct Vertex2 {
    vec3_t position;
    vec3_t normal;
    vec2_t texCoord;
};

static const glt::Attr vertex2AttrVals[] = {
    glt::attr::vec3(offsetof(Vertex, position)),
    glt::attr::vec3(offsetof(Vertex, normal)),
    glt::attr::vec2(offsetof(Vertex, texCoord))
};

static const glt::Attrs<Vertex> vertex2Attrs(
    ARRAY_LENGTH(vertex2AttrVals), vertex2AttrVals
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

struct MaterialProperties {
    float ambientContribution;
    float diffuseContribution;
    float specularContribution;
    float shininess;
};

struct Anim EXPLICIT : public ge::GameWindow {

    glt::RenderManager rm;
    glt::ShaderManager sm;
    glt::Frame         camera;
    glt::GenBatch<Vertex> cubeModel;
    glt::ShaderProgram voxelShader;

    float gamma_correction;

    vec3_t light;
    vec3_t ecLight;
    
    Timer fpsTimer;
    uint64 fpsFirstFrame;

    Anim() :
        cubeModel(vertex2Attrs),
        voxelShader(sm)
        {}
    
    bool onInit() OVERRIDE;
    bool loadShaders();
    void animate() OVERRIDE;
    void renderScene(float interpolation) OVERRIDE;
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

    if (!woodTexture.LoadFromFile("data/wood.jpg")) {
        std::cerr << "couldnt load data/wood.jpg" << std::endl;
        return false;
    }

    fpsFirstFrame = 0;
    fpsTimer.start(*this, FPS_UPDATE_INTERVAL, true);

    cubeModel.primType(GL_QUADS);
    makeUnitCube(cubeModel);

    if (!loadShaders())
        return false;

    return true;
}

bool Anim::loadShaders() {
    bool ok = true;

    glt::ShaderProgram vs(sm);
    
    vs.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/voxel.vert");
    vs.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/voxel.frag");
    vs.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    vs.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    vs.bindAttribute("texCoord", vertexAttrs.index(offsetof(Vertex, texCoord)));
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
    GL_CHECK(glClearColor(1.f, 1.f, 1.f, 1.f));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    rm.setCameraMatrix(glt::transformationWorldToLocal(camera));

    rm.beginScene();

    rm.endScene();

    if (fpsTimer.fire()) {
        uint64 id = currentRenderFrameID();
        uint64 frames = id - fpsFirstFrame;
        fpsFirstFrame = id;

        std::cerr << "fps: " << (frames / FPS_UPDATE_INTERVAL) << std::endl;
    }
}

void Anim::windowResized(uint32 width, uint32 height) {
    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;
    GL_CHECK(glViewport(0, 0, width, height));

    float fov = degToRad(17.5f);
    float aspect = float(width) / float(height);

    rm.setPerspectiveProjection(fov, aspect, 0.25f, 100.f);
}

void Anim::mouseMoved(int32 dx, int32 dy) {
    float rotX = dx * 0.001f;
    float rotY = dy * 0.001f;
    
    camera.rotateLocal(rotY, vec3(1.f, 0.f, 0.f));
    camera.rotateWorld(-rotX, vec3(0.f, 1.f, 0.f));
}

void Anim::handleInternalEvents() {
    vec3_t step = vec3(0.f);

    using namespace sf::Key;

    if (isKeyDown(W))
        step += vec3(0.f, 0.f, 1.f);
    if (isKeyDown(S))
        step += vec3(0.f, 0.f, -1.f);
    if (isKeyDown(D))
        step += vec3(1.f, 0.f, 0.f);
    if (isKeyDown(A))
        step += vec3(-1.f, 0.f, 0.f);

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
    Anim anim;
    if (!anim.init("teapot"))
        return 1;
    return anim.run();
}

static void makeUnitCube(glt::GenBatch<Vertex>& cube) {
    Vertex v;
    cube.primType(GL_QUADS);

    v.normal = vec3( 0.0f, 0.0f, 1.0f);					
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);

    v.normal = vec3( 0.0f, 0.0f,0.f);					
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);

    v.normal = vec3( 0.0f, 1.0f, 0.0f);					
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);

    v.normal = vec3( 0.0f,0.f, 0.0f);					
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);

    v.normal = vec3( 1.0f, 0.0f, 0.0f);					
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);

    v.normal = vec3(0.f, 0.0f, 0.0f);					
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);

    cube.freeze();
}
