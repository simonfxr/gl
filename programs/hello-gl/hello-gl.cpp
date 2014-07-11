#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "opengl.hpp"
#include "cutil.h"

#include "sys/clock.hpp"
#include "ge/Engine.hpp"

#include "math/vec2.hpp"
#include "math/glvec.hpp"

#include "glt/utils.hpp"
#include "glt/Mesh.hpp"

using namespace defs;
using namespace math;

#define VERTEX(V, F, Z) \
    V(Vertex, Z(vec2_t, position))

DEFINE_VERTEX(VERTEX);
#undef VERTEX

struct Anim {
    ge::Engine engine;
    Ref<glt::ShaderProgram> program;

    Ref<glt::TextureSampler> texture0;
    Ref<glt::TextureSampler> texture1;
    glt::Mesh<Vertex> mesh;

    struct GlobalResources {
	GLfloat fade_factor;	
    };

    GlobalResources g_resources;

    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent>&);
    Ref<glt::TextureSampler> make_texture(const char *filename);
    int make_resources();
};

Ref<glt::TextureSampler> Anim::make_texture(const char *filename)
{
    Ref<glt::TextureSampler> tex = makeRef<glt::TextureSampler>(0);
    int width, height;
    void *pixels = read_tga(filename, &width, &height);

    if (!pixels)
        return tex;

    tex = makeRef(new glt::TextureSampler);
    GL_CALL(glBindTexture, GL_TEXTURE_2D, *tex->data()->ensureHandle());

    GL_CALL(glTexImage2D, 
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        width, height, 0,           /* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );
    free(pixels);

    tex->filterMode(glt::TextureSampler::FilterLinear);
    tex->clampMode(glt::TextureSampler::ClampToEdge);
    
    return tex;
}

/*
 * Data used to seed our vertex array and element array buffers:
 */
static const GLfloat g_vertex_buffer_data[] = { 
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

/*
 * Load and create all of our resources:
 */
int Anim::make_resources()
{
    texture0 = make_texture("programs/hello-gl/hello1.tga");
    texture1 = make_texture("programs/hello-gl/hello2.tga");

    if (!texture0 || !texture1)
        return 0;

    program = engine.shaderManager().program("hello-gl");
    if (!program)
        return 0;

    for (int i = 0; i < 4; ++i) {
        Vertex v;
        vec2_t pos;
        pos[0] = g_vertex_buffer_data[i * 2];
        pos[1] = g_vertex_buffer_data[i * 2 + 1];
        v.position = pos;
        mesh.addVertex(v);
    }

    for (int i = 0; i < 4; ++i) {
        mesh.addElement(g_element_buffer_data[i]);
    }

    mesh.primType(GL_TRIANGLE_STRIP);
    mesh.drawType(glt::DrawElements);
    mesh.send();

    return 1;
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    if (!make_resources())
        return;
    ev.info.success = true;
    INFO("resources loaded");
}

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>& ev) {
    ge::Engine& engine = ev.info.engine;
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    float secs = (float) sys::queryTimer();
    g_resources.fade_factor = sinf(secs * 0.5f + 0.5f);

    texture0->bind(0);
    texture1->bind(1);
    
    program->use();
    glt::Uniforms(*program)
	.optional("fade_factor", g_resources.fade_factor)
        .optional("texture0", glt::Sampler(*texture0, 0))
        .optional("texture1", glt::Sampler(*texture1, 1));

    mesh.draw();
}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    return anim.engine.run(opts);
}
