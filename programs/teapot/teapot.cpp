#include <iostream>
#include <cstdio>
#include <cstring>

#include <GL/glew.h>

#include <SFML/Graphics/Image.hpp>

#include "defs.hpp"
#include "mesh.h"

#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/io.hpp"

#include "glt/utils.hpp"
#include "glt/Uniforms.hpp"
#include "glt/Frame.hpp"
#include "glt/color.hpp"
#include "glt/primitives.hpp"
#include "glt/TextureRenderTarget.hpp"

#include "ge/Engine.hpp"
#include "ge/Camera.hpp"
#include "ge/CommandParams.hpp"

#include "parse_sply.hpp"

using namespace math;
using namespace ge;
using namespace defs;

static const point3_t LIGHT_CENTER_OF_ROTATION = vec3(0.f, 15.f, 0.f);
static const float  LIGHT_ROTATION_RAD = 15.f;

struct Vertex2 {
    vec3_t position;
    vec3_t normal;
    vec2_t texCoord;
};

struct ScreenVertex {
    vec3_t position;
    vec3_t normal;
};

#if defined(MESH_MESH)

DEFINE_VERTEX_DESC(Vertex,
                    VERTEX_ATTR(Vertex, position),
                    VERTEX_ATTR(Vertex, normal));

DEFINE_VERTEX_DESC(Vertex2,
                    VERTEX_ATTR(Vertex2, position),
                    VERTEX_ATTR(Vertex2, normal),
                    VERTEX_ATTR(Vertex2, texCoord));

DEFINE_VERTEX_DESC(ScreenVertex,
                   VERTEX_ATTR(ScreenVertex, position),
                   VERTEX_ATTR(ScreenVertex, normal));

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
    glt::CubeMesh<ScreenVertex> screenQuad;

    sf::Texture woodTexture;

    Teapot teapot1;
    Teapot teapot2;

    bool wireframe_mode;

    real light_angular_position;
    real light_rotation_speed;

    real gamma_correction;

    vec3_t light;
    vec3_t ecLight;
    
    uint32 shade_mode;

    bool use_spotlight;
    bool spotlight_smooth;
    direction3_t ec_spotlight_dir;

    Ref<glt::TextureRenderTarget> tex_render_target;

    std::string data_dir;

    Ref<Command> setDataDirCommand;

    Anim(ge::Engine& e) :
        engine(e)
        {}

    void init(const Event<InitEvent>&);
    void link(const Event<InitEvent>&);
    void animate(const Event<AnimationEvent>&);
    void renderScene(const Event<RenderEvent>&);
    vec3_t lightPosition(real interpolation);
    void setupTeapotShader(const std::string& prog, const vec4_t& surfaceColor, const MaterialProperties& mat);
    void renderTeapot(const Teapot& teapot);
    void renderGround();
    void renderLight();
    void renderTable(const std::string& shader);
    // void windowResized(const Event<WindowResized>&);
    void mouseMoved(const Event<MouseMoved>&);
    void keyPressed(const Event<KeyPressed>&);
    // void handleInput(const Event<InputEvent>&);
    void setDataDir(const Event<CommandEvent>&, const Array<CommandArg>& args);
    
    void onWindowResized(const Event<WindowResized>&);
};

void Anim::link(const Event<InitEvent>& e) {
    // engine.events().handleInput.reg(makeEventHandler(this, &Anim::handleInput));
    engine.events().animate.reg(makeEventHandler(this, &Anim::animate));
    engine.events().render.reg(makeEventHandler(this, &Anim::renderScene));
    GameWindow& win = engine.window();
    // win.events().windowResized.reg(makeEventHandler(this, &Anim::windowResized));
    win.events().mouseMoved.reg(makeEventHandler(this, &Anim::mouseMoved));
    win.events().windowResized.reg(makeEventHandler(this, &Anim::onWindowResized));
    engine.keyHandler().keyPressedEvent().reg(makeEventHandler(this, &Anim::keyPressed));
    e.info.success = true;
}

void Anim::init(const Event<InitEvent>& e) {
    engine.window().showMouseCursor(false);
    engine.window().grabMouse(true);

    camera.registerWith(engine);
    camera.registerCommands(engine.commandProcessor());

    setDataDirCommand = makeCommand(this, &Anim::setDataDir, ge::STR_PARAMS,
                                    "setDataDir", "set the texture directory");
    engine.commandProcessor().define(setDataDirCommand);    

    engine.gameLoop().ticksPerSecond(100);
    engine.gameLoop().sync(true);

    use_spotlight = false;
    spotlight_smooth = false;

    wireframe_mode = false;
    // GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    shade_mode = SHADE_MODE_DEFAULT;

    light_rotation_speed = 0.0025f;
    light_angular_position = 0.f;

    gamma_correction = 1.35f;

    QUAD_MESH(cubeModel);
    glt::primitives::unitCube3(cubeModel);
    FREEZE_MESH(cubeModel);

    glt::primitives::sphere(sphereModel, 1.f, 26, 13);
    sphereModel.send();

    {
        QUAD_MESH(groundModel);

        Vertex v;
        v.normal = vec3(0.f, 1.f, 0.f);
        v.position = vec3(0.f, 0.f, 0.f); ADD_VERTEX(groundModel, v);
        v.position = vec3(1.f, 0.f, 0.f); ADD_VERTEX(groundModel, v);
        v.position = vec3(1.f, 0.f, 1.f); ADD_VERTEX(groundModel, v);
        v.position = vec3(0.f, 0.f, 1.f); ADD_VERTEX(groundModel, v);

        FREEZE_MESH(groundModel);
    }

    {
        QUAD_MESH(screenQuad);
        ScreenVertex v;
        v.normal = vec3(0.f, 0.f, 1.f);
        v.position = vec3(0, 0, 0); screenQuad.add(v);
        v.position = vec3(1, 0, 0); screenQuad.add(v);
        v.position = vec3(1, 1, 0); screenQuad.add(v);
        v.position = vec3(0, 1, 0); screenQuad.add(v);
        
        FREEZE_MESH(screenQuad);
    }

    camera.frame.origin = vec3(6.36, 5.87, 1.97);
    camera.frame.setXZ(normalize(vec3(-0.29, 0.f, 0.95f)),
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

    {
        size w = SIZE(engine.window().window().getSize().x);
        size h = SIZE(engine.window().window().getSize().y);
        glt::TextureRenderTarget::Params ps;
        ps.samples = 4;
        ps.buffers = glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER;
        tex_render_target = makeRef(new glt::TextureRenderTarget(w, h, ps));
        engine.renderManager().setDefaultRenderTarget(tex_render_target.ptr());
    }

    e.info.success = true;
}

void Anim::animate(const Event<AnimationEvent>&) {
    light_angular_position = wrapPi(light_angular_position + light_rotation_speed);
}

vec3_t Anim::lightPosition(real interpolation) {
    real theta = wrapPi(light_angular_position + interpolation * light_rotation_speed);
    vec3_t d = vec3(cos(theta), 0.f, sin(theta));
    return d * LIGHT_ROTATION_RAD + LIGHT_CENTER_OF_ROTATION;
}

void Anim::renderScene(const Event<RenderEvent>& e) {
    real interpolation = e.info.interpolation;

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    glt::RenderManager& rm = engine.renderManager();
    rm.activeRenderTarget()->clearColor(glt::color(vec4(1.f, 1.f, 1.f, 1.f)));
    rm.activeRenderTarget()->clear();

    light = lightPosition(interpolation);
    
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

    { // render texture to window framebuffer
        engine.renderManager().setActiveRenderTarget(&engine.window().renderTarget());
        GL_CHECK(glDisable(GL_DEPTH_TEST));
        Ref<glt::ShaderProgram> postprocShader = engine.shaderManager().program("postproc");
        ASSERT(postprocShader);
        postprocShader->use();

        tex_render_target->textureHandle().bind(0);
        glt::Uniforms(*postprocShader)
            .mandatory("texture0", glt::Sampler(tex_render_target->textureHandle(), 0));
        screenQuad.draw();
    }
}

void Anim::onWindowResized(const Event<WindowResized>& ev) {
    size w = SIZE(ev.info.window.window().getSize().x);
    size h = SIZE(ev.info.window.window().getSize().y);
    engine.renderManager().setActiveRenderTarget(0);
    tex_render_target->resize(w, h);
    engine.renderManager().setDefaultRenderTarget(tex_render_target.ptr());
}

void Anim::setupTeapotShader(const std::string& progname, const vec4_t& surfaceColor, const MaterialProperties& mat) {
    glt::RenderManager& rm = engine.renderManager();
    Ref<glt::ShaderProgram> prog = engine.shaderManager().program(progname);
    if (!prog) {
        ASSERT_MSG(prog, "undefined program: "  + progname);
        return;
    }

    ASSERT(!prog->wasError());
    ASSERT(prog->validate());

    prog->use();

    vec4_t materialSelector = vec4((shade_mode & SHADE_MODE_AMBIENT) != 0,
                                   (shade_mode & SHADE_MODE_DIFFUSE) != 0,
                                   (shade_mode & SHADE_MODE_SPECULAR) != 0,
                                   1.f);

    real scale = 1 / (2.f * math::PI);
    real specular_factor = mat.specularContribution * (mat.shininess + 2) * scale;
    vec4_t vm = vec4(mat.ambientContribution, mat.diffuseContribution,
                     specular_factor, mat.shininess);
    
    glt::Uniforms us(*prog);
    us.optional("projectionMatrix", rm.geometryTransform().projectionMatrix());
    us.optional("mvMatrix", rm.geometryTransform().mvMatrix());
    us.optional("normalMatrix", rm.geometryTransform().normalMatrix());
    us.optional("surfaceColor", surfaceColor);
    us.optional("materialProperties", vm * materialSelector);
    us.optional("ecLight", ecLight);
    us.optional("gammaCorrection", gamma_correction);
    us.optional("spotDirection", ec_spotlight_dir);
    us.optional("useSpot", 1.f * use_spotlight);
    us.optional("spotSmooth", 1.f * spotlight_smooth);
    us.optional("texData", glt::BoundSampler(GL_SAMPLER_2D, 0));
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

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    woodTexture.bind();

    vec4_t color = glt::color(0xcd, 0x85, 0x3f).vec4();
    MaterialProperties mat;
    mat.ambientContribution = 0.55;
    mat.diffuseContribution = 0.6;
    mat.specularContribution = 0.075f;
    mat.shininess = 30;

    real table_height = 1.f;
    real table_thickness = 1 / 33.f;
    real foot_height = table_height - table_thickness;
    real foot_width = 0.03f;
    real foot_depth = 10.f/16.f * foot_width;
    real foot_x_dist = 1.f - foot_width;
    real foot_z_dist = 1.f - foot_depth;

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

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    
}

// void Anim::windowResized(const Event<WindowResized>& e) {
//     uint32 width = e.info.width;
//     uint32 height = e.info.height;
//     std::cerr << "new window dimensions: " << width << "x" << height << std::endl;
//     GL_CHECK(glViewport(0, 0, width, height));

//     real fov = degToRad(17.5f);
//     real aspect = real(width) / real(height);

//     engine.renderManager().setPerspectiveProjection(fov, aspect, 0.1f, 100.f);
// }

void Anim::mouseMoved(const Event<MouseMoved>& e) {
    int32 dx = e.info.dx;
    int32 dy = e.info.dy;

    dx = - dx;

    if (engine.keyHandler().keyState(ge::keycode::M) <= Pressed) {
        teapot1.frame.rotateWorld(-dx * 0.001f, camera.frame.localY());
        teapot1.frame.rotateWorld(dy * 0.001f, camera.frame.localX());
        e.abort = true;        
    }
}

void Anim::keyPressed(const Event<KeyPressed>& e) {
    using namespace ge::keycode;

    switch (e.info.key) {
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

void Anim::setDataDir(const Event<CommandEvent>&, const Array<CommandArg>& args) {
    data_dir = *args[0].string;

    if (!woodTexture.loadFromFile(data_dir + "/wood.jpg")) {
        std::cerr << "couldnt load data/wood.jpg" << std::endl;
        return;
    }

    woodTexture.setSmooth(true);

    int32 nfaces = parse_sply((data_dir + "/teapot.sply").c_str(), teapotModel);
    if (nfaces < 0) {
        std::cerr << "couldnt parse teapot model" << std::endl;
        return;
    } else {
        std::cerr << "parsed teapot model: " << nfaces << " vertices" << std::endl;
    }

    QUAD_MESH(teapotModel);
    teapotModel.drawType(glt::DrawElements);
    FREEZE_MESH(teapotModel);
}

int main(int argc, char *argv[]) {
    EngineOptions opts;
    Engine engine;
    Anim anim(engine);

    opts.parse(&argc, &argv);
    opts.inits.init.reg(makeEventHandler(&anim, &Anim::init));
    opts.inits.init.reg(makeEventHandler(&anim, &Anim::link));
    
    return engine.run(opts);
}
