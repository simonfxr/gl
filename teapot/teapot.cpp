#include <iostream>
#include <cstdio>
#include <cstring>

#include <GL/glew.h>

#include <SFML/Graphics/Image.hpp>

#include "defs.h"
#include "mesh.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/math.hpp"

#include "glt/utils.hpp"
#include "glt/RenderManager.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"
#include "glt/Frame.hpp"
#include "glt/color.hpp"
#include "glt/Transformations.hpp"

#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

#include "parse_sply.hpp"

using namespace math;
using namespace ge;

static const point3_t LIGHT_CENTER_OF_ROTATION = vec3(0.f, 15.f, 0.f);
static const float  LIGHT_ROTATION_RAD = 15.f;

struct Vertex2 {
    vec3_t position;
    vec3_t normal;
    vec2_t texCoord;
};

#ifdef MESH_GENBATCH

static const glt::Attr vertexAttrVals[] = {
    glt::attr::vec3(offsetof(Vertex, position)),
    glt::attr::vec3(offsetof(Vertex, normal))
};

static const glt::Attrs<Vertex> vertexAttrs(
    ARRAY_LENGTH(vertexAttrVals), vertexAttrVals
);

static const glt::Attr vertex2AttrVals[] = {
    glt::attr::vec3(offsetof(Vertex2, position)),
    glt::attr::vec3(offsetof(Vertex2, normal)),
    glt::attr::vec2(offsetof(Vertex2, texCoord))
};

static const glt::Attrs<Vertex2> vertex2Attrs(
    ARRAY_LENGTH(vertex2AttrVals), vertex2AttrVals
);

#elif defined(MESH_MESH)

DEFINE_VERTEX_ATTRS(vertexAttrs, Vertex,
                    VERTEX_ATTR(Vertex, position),
                    VERTEX_ATTR(Vertex, normal));

DEFINE_VERTEX_ATTRS(vertex2Attrs, Vertex2,
                    VERTEX_ATTR(Vertex2, position),
                    VERTEX_ATTR(Vertex2, normal),
                    VERTEX_ATTR(Vertex2, texCoord));

#else
#error "no meshtype defined"
#endif

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

struct Anim {
    ge::Engine& engine;
    ge::Camera camera;
    CubeMesh groundModel;
    CubeMesh teapotModel;
    Mesh sphereModel;
    CubeMesh2 cubeModel;

    sf::Image woodTexture;

    Teapot teapot1;
    Teapot teapot2;

    bool wireframe_mode;

    float light_angular_position;
    float light_rotation_speed;

    float gamma_correction;

    vec3_t light;
    vec3_t ecLight;
    
    uint32 shade_mode;

    bool use_spotlight;
    bool spotlight_smooth;
    direction3_t ec_spotlight_dir;

    Anim(ge::Engine& e) :
        engine(e),
        groundModel(vertexAttrs),
        teapotModel(vertexAttrs),
        sphereModel(vertexAttrs),
        cubeModel(vertex2Attrs)
        {}

    void linkEvents(const Event<InitEvent>& e);
    void init(const Event<InitEvent>&);
    void animate(const Event<EngineEvent>&);
    void renderScene(const Event<RenderEvent>&);
    vec3_t lightPosition(float interpolation);
    void setupTeapotShader(const std::string& prog, const vec4_t& surfaceColor, const MaterialProperties& mat);
    void renderTeapot(const Teapot& teapot);
    void renderGround();
    void renderLight();
    void renderTable(const std::string& shader);
    void windowResized(const Event<WindowResized>&);
    void mouseMoved(const Event<MouseMoved>&);
    void keyStateChanged(const Event<KeyChanged>&);
    void handleInput(const Event<EngineEvent>&);
};


static void makeUnitCube(CubeMesh2& cube);
static void makeSphere(Mesh& sphere, float rad, int32 stacks, int32 slices);

std::ostream& operator <<(std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

void Anim::linkEvents(const Event<InitEvent>& e) {
    engine.events().handleInput.reg(makeEventHandler(this, &Anim::handleInput));
    engine.events().animate.reg(makeEventHandler(this, &Anim::animate));
    engine.events().render.reg(makeEventHandler(this, &Anim::renderScene));
    GameWindow& win = engine.window();
    win.events().windowResized.reg(makeEventHandler(this, &Anim::windowResized));
    win.events().mouseMoved.reg(makeEventHandler(this, &Anim::mouseMoved));
    win.events().keyChanged.reg(makeEventHandler(this, &Anim::keyStateChanged));
    e.info.success = true;
}


void Anim::init(const Event<InitEvent>& e) {
    engine.renderManager().setDefaultRenderTarget(&engine.window().renderTarget());

    engine.window().showMouseCursor(false);
    engine.window().grabMouse(true);
    
    engine.shaderManager().addPath("shaders");
    engine.shaderManager().addPath("../shaders");

    engine.gameLoop().ticksPerSecond(100);
    engine.gameLoop().sync(true);

    use_spotlight = false;
    spotlight_smooth = false;

    wireframe_mode = true;
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    shade_mode = 0;

    light_rotation_speed = 0.0025f;
    light_angular_position = 0.f;

    gamma_correction = 1.35f;
    
    if (!woodTexture.LoadFromFile("data/wood.jpg")) {
        std::cerr << "couldnt load data/wood.jpg" << std::endl;
        return;
    }

    woodTexture.SetSmooth(true);

    int32 nfaces = parse_sply("data/teapot.sply", teapotModel);
    if (nfaces < 0) {
        std::cerr << "couldnt parse teapot model" << std::endl;
        return;
    } else {
        std::cerr << "parsed teapot model: " << nfaces << " vertices" << std::endl;
    }

    QUAD_MESH(teapotModel);
    FREEZE_MESH(teapotModel);

    QUAD_MESH(groundModel);
    makeUnitCube(cubeModel);

    makeSphere(sphereModel, 1.f, 26, 13);

    {
        Vertex v;
        v.normal = vec3(0.f, 1.f, 0.f);
        v.position = vec3(0.f, 0.f, 0.f); ADD_VERTEX(groundModel, v);
        v.position = vec3(1.f, 0.f, 0.f); ADD_VERTEX(groundModel, v);
        v.position = vec3(1.f, 0.f, 1.f); ADD_VERTEX(groundModel, v);
        v.position = vec3(0.f, 0.f, 1.f); ADD_VERTEX(groundModel, v);

        FREEZE_MESH(groundModel);
    }

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

    Ref<glt::ShaderProgram> teapot = engine.shaderManager().defineProgram("teapot");
    teapot->addShaderFilePair("teapot");
    teapot->bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    teapot->bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    if (!teapot->tryLink())
        return;

    Ref<glt::ShaderProgram> teapotTex = engine.shaderManager().defineProgram("teapotTextured");
    teapotTex->addShaderFilePair("teapot_textured");
    teapotTex->bindAttribute("position", vertex2Attrs.index(offsetof(Vertex2, position)));
    teapotTex->bindAttribute("normal", vertex2Attrs.index(offsetof(Vertex2, normal)));
    teapotTex->bindAttribute("texCoord", vertex2Attrs.index(offsetof(Vertex2, texCoord)));
    if (!teapotTex->tryLink())
        return;

    e.info.success = true;
}

void Anim::animate(const Event<EngineEvent>&) {
    light_angular_position = wrapPi(light_angular_position + light_rotation_speed);
}

vec3_t Anim::lightPosition(float interpolation) {
    float theta = wrapPi(light_angular_position + interpolation * light_rotation_speed);
    vec3_t d = vec3(cos(theta), 0.f, sin(theta));
    return d * LIGHT_ROTATION_RAD + LIGHT_CENTER_OF_ROTATION;
}

void Anim::renderScene(const Event<RenderEvent>& e) {
    float interpolation = e.info.interpolation;
    
    GL_CHECK(glClearColor(1.f, 1.f, 1.f, 1.f));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    glt::RenderManager& rm = engine.renderManager();

    rm.setCameraMatrix(glt::transformationWorldToLocal(camera));

    light = lightPosition(interpolation);
    
    rm.beginScene();

    ecLight = rm.geometryTransform().transformPoint(light);

    {
        vec3_t spot_center = vec3(5.f, 2.f, 8.f);
        vec3_t spotdir = spot_center - light;
        spotdir = normalize(spotdir);
        ec_spotlight_dir = rm.geometryTransform().normalMatrix() * spotdir;
        ec_spotlight_dir = normalize(ec_spotlight_dir);
    }

    renderLight();

    renderTable("teapotTextured");
    
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
}

void Anim::setupTeapotShader(const std::string& progname, const vec4_t& surfaceColor, const MaterialProperties& mat) {
    glt::RenderManager& rm = engine.renderManager();
    Ref<glt::ShaderProgram> progRef = engine.shaderManager().program(progname);
    if (!progRef) {
        ERR(("undefined program: "  + progname).c_str());
        return;
    }

    glt::ShaderProgram& prog = *progRef;

    ASSERT(!prog.wasError());
    ASSERT(prog.validate());

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
    us.optional("spotDirection", ec_spotlight_dir);
    us.optional("useSpot", use_spotlight);
    us.optional("spotSmooth", spotlight_smooth);
}

void Anim::renderLight() {
    glt::RenderManager& rm = engine.renderManager();
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().translate(light);
    rm.geometryTransform().scale(vec3(0.66f));

    MaterialProperties mat = { 0.8f, 0.2f, 1.f, 120.f };

    setupTeapotShader("teapot", vec4(1.f, 1.f, 0.f, 1.f), mat);
    sphereModel.draw();
}

void Anim::renderTeapot(const Teapot& teapot) {
    glt::RenderManager& rm = engine.renderManager();
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().concat(transformationLocalToWorld(teapot.frame));
    rm.geometryTransform().scale(vec3(13.f));

    setupTeapotShader("teapot", teapot.color.vec4(), teapot.material);
    teapotModel.draw();
}

void Anim::renderGround() {
    glt::RenderManager& rm = engine.renderManager();
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().scale(vec3(50.f));
    rm.geometryTransform().translate(vec3(-0.5f, 0.f, -0.5f));

    MaterialProperties mat = { 0.f, 1.f, 0.f, 30.f };
    vec4_t color = glt::color(0xcd, 0xc9, 0xc9).vec4();
    setupTeapotShader("teapot", color, mat);
    groundModel.draw();
}

void Anim::renderTable(const std::string& shader) {
    glt::RenderManager& rm = engine.renderManager();
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

void Anim::windowResized(const Event<WindowResized>& e) {
    uint32 width = e.info.width;
    uint32 height = e.info.height;
    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;
    GL_CHECK(glViewport(0, 0, width, height));

    float fov = degToRad(17.5f);
    float aspect = float(width) / float(height);

    engine.renderManager().setPerspectiveProjection(fov, aspect, 0.1f, 100.f);
}

void Anim::mouseMoved(const Event<MouseMoved>& e) {
    int32 dx = e.info.dx;
    int32 dy = e.info.dy;

    dx = - dx;

    if (engine.window().keyState(sf::Key::M) <= Down) {

        teapot1.frame.rotateWorld(-dx * 0.001f, camera.localY());
        teapot1.frame.rotateWorld(dy * 0.001f, camera.localX());

    } else {
        float rotX = dx * 0.001f;
        float rotY = dy * 0.001f;

        camera.rotateLocal(rotY, vec3(1.f, 0.f, 0.f));
        camera.rotateWorld(-rotX, vec3(0.f, 1.f, 0.f));
    }
}

void Anim::handleInput(const Event<EngineEvent>&) {
    vec3_t step = vec3(0.f);

    GameWindow& w = engine.window();

    namespace K = sf::Key;

    if (w.keyState(K::W) <= Down)
        step += vec3(0.f, 0.f, 1.f);
    if (w.keyState(K::S) <= Down)
        step += vec3(0.f, 0.f, -1.f);
    if (w.keyState(K::A) <= Down)
        step += vec3(1.f, 0.f, 0.f);
    if (w.keyState(K::D) <= Down)
        step += vec3(-1.f, 0.f, 0.f);

    static const float STEP = 0.1f;

    if (lengthSq(step) != 0.f)
        camera.translateLocal(STEP * normalize(step));
}

void Anim::keyStateChanged(const Event<KeyChanged>& e) {
    if (!e.info.pressed) return;

    const sf::Event::KeyEvent& key = e.info.key;

    using namespace sf::Key;

    switch (key.Code) {
    case C: engine.shaderManager().reloadShaders(); break;
    case B: engine.gameLoop().pause(!engine.gameLoop().paused()); break;
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
    case G:
        if (use_spotlight && spotlight_smooth) {
            use_spotlight = false;
            spotlight_smooth = false;
        } else if (use_spotlight) {
            spotlight_smooth = true;
        } else {
            use_spotlight = true;
        }
        break;
    }
}

int main(int argc, char *argv[]) {
    EngineOpts opts;
    opts.parseOpts(&argc, &argv);
    Engine engine;
    Anim anim(engine);
    opts.inits.init.reg(makeEventHandler(&anim, &Anim::linkEvents));
    opts.inits.init.reg(makeEventHandler(&anim, &Anim::init));
    return engine.run(opts);
}

static void makeUnitCube(CubeMesh2& cube) {
    Vertex2 v;
    QUAD_MESH(cube);

    v.normal = vec3( 0.0f, 0.0f, 1.0f);					
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3(0.f, 0.f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3( 1.0f, 0.f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3( 1.0f,  1.0f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3(0.f,  1.0f,  1.0f); ADD_VERTEX(cube, v);

    v.normal = vec3( 0.0f, 0.0f,0.f);					
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3(0.f, 0.f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3(0.f,  1.0f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3( 1.0f,  1.0f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3( 1.0f, 0.f, 0.f); ADD_VERTEX(cube, v);

    v.normal = vec3( 0.0f, 1.0f, 0.0f);					
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3(0.f,  1.0f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3(0.f,  1.0f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3( 1.0f,  1.0f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3( 1.0f,  1.0f, 0.f); ADD_VERTEX(cube, v);

    v.normal = vec3( 0.0f,0.f, 0.0f);					
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3(0.f, 0.f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3( 1.0f, 0.f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3( 1.0f, 0.f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3(0.f, 0.f,  1.0f); ADD_VERTEX(cube, v);

    v.normal = vec3( 1.0f, 0.0f, 0.0f);					
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3( 1.0f, 0.f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3( 1.0f,  1.0f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3( 1.0f,  1.0f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3( 1.0f, 0.f,  1.0f); ADD_VERTEX(cube, v);

    v.normal = vec3(0.f, 0.0f, 0.0f);					
    v.texCoord = vec2(0.0f, 0.0f); v.position = vec3(0.f, 0.f, 0.f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 0.0f); v.position = vec3(0.f, 0.f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(1.0f, 1.0f); v.position = vec3(0.f,  1.0f,  1.0f); ADD_VERTEX(cube, v);
    v.texCoord = vec2(0.0f, 1.0f); v.position = vec3(0.f,  1.0f, 0.f); ADD_VERTEX(cube, v);

    FREEZE_MESH(cube);
}

static void addTriangle(Mesh& s, const vec3_t vertices[3], const vec3_t normals[3], const vec2_t texCoords[3]) {
    UNUSED(texCoords);
    
    for (uint32 i = 0; i < 3; ++i) {
        Vertex v;
        v.position = vertices[i];
        v.normal = normals[i];
        ADD_VERTEX(s, v);
    }
}

// from the GLTools library (OpenGL Superbible)
static void gltMakeSphere(Mesh& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks)
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

    FREEZE_MESH(sphereBatch);
}

static void makeSphere(Mesh& sphere, float rad, int32 stacks, int32 slices) {
    sphere.primType(GL_TRIANGLES);
    gltMakeSphere(sphere, rad, stacks, slices);
}
