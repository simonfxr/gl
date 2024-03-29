#include "ge/Camera.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"
#include "ge/Timer.hpp"
#include "glt/GLPerfCounter.hpp"
#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"
#include "glt/Uniforms.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "util/range.hpp"

#include <algorithm>
#include <map>
#include <vector>

const size_t N_SPRINGS = 20;

using namespace math;

DEF_GL_MAPPED_TYPE(Vertex, (vec3_t, position), (vec3_t, normal))

template<typename Vertex>
void
sphere(glt::Mesh<Vertex> &mesh, real radius, int slices, int stacks);

template<typename V>
void
icoSphere(glt::Mesh<V> &mesh, size_t subdivs);

struct Anim
{
    ge::Engine engine;

    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;

    glt::Mesh<Vertex> sphere_model;

    GLuint particle_vertex_array[2];

    size_t particle_source; // 0 or 1

    // vec4 of (x, y, z, 1/m)
    GLuint particle_pos_mass_handle[2];
    // velocity (vec3)
    GLuint particle_vel_handle[2];
    // ivec4 spring connections
    GLuint particle_conn_handle;

    GLuint particle_pos_mass_tex[2];
    GLuint particle_vel_tex[2];
    GLuint particle_conn_tex;

    ge::Timer fpsTimer;

    glt::GLPerfCounter physics_perf_counter;
    double pc_sum;
    int pc_count;

    std::shared_ptr<glt::ShaderProgram> physics_prog;

    Anim() : fpsTimer(engine) {}

    void init(const ge::Event<ge::InitEvent> &);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent> &);
    void animate(const ge::Event<ge::AnimationEvent> &);
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    engine.enablePlugin(mouse_look);
    mouse_look.camera(&camera);
    engine.enablePlugin(camera);
    camera.frame().origin = vec3(0.f);

    engine.gameLoop().ticks(300);
    engine.gameLoop().pause(true);

    fpsTimer.start(1.f, true);

    glt::primitives::sphere(sphere_model, 1.f, 30, 15);
    sphere_model.send();
    sphere_model.enableAttribute(1, false); // disable normals

    physics_perf_counter.init(2);

    particle_source = 0;
    std::vector<vec4_t> particle_pos_mass;
    std::vector<vec3_t> particle_vel;
    std::vector<GLint> particle_conn;

    for (const auto i : irange(N_SPRINGS)) {
        for (const auto j : irange(N_SPRINGS)) {
            // arrange in grid of N_SPRINGS by N_SPRINGS
            vec3_t x = vec3(real(i * N_SPRINGS), 0, real(j * N_SPRINGS));
            real inv_mass = real(N_SPRINGS * N_SPRINGS); // accumulated
                                                         // mass is 1
            if ((i == 0 || i == N_SPRINGS - 1) ||
                (j == 0 || j == N_SPRINGS - 1)) {
                inv_mass = 0.f; // the edges have infinite mass;
            }

            float d = length(x - vec3(real(N_SPRINGS) * real(0.5),
                                      0,
                                      real(N_SPRINGS) * real(0.5)));
            x[1] += real(0.5) * d;

            particle_pos_mass.push_back(vec4(x[0], x[1], x[2], inv_mass));
            particle_vel.push_back(vec3(real(0)));
#define INDEX(i, j) GLint((i) *N_SPRINGS + (j))
            particle_conn.push_back(i > 0 ? INDEX(i - 1, j) : INDEX(i, j));
            particle_conn.push_back(i + 1 < N_SPRINGS ? INDEX(i + 1, j)
                                                      : INDEX(i, j));
            particle_conn.push_back(j > 0 ? INDEX(i, j - 1) : INDEX(i, j));
            particle_conn.push_back(j + 1 < N_SPRINGS ? INDEX(i, j + 1)
                                                      : INDEX(i, j));
#undef INDEX
        }
    }

    GL_CALL(glGenVertexArrays, 2, particle_vertex_array);
    GL_CALL(glGenBuffers, 2, particle_pos_mass_handle);
    GL_CALL(glGenBuffers, 2, particle_vel_handle);
    GL_CALL(glGenBuffers, 1, &particle_conn_handle);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_pos_mass_handle[0]);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            sizeof(vec4_t) * particle_pos_mass.size(),
            &particle_pos_mass[0],
            GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_vel_handle[0]);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            sizeof(vec3_t) * particle_vel.size(),
            &particle_vel[0],
            GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_pos_mass_handle[1]);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            sizeof(vec4_t) * particle_pos_mass.size(),
            nullptr,
            GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_vel_handle[1]);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            sizeof(vec3_t) * particle_vel.size(),
            nullptr,
            GL_STREAM_DRAW);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_conn_handle);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            sizeof(GLint) * particle_conn.size(),
            &particle_conn[0],
            GL_STREAM_DRAW);

    for (const auto i : irange(2)) {
        GL_CALL(glBindVertexArray, particle_vertex_array[i]);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_pos_mass_handle[i]);
        GL_CALL(glVertexAttribPointer, 0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        GL_CALL(glEnableVertexAttribArray, 0);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_vel_handle[i]);
        GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        GL_CALL(glEnableVertexAttribArray, 1);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, particle_conn_handle);
        GL_CALL(glVertexAttribIPointer, 2, 4, GL_INT, 0, nullptr);
        GL_CALL(glEnableVertexAttribArray, 2);

        GL_CALL(glBindVertexArray, 0);
    }

    GL_CALL(glGenTextures, 2, particle_pos_mass_tex);
    GL_CALL(glGenTextures, 2, particle_vel_tex);
    GL_CALL(glGenTextures, 1, &particle_conn_tex);

    for (const auto i : irange(2)) {
        GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_pos_mass_tex[i]);
        GL_CALL(glTexBuffer,
                GL_TEXTURE_BUFFER,
                GL_RGBA32F,
                particle_pos_mass_handle[i]);

        GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_vel_tex[i]);
        GL_CALL(
          glTexBuffer, GL_TEXTURE_BUFFER, GL_RGB32F, particle_vel_handle[i]);
    }

    GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, particle_conn_tex);
    GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA32I, particle_conn_handle);

    physics_prog = std::make_shared<glt::ShaderProgram>(engine.shaderManager());
    physics_prog->addShaderFile("physics.vert");
    physics_prog->bindAttribute("position_mass", 0);
    physics_prog->bindAttribute("velocity", 1);
    physics_prog->bindAttribute("connection", 2);
    const char *stream_vars[] = { "Out1.position_mass", "Out2.velocity" };

    GL_CALL(glTransformFeedbackVaryings,
            *physics_prog->program(),
            ARRAY_LENGTH(stream_vars),
            stream_vars,
            GL_SEPARATE_ATTRIBS);

    ASSERT(physics_prog->link());

    ev.info.success = true;
}

void
Anim::link()
{
    engine.events().render.reg(*this, &Anim::renderScene);
    engine.events().animate.reg(*this, &Anim::animate);
}

void
Anim::renderScene(const ge::Event<ge::RenderEvent> &)
{
    auto rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    auto prog = engine.shaderManager().program("sphere");
    ASSERT(prog);

    GL_CALL(glActiveTexture, GL_TEXTURE0);
    GL_CALL(
      glBindTexture, GL_TEXTURE_BUFFER, particle_pos_mass_tex[particle_source]);

    {
        glt::GeometryTransform &gt = engine.renderManager().geometryTransform();
        prog->use();
        glt::Uniforms(*prog)
          .optional("color", glt::color(0xFF, 0, 0))
          .optional("vpMatrix", gt.vpMatrix())
          .optional("grid_size", real(N_SPRINGS))
          .mandatory("particle_data1", glt::BoundTexture(GL_SAMPLER_BUFFER, 0));
    }

    sphere_model.drawInstanced(N_SPRINGS * N_SPRINGS);

    ++pc_count;
    pc_sum += physics_perf_counter.query();

    if (fpsTimer.fire()) {
        sys::io::stdout() << "simulation fps: "
                          << (1 / physics_perf_counter.query())
                          << ", avg: " << (pc_count / pc_sum) << "\n";
        pc_sum = 0.0;
        pc_count = 0;
    }
}

void
Anim::animate(const ge::Event<ge::AnimationEvent> &)
{
    physics_perf_counter.begin();

    GL_CALL(glEnable, GL_RASTERIZER_DISCARD);

    auto particle_dest = (1 + particle_source) % 2;

    physics_prog->use();

    GL_CALL(glActiveTexture, GL_TEXTURE0);
    GL_CALL(
      glBindTexture, GL_TEXTURE_BUFFER, particle_pos_mass_tex[particle_source]);

    auto dt = real(engine.gameLoop().tickDuration());
    auto damp_coeff = 0.99_r; // damping over the period of 1 sec;
                              //    real damping = pow(damp_coeff, dt);
    glt::Uniforms(*physics_prog)
      .optional("dt", dt)
      .optional("D", 1.f)
      .optional("damping", damp_coeff)
      .mandatory("position_mass_sampler",
                 glt::BoundTexture(GL_SAMPLER_BUFFER, 0));

    GL_CALL(glBindBufferBase,
            GL_TRANSFORM_FEEDBACK_BUFFER,
            0,
            particle_pos_mass_handle[particle_dest]);
    GL_CALL(glBindBufferBase,
            GL_TRANSFORM_FEEDBACK_BUFFER,
            1,
            particle_vel_handle[particle_dest]);

    GL_CALL(glBindVertexArray, particle_vertex_array[particle_source]);

    GL_CALL(glBeginTransformFeedback, GL_POINTS);
    GL_CALL(glDrawArrays, GL_POINTS, 0, N_SPRINGS * N_SPRINGS);
    GL_CALL(glEndTransformFeedback);

    GL_CALL(glDisable, GL_RASTERIZER_DISCARD);

    particle_source = particle_dest;

    physics_perf_counter.end();
}

int
main(int argc, char *argv[])
{
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    anim.engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    opts.inits.reg(ge::Init, anim, &Anim::init);
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
