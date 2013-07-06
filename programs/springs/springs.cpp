#include "ge/Engine.hpp"
#include "ge/Camera.hpp"
#include "ge/MouseLookPlugin.hpp"

#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"
#include "glt/utils.hpp"
#include "glt/Uniforms.hpp"

#include "data/SharedArray.hpp"

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

    GLuint particle_vertex_array[2];

    index particle_source; // 0 or 1
    
    // vec4 of (x, y, z, 1/m)
    GLuint particle_pos_mass_handle[2];
    // velocity (vec3)
    GLuint particle_vel_handle[2];
    // ivec4 spring connections
    GLuint particle_conn_handle;

    GLuint particle_pos_mass_tex[2];
    GLuint particle_vel_tex[2];
    GLuint particle_conn_tex;

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

    engine.gameLoop().ticksPerSecond(400);

    glt::primitives::sphere(sphere_model, 1.f, 30, 15);
    sphere_model.send();
    sphere_model.enableAttribute(1, false); // disable normals

    particle_source = 0;
    std::vector<vec4_t> particle_pos_mass;
    std::vector<vec3_t> particle_vel;
    std::vector<GLint>  particle_conn;

    for (index i = 0; i < N_SPRINGS; ++i) {
        for (index j = 0; j < N_SPRINGS; ++j) {
            // arrange in grid of N_SPRINGS by N_SPRINGS 
            vec3_t pos = vec3(real(i * N_SPRINGS), 0, real(j * N_SPRINGS));
            real inv_mass = real(N_SPRINGS * N_SPRINGS); // accumulated
                                                         // mass is 1
            if ((i == 0 || i == N_SPRINGS - 1) || (j == 0 || j == N_SPRINGS - 1)) {
                inv_mass = 0.f; // the edges have infinite mass;
            }

            
            particle_pos_mass.push_back(vec4(pos[0], pos[1], pos[2], inv_mass));
            particle_vel.push_back(vec3(real(0)));
#define INDEX(i, j) ((i) * N_SPRINGS + (j))
            particle_conn.push_back(i > 0 ? INDEX(i - 1, j) : INDEX(i, j));
            particle_conn.push_back(i + 1 < N_SPRINGS ? INDEX(i + 1, j) : INDEX(i, j));
            particle_conn.push_back(j > 0 ? INDEX(i, j - 1) : INDEX(i, j));
            particle_conn.push_back(j + 1 < N_SPRINGS ? INDEX(i, j + 1) : INDEX(i, j));
#undef INDEX
        }
    }

    GL_CALL(glGenVertexArrays, 2, particle_vertex_array);
    GL_CALL(glGenBuffers, 2, particle_pos_mass_handle);
    GL_CALL(glGenBuffers, 2, particle_vel_handle);
    GL_CALL(glGenBuffers, 1, &particle_conn_handle);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_pos_mass_handle[0]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vec4_t) * particle_pos_mass.size(), &particle_pos_mass[0], GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_vel_handle[0]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vec3_t) * particle_vel.size(), &particle_vel[0], GL_STREAM_DRAW);
    
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_pos_mass_handle[1]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vec4_t) * particle_pos_mass.size(), 0, GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_vel_handle[1]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vec3_t) * particle_vel.size(), 0, GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_conn_handle);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(GLint) * particle_conn.size(), &particle_conn[0], GL_STREAM_DRAW);

    for (index i = 0; i < 2; ++i) {
        GL_CALL(glBindVertexArray, particle_vertex_array[i]);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_pos_mass_handle[i]);
        GL_CALL(glVertexAttribPointer, 0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CALL(glEnableVertexAttribArray, 0);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_vel_handle[i]);
        GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CALL(glEnableVertexAttribArray, 1);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_conn_handle);
        GL_CALL(glVertexAttribIPointer, 2, 4, GL_INT, 0, 0);
        GL_CALL(glEnableVertexAttribArray, 2);
            
        GL_CALL(glBindVertexArray, 0);
    }

    GL_CALL(glGenTextures, 2, particle_pos_mass_tex);
    GL_CALL(glGenTextures, 2, particle_vel_tex);
    GL_CALL(glGenTextures, 1, &particle_conn_tex);

    for (index i = 0; i < 2; ++i) {
        GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_pos_mass_tex[i]);
        GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA32F, particle_pos_mass_handle[i]);

        GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_vel_tex[i]);
        GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGB32F, particle_vel_handle[i]);
    }
    
    GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_conn_tex);
    GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA32I, particle_conn_handle);

    physics_prog = makeRef(new glt::ShaderProgram(engine.shaderManager()));
    physics_prog->addShaderFile("physics.vert");
    physics_prog->bindAttribute("position_mass", 0);
    physics_prog->bindAttribute("velocity", 1);
    physics_prog->bindAttribute("connection", 2);
    const char *stream_vars[] = { "Out1.position_mass", "Out2.velocity" };

    GL_CALL(glTransformFeedbackVaryings, *physics_prog->program(), ARRAY_LENGTH(stream_vars), stream_vars, GL_SEPARATE_ATTRIBS);
    
    ASSERT(physics_prog->link());
    
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
    GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_pos_mass_tex[particle_source]);

    prog->use();
    glt::Uniforms(*prog)
        .optional("color", glt::color(0xFF, 0, 0))
        .optional("mvpMatrix", engine.renderManager().geometryTransform().mvpMatrix())
        .mandatory("particle_data1", glt::BoundTexture(GL_SAMPLER_BUFFER, 0));

    sphere_model.drawInstanced(N_SPRINGS * N_SPRINGS);    
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    GL_CALL(glEnable, GL_RASTERIZER_DISCARD);

    index particle_dest = (1 + particle_source) % 2;

    physics_prog->use();

    GL_CALL(glActiveTexture, GL_TEXTURE0);
    GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_pos_mass_tex[particle_source]);


    glt::Uniforms(*physics_prog)
        .optional("dt", engine.gameLoop().frameDuration())
        .optional("D", 0.1f)
        .optional("damping", 0.98f)
        .mandatory("position_mass_sampler", glt::BoundTexture(GL_SAMPLER_BUFFER, 0));
    
    GL_CALL(glBindBufferBase, GL_TRANSFORM_FEEDBACK_BUFFER, 0, particle_pos_mass_handle[particle_dest]);
    GL_CALL(glBindBufferBase, GL_TRANSFORM_FEEDBACK_BUFFER, 1, particle_vel_handle[particle_dest]);

    GL_CALL(glBindVertexArray, particle_vertex_array[particle_source]);

    GL_CALL(glBeginTransformFeedback, GL_POINTS);
    GL_CALL(glDrawArrays, GL_POINTS, 0, N_SPRINGS * N_SPRINGS);
    GL_CALL(glEndTransformFeedback);
    
    GL_CALL(glDisable, GL_RASTERIZER_DISCARD);

    particle_source = particle_dest;
}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
