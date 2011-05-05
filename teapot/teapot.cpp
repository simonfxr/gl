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

#include "parse_sply.hpp"

using namespace math;

static const point3_t LIGHT_CENTER_OF_ROTATION = vec3(0.f, 15.f, 0.f);
static const float  LIGHT_ROTATION_RAD = 15.f;

static const glt::Attr vertexAttrVals[] = {
    glt::attr::vec3(offsetof(Vertex, position)),
    glt::attr::vec3(offsetof(Vertex, normal))
};

static const glt::Attrs<Vertex> vertexAttrs(
    ARRAY_LENGTH(vertexAttrVals), vertexAttrVals
);

struct Vertex2 {
    vec3_t position;
    vec3_t normal;
    vec2_t texCoord;
};

static const glt::Attr vertex2AttrVals[] = {
    glt::attr::vec3(offsetof(Vertex2, position)),
    glt::attr::vec3(offsetof(Vertex2, normal)),
    glt::attr::vec2(offsetof(Vertex2, texCoord))
};

static const glt::Attrs<Vertex2> vertex2Attrs(
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

static const float FPS_UPDATE_INTERVAL = 3.f;

static const uint32 SHADE_MODE_AMBIENT = 1;
static const uint32 SHADE_MODE_DIFFUSE = 2;
static const uint32 SHADE_MODE_SPECULAR = 4;
static const uint32 SHADE_MODE_DEFAULT =
    SHADE_MODE_AMBIENT | SHADE_MODE_DIFFUSE | SHADE_MODE_SPECULAR;

struct MaterialProperties {
    float ambientContribution;
    float diffuseContribution;
    float specularContribution;
    float shininess;
};

struct Teapot {
    glt::Frame frame;
    MaterialProperties material;
    glt::color color;
};

struct Anim EXPLICIT : public ge::GameWindow {

    glt::RenderManager rm;
    glt::ShaderManager sm;
    glt::Frame         camera;
    glt::GenBatch<Vertex> groundModel;
    glt::GenBatch<Vertex> teapotModel;
    glt::GenBatch<Vertex> sphereModel;
    glt::GenBatch<Vertex2> cubeModel;
    glt::ShaderProgram teapotShader;
    glt::ShaderProgram teapotTexturedShader;
    sf::Image woodTexture;

    Teapot teapot1;
    Teapot teapot2;

    bool wireframe_mode;

    float light_angular_position;
    float light_rotation_speed;

    float gamma_correction;

    vec3_t light;
    vec3_t ecLight;
    
    Timer fpsTimer;
    uint64 fpsFirstFrame;

    uint32 shade_mode;

    Anim() :
        groundModel(vertexAttrs),
        teapotModel(vertexAttrs),
        sphereModel(vertexAttrs),
        cubeModel(vertex2Attrs),
        teapotShader(sm),
        teapotTexturedShader(sm)
        {}
    
    bool onInit() OVERRIDE;
    bool loadShaders();
    void animate() OVERRIDE;
    void renderScene(float interpolation) OVERRIDE;
    vec3_t lightPosition(float interpolation);
    void setupTeapotShader(glt::ShaderProgram& prog, const vec4_t& surfaceColor, const MaterialProperties& mat);
    void renderTeapot(const Teapot& teapot);
    void renderGround();
    void renderLight();
    void renderTable(glt::ShaderProgram& shader);
    void windowResized(uint32 width, uint32 height) OVERRIDE;
    void mouseMoved(int32 dx, int32 dy) OVERRIDE;
    void keyStateChanged(const sf::Event::KeyEvent& key, bool pressed) OVERRIDE;
    void handleInternalEvents() OVERRIDE;
};

static void makeUnitCube(glt::GenBatch<Vertex2>& cube);
static void makeSphere(glt::GenBatch<Vertex>& sphere, float rad, int32 stacks, int32 slices);

std::ostream& operator <<(std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

bool Anim::onInit() {
    sm.verbosity(glt::ShaderManager::Info);

    if (GLEW_ARB_multisample) {
        std::cerr << "multisampling support available" << std::endl;

        GL_CHECK(glEnable(GL_MULTISAMPLE_ARB));
    }

    rm.setDefaultRenderTarget(&this->renderTarget());

    sm.addPath(".");
    sm.addPath("shaders");
    sm.addPath("../shaders");

    ticksPerSecond(100);
    synchronizeDrawing(true);
    // maxDrawFramesSkipped(1);
    // maxFPS(120);

    wireframe_mode = true;
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    shade_mode = 0;

    light_rotation_speed = 0.0025f;
    light_angular_position = 0.f;

    gamma_correction = 1.35f;
    
    grabMouse(true);

    if (!woodTexture.LoadFromFile("data/wood.jpg")) {
        std::cerr << "couldnt load data/wood.jpg" << std::endl;
        return false;
    }

    fpsFirstFrame = 0;
    fpsTimer.start(*this, FPS_UPDATE_INTERVAL, true);

    woodTexture.SetSmooth(true);

    int32 nfaces = parse_sply("data/teapot.sply", teapotModel);
    if (nfaces < 0) {
        std::cerr << "couldnt parse teapot model" << std::endl;
        return false;
    } else {
        std::cerr << "parsed teapot model: " << nfaces << " vertices" << std::endl;
    }

    teapotModel.primType(GL_QUADS);
    teapotModel.freeze();

    groundModel.primType(GL_QUADS);

    makeUnitCube(cubeModel);

    makeSphere(sphereModel, 1.f, 26, 13);

    Vertex v;
    v.normal = vec3(0.f, 1.f, 0.f);
    v.position = vec3(0.f, 0.f, 0.f); groundModel.add(v);
    v.position = vec3(1.f, 0.f, 0.f); groundModel.add(v);
    v.position = vec3(1.f, 0.f, 1.f); groundModel.add(v);
    v.position = vec3(0.f, 0.f, 1.f); groundModel.add(v);
    groundModel.freeze();

    if (!loadShaders())
        return false;

    camera.setOrigin(vec3(6.36, 5.87, 1.97));
    camera.setXZ(normalize(vec3(-0.29, 0.f, 0.95f)),
                 normalize(vec3(-0.8f, -0.54f, -0.25f)));

    
    teapot1.frame.setXZ(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    teapot1.frame.origin = vec3(5.f, 4.f, 4.f);

    teapot1.material.ambientContribution = 0.4f;
    teapot1.material.diffuseContribution = 0.6f;
    teapot1.material.specularContribution = 0.2f;
    teapot1.material.shininess = 110;
    teapot1.color = glt::color(0xFF, 0xFF, 0xFF);

    teapot2.frame.setXZ(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    teapot2.frame.origin = vec3(3.f, 4.f, 12.f);
    teapot2.frame.rotateLocal(degToRad(110.f), vec3(0.f, 0.f, 1.f));
    teapot2.material.ambientContribution = 0.4f;
    teapot2.material.diffuseContribution = 0.6f;
    teapot2.material.specularContribution = 0.15f;
    teapot2.material.shininess = 35;
    teapot2.color = glt::color(0xFF, 0x8C, 0x00);

    return true;
}

bool Anim::loadShaders() {
    bool ok = true;

    glt::ShaderProgram ts(sm);
    ts.addShaderFilePair("teapot");
    ts.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    ts.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    ts.tryLink();
    ok = ok && teapotShader.replaceWith(ts);

    glt::ShaderProgram tt(sm);
    tt.addShaderFilePair("teapot_textured");
    tt.bindAttribute("position", vertex2Attrs.index(offsetof(Vertex2, position)));
    tt.bindAttribute("normal", vertex2Attrs.index(offsetof(Vertex2, normal)));
    tt.bindAttribute("texCoord", vertex2Attrs.index(offsetof(Vertex2, texCoord)));
    tt.tryLink();
    ok = ok && teapotTexturedShader.replaceWith(tt);

    return ok;
}

void Anim::animate() {
    light_angular_position = wrapPi(light_angular_position + light_rotation_speed);
}

vec3_t Anim::lightPosition(float interpolation) {
    float theta = wrapPi(light_angular_position + interpolation * light_rotation_speed);
    vec3_t d = vec3(cos(theta), 0.f, sin(theta));
    return d * LIGHT_ROTATION_RAD + LIGHT_CENTER_OF_ROTATION;
}

void Anim::renderScene(float interpolation) {
    GL_CHECK(glClearColor(1.f, 1.f, 1.f, 1.f));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    rm.setCameraMatrix(glt::transformationWorldToLocal(camera));

    light = lightPosition(interpolation);
    
    rm.beginScene();

    ecLight = rm.geometryTransform().transformPoint(light);        

    renderLight();

    renderTable(teapotTexturedShader);
    
    // { // render a shadow of the table
    //     glt::GeometryTransform& gt = rm.geometryTransform();
    //     glt::SavePoint sp(rm.geometryTransform().save());

    //     const point3_t& s = light;
        
    //     mat4_t shadowProjection = mat4(vec4(-s.y, 0.f,  0.f,  0.f),
    //                                vec4( s.x, 0.f,  s.z,  1.f),
    //                                vec4( 0.f, 0.f, -s.y,  0.f),
    //                                vec4( 0.f, 0.f,  0.f, -s.y));

    //     gt.concat(shadowProjection);

    //     renderTable(teapotShader);
    // }
    
//    renderGround();
    renderTeapot(teapot1);
    renderTeapot(teapot2);
    
    rm.endScene();

    if (fpsTimer.fire()) {
        uint64 id = currentRenderFrameID();
        uint64 frames = id - fpsFirstFrame;
        fpsFirstFrame = id;

        std::cerr << "fps: " << (frames / FPS_UPDATE_INTERVAL) << std::endl;
    }
}

void Anim::setupTeapotShader(glt::ShaderProgram& prog, const vec4_t& surfaceColor, const MaterialProperties& mat) {
    prog.use();

    vec4_t materialSelector = vec4((shade_mode & SHADE_MODE_AMBIENT) != 0,
                                   (shade_mode & SHADE_MODE_DIFFUSE) != 0,
                                   (shade_mode & SHADE_MODE_SPECULAR) != 0,
                                   1.f);

    float scale = 1 / (2.f * math::PI);
    float specular_factor = mat.specularContribution * (mat.shininess + 2) * scale;
    vec4_t vm = vec4(mat.ambientContribution, mat.diffuseContribution,
                     specular_factor, mat.shininess);
    
    glt::Uniforms us(prog);
    us.optional("projectionMatrix", rm.geometryTransform().projectionMatrix());
    us.optional("mvMatrix", rm.geometryTransform().mvMatrix());
    us.optional("normalMatrix", rm.geometryTransform().normalMatrix());
    us.optional("surfaceColor", surfaceColor);
    us.optional("materialProperties", vm * materialSelector);
    us.optional("ecLight", ecLight);
    us.optional("gammaCorrection", gamma_correction);
}

void Anim::renderLight() {
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().translate(light);
    rm.geometryTransform().scale(vec3(0.66f));

    MaterialProperties mat = { 0.8f, 0.2f, 1.f, 120.f };

    setupTeapotShader(teapotShader, vec4(1.f, 1.f, 0.f, 1.f), mat);
    sphereModel.draw();
}

void Anim::renderTeapot(const Teapot& teapot) {
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().concat(transformationLocalToWorld(teapot.frame));
    rm.geometryTransform().scale(vec3(13.f));

    setupTeapotShader(teapotShader, teapot.color.vec4(), teapot.material);
    teapotModel.draw();
}

void Anim::renderGround() {
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().scale(vec3(50.f));
    rm.geometryTransform().translate(vec3(-0.5f, 0.f, -0.5f));

    MaterialProperties mat = { 0.f, 1.f, 0.f, 30.f };
    vec4_t color = glt::color(0xcd, 0xc9, 0xc9).vec4();
    setupTeapotShader(teapotShader, color, mat);
    groundModel.draw();
}

void Anim::renderTable(glt::ShaderProgram& shader) {
    glt::GeometryTransform& gt = rm.geometryTransform();
    glt::SavePoint sp(gt.save());

    gt.scale(vec3(10.f, 4.f, 16.f));

    gt.dup();

    woodTexture.Bind();

    vec4_t color = glt::color(0xcd, 0x85, 0x3f).vec4();
    MaterialProperties mat;
    mat.ambientContribution = 0.55;
    mat.diffuseContribution = 0.6;
    mat.specularContribution = 0.075f;
    mat.shininess = 30;

    float table_height = 1.f;
    float table_thickness = 1 / 33.f;
    float foot_height = table_height - table_thickness;
    float foot_width = 0.03f;
    float foot_depth = 10.f/16.f * foot_width;
    float foot_x_dist = 1.f - foot_width;
    float foot_z_dist = 1.f - foot_depth;

    vec3_t foot_dim = vec3(foot_width, foot_height, foot_depth);

    gt.translate(vec3(0.f, foot_height, 0.f));
    gt.scale(vec3(1.f, table_thickness, 1.f));
    setupTeapotShader(shader, color, mat);
    cubeModel.draw();

    for (uint32 x = 0; x < 2; ++x) {
        for (uint32 z = 0; z < 2; ++z) {
            gt.pop(); gt.dup();
            gt.translate(vec3(x * foot_x_dist, 0.f, z * foot_z_dist));
            gt.scale(foot_dim);
            setupTeapotShader(shader, color, mat);
            cubeModel.draw();
        }
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

    dx = - dx;

    if (isKeyDown(sf::Key::M)) {

        teapot1.frame.rotateWorld(-dx * 0.001f, camera.localY());
        teapot1.frame.rotateWorld(dy * 0.001f, camera.localX());

    } else {
        float rotX = dx * 0.001f;
        float rotY = dy * 0.001f;

        camera.rotateLocal(rotY, vec3(1.f, 0.f, 0.f));
        camera.rotateWorld(-rotX, vec3(0.f, 1.f, 0.f));
    }
}

void Anim::handleInternalEvents() {
    vec3_t step = vec3(0.f);

    using namespace sf::Key;

    if (isKeyDown(W))
        step += vec3(0.f, 0.f, 1.f);
    if (isKeyDown(S))
        step += vec3(0.f, 0.f, -1.f);
    if (isKeyDown(A))
        step += vec3(1.f, 0.f, 0.f);
    if (isKeyDown(D))
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
    case F:
        wireframe_mode = !wireframe_mode;
        GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL));
        break;
    case H:
        
        if (shade_mode == SHADE_MODE_DEFAULT)
            shade_mode = 0;
        else
            shade_mode = SHADE_MODE_DEFAULT;
        break;
    case J: shade_mode ^= SHADE_MODE_AMBIENT; break;
    case K: shade_mode ^= SHADE_MODE_DIFFUSE; break;
    case L: shade_mode ^= SHADE_MODE_SPECULAR; break;
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

static void makeUnitCube(glt::GenBatch<Vertex2>& cube) {
    Vertex2 v;
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


static void addTriangle(glt::GenBatch<Vertex>& s, const vec3_t vertices[3], const vec3_t normals[3], const vec2_t texCoords[3]) {
    UNUSED(texCoords);
    
    for (uint32 i = 0; i < 3; ++i) {
        Vertex v;
        v.position = vertices[i];
        v.normal = normals[i];
        s.add(v);
    }
}

// from the GLTools library (OpenGL Superbible)
static void gltMakeSphere(glt::GenBatch<Vertex>& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks)
{
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;
    GLfloat ds = 1.0f / (GLfloat) iSlices;
    GLfloat dt = 1.0f / (GLfloat) iStacks;
    GLfloat t = 1.0f;	
    GLfloat s = 0.0f;
    GLint i, j;     // Looping variables
    
    for (i = 0; i < iStacks; i++) 
    {
        GLfloat rho = (GLfloat)i * drho;
        GLfloat srho = (GLfloat)(sin(rho));
        GLfloat crho = (GLfloat)(cos(rho));
        GLfloat srhodrho = (GLfloat)(sin(rho + drho));
        GLfloat crhodrho = (GLfloat)(cos(rho + drho));
		
        // Many sources of OpenGL sphere drawing code uses a triangle fan
        // for the caps of the sphere. This however introduces texturing 
        // artifacts at the poles on some OpenGL implementations
        s = 0.0f;
        vec3_t vVertex[4];
        vec3_t vNormal[4];
        vec2_t vTexture[4];

        for ( j = 0; j < iSlices; j++) 
        {
            GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
            GLfloat stheta = (GLfloat)(-sin(theta));
            GLfloat ctheta = (GLfloat)(cos(theta));
			
            GLfloat x = stheta * srho;
            GLfloat y = ctheta * srho;
            GLfloat z = crho;
        
            vTexture[0][0] = s;
            vTexture[0][1] = t;
            vNormal[0][0] = x;
            vNormal[0][1] = y;
            vNormal[0][2] = z;
            vVertex[0][0] = x * fRadius;
            vVertex[0][1] = y * fRadius;
            vVertex[0][2] = z * fRadius;
			
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            vTexture[1][0] = s;
            vTexture[1][1] = t - dt;
            vNormal[1][0] = x;
            vNormal[1][1] = y;
            vNormal[1][2] = z;
            vVertex[1][0] = x * fRadius;
            vVertex[1][1] = y * fRadius;
            vVertex[1][2] = z * fRadius;
			

            theta = ((j+1) == iSlices) ? 0.0f : (j+1) * dtheta;
            stheta = (GLfloat)(-sin(theta));
            ctheta = (GLfloat)(cos(theta));
			
            x = stheta * srho;
            y = ctheta * srho;
            z = crho;
        
            s += ds;
            vTexture[2][0] = s;
            vTexture[2][1] = t;
            vNormal[2][0] = x;
            vNormal[2][1] = y;
            vNormal[2][2] = z;
            vVertex[2][0] = x * fRadius;
            vVertex[2][1] = y * fRadius;
            vVertex[2][2] = z * fRadius;
			
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            vTexture[3][0] = s;
            vTexture[3][1] = t - dt;
            vNormal[3][0] = x;
            vNormal[3][1] = y;
            vNormal[3][2] = z;
            vVertex[3][0] = x * fRadius;
            vVertex[3][1] = y * fRadius;
            vVertex[3][2] = z * fRadius;

            addTriangle(sphereBatch, vVertex, vNormal, vTexture);
			
            // Rearrange for next triangle
            memcpy(&vVertex[0], &vVertex[1], sizeof(vec3_t));
            memcpy(&vNormal[0], &vNormal[1], sizeof(vec3_t));
            memcpy(&vTexture[0], &vTexture[1], sizeof(vec2_t));
			
            memcpy(&vVertex[1], &vVertex[3], sizeof(vec3_t));
            memcpy(&vNormal[1], &vNormal[3], sizeof(vec3_t));
            memcpy(&vTexture[1], &vTexture[3], sizeof(vec2_t));
					
            addTriangle(sphereBatch, vVertex, vNormal, vTexture);		       
        }
        t -= dt;
    }
    
    sphereBatch.freeze();
}

static void makeSphere(glt::GenBatch<Vertex>& sphere, float rad, int32 stacks, int32 slices) {
    sphere.primType(GL_TRIANGLES);
    gltMakeSphere(sphere, rad, stacks, slices);
}
