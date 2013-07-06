#include "ge/Engine.hpp"
#include "ge/Camera.hpp"
#include "ge/MouseLookPlugin.hpp"

#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"
#include "glt/utils.hpp"
#include "glt/Uniforms.hpp"

#include "data/Array.hpp"

#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include <vector>
#include <map>
#include <algorithm>

const size N_SPRINGS = 10;

using namespace defs;
using namespace math;

struct Vertex {
    vec3_t position;
    vec3_t normal;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position),
                   VERTEX_ATTR(Vertex, normal)
);

template <typename Vertex>
void sphere(glt::Mesh<Vertex>& mesh, real radius, int slices, int stacks);

template <typename V>
void icoSphere(glt::Mesh<V>& mesh, size subdivs);

struct Anim {
    ge::Engine engine;

    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;

    glt::Mesh<Vertex> sphere_model;

    GLuint particle_vertex_array;

    index particle_source; // 0 or 1
    
    // vec4 of (x, y, z, m)
    GLuint particle_data1_handle[2];
    // ivec4 spring connections
    GLuint particle_data2_handle;

    GLuint particle_data1_tex[2];
    GLuint particle_data2_tex;

    Ref<glt::ShaderProgram> physics_prog;
    
    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent>&);
    void animate(const ge::Event<ge::AnimationEvent>&);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    engine.enablePlugin(mouse_look);
    mouse_look.camera(&camera);
    engine.enablePlugin(camera);
    camera.frame().origin = vec3(0.f);

    glt::primitives::sphere(sphere_model, 1.f, 30, 15);
    sphere_model.send();
    sphere_model.enableAttribute(1, false); // disable normals

    GL_CALL(glGenVertexArrays, 1, &particle_vertex_array);
    GL_CALL(glGenBuffers, 2, particle_data1_handle);
    GL_CALL(glGenBuffers, 1, &particle_data2_handle);

    particle_source = 0;
    std::vector<vec4_t> particle_data1;
    std::vector<GLshort>  particle_data2;


    for (index i = 0; i < N_SPRINGS; ++i) {
        for (index j = 0; j < N_SPRINGS; ++j) {
            // arrange in grid of N_SPRINGS by N_SPRINGS 
            vec3_t pos = vec3(real(i * N_SPRINGS), 0, real(j * N_SPRINGS));
            real mass = real(1) / real(N_SPRINGS * N_SPRINGS); // accumulated
                                                     // mass is 1
            particle_data1.push_back(vec4(pos[0], pos[1], pos[2], mass));
#define INDEX(i, j) ((i) * N_SPRINGS + (j))
            particle_data2.push_back(i > 0 ? INDEX(i - 1, j) : -1);
            particle_data2.push_back(i + 1 < N_SPRINGS ? INDEX(i + 1, j) : -1);
            particle_data2.push_back(j > 0 ? INDEX(i, j - 1) : -1);
            particle_data2.push_back(j + 1 < N_SPRINGS ? INDEX(i, j + 1) : -1);
#undef INDEX
        }
    }

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_data1_handle[0]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vec4_t) * particle_data1.size(), &particle_data1[0], GL_STREAM_DRAW);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_data2_handle);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(GLshort) * particle_data1.size(), &particle_data2[0], GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_data1_handle[1]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vec4_t) * particle_data1.size(), 0, GL_STREAM_DRAW);

    // GL_CALL(glBindVertexArray, particle_vertex_array);

    // GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_data1_handle);
    // GL_CALL(glVertexAttribPointer, 0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    // GL_CALL(glEnableVertexAttribArray, 0);
    
    // GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_data2_handle);
    // GL_CALL(glVertexAttribIPointer, 1, 4, GL_SHORT, 0, 0);
    // GL_CALL(glEnableVertexAttribArray, 1);
            
    // GL_CALL(glBindVertexArray, 0);

    GL_CALL(glGenTextures, 2, particle_data1_tex);
    GL_CALL(glGenTextures, 1, &particle_data2_tex);

    for (index i = 0; i < 2; ++i) {
        GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_data1_tex[i]);
        GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA32F, particle_data1_handle[i]);
    }
    
    GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_data2_tex);
    GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA16I, particle_data2_handle);

    physics_prog = makeRef(new glt::ShaderProgram(engine.shaderManager()));
    physics_prog->addShaderFile("physics.vert");
    physics_prog->bindAttribute("position_mass", 0);
    std::string stream_vars[] = { "Vertex.position_mass" };
    physics_prog->bindStreamOutVaryings(SharedArray<std::string>(stream_vars, ARRAY_LENGTH(stream_vars)));
    physics_prog->link();
    
    ev.info.success = true;
}

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
    engine.events().animate.reg(ge::makeEventHandler(this, &Anim::animate));
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>&) {
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    Ref<glt::ShaderProgram> prog = engine.shaderManager().program("sphere");
    ASSERT(prog);

    GL_CALL(glActiveTexture, GL_TEXTURE0);
    GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_data1_tex[particle_source]);

    prog->use();
    glt::Uniforms(*prog)
        .optional("color", glt::color(0xFF, 0, 0))
        .optional("mvpMatrix", engine.renderManager().geometryTransform().mvpMatrix())
        .mandatory("particle_data1", glt::BoundTexture(GL_SAMPLER_BUFFER, 0));

    sphere_model.drawInstanced(N_SPRINGS * N_SPRINGS);    
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {

}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
