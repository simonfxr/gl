#ifndef SIM_UTILS_HPP
#define SIM_UTILS_HPP 1

#include "ge/Engine.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/color.hpp"
#include "math/vec2.hpp"
#include "math/vec4.hpp"

#include "glt/Mesh.hpp"
#include "glt/utils.hpp"

DEF_GL_MAPPED_TYPE(Vertex2D, (math::vec2_t, position))

struct ParticleRenderer
{

    struct Opts
    {
        math::vec2_t world_size;
        Opts() {}
    };

    struct Particle
    {
        math::vec2_t position;
        math::real radius;
        math::real _internal_padding;
        math::vec4_t color;
    };

    bool init(ge::Engine *, const Opts &);
    void renderParticles(const ge::Event<ge::RenderEvent> &,
                         size_t,
                         const Particle *);

private:
    ge::Engine *engine;
    Opts opts;
    std::shared_ptr<glt::Mesh<Vertex2D>> circle_mesh;
    std::shared_ptr<glt::ShaderProgram> shader;
};

#define DEF_SHADER(name, ...) const char *const name = PP_TOSTR(__VA_ARGS__);

DEF_SHADER(
  PARTICLE_VERTEX_SHADER, uniform mat4 mvpMatrix;
  uniform sampler1D instanceData;

  in vec2 position;
  out vec4 color;

  void main() {
      vec4 data1 = texelFetch(instanceData, gl_InstanceID * 2, 0);
      vec4 data2 = texelFetch(instanceData, gl_InstanceID * 2 + 1, 0);

      vec2 center = data1.xy;
      float radius = data1.z;
      color = data2.rgba;

      gl_Position = mvpMatrix * vec4(position * vec2(radius) + center, 0, 1);
  });

DEF_SHADER(
  PARTICLE_FRAGMENT_SHADER, in vec4 color; out vec4 fragColor;
  void main() { fragColor = color; });

#undef DEF_SHADER

bool
ParticleRenderer::init(ge::Engine *e, const ParticleRenderer::Opts &popts)
{

    this->engine = e;
    this->opts = popts;

    this->shader = std::make_shared<glt::ShaderProgram>(e->shaderManager());
    this->shader->addShaderSrc(PARTICLE_VERTEX_SHADER,
                               glt::ShaderType::VertexShader);
    this->shader->addShaderSrc(PARTICLE_FRAGMENT_SHADER,
                               glt::ShaderType::FragmentShader);
    this->shader->bindAttributes<Vertex2D>();

    if (!this->shader->tryLink())
        return false;

    this->circle_mesh = std::make_shared<glt::Mesh<Vertex2D>>();
    this->circle_mesh->primType(GL_TRIANGLE_FAN);

    Vertex2D vertex;
    vertex.position = math::vec2(0.f);
    this->circle_mesh->addVertex(vertex);

    const size_t N = 100;
    for (size_t i = 0; i < N; ++i) {
        math::real phi = math::real(i) / (N - 1) * 2 * math::PI;
        math::real x, y;
        math::sincos(phi, x, y);
        vertex.position = math::vec2(x, y);
        this->circle_mesh->addVertex(vertex);
    }

    this->circle_mesh->send();
    return true;
}

void
ParticleRenderer::renderParticles(const ge::Event<ge::RenderEvent> &ev,
                                  size_t n,
                                  const ParticleRenderer::Particle *ps)
{

    glt::TextureSampler particle_sampler(
      std::make_shared<glt::TextureData>(glt::Texture1D));

    particle_sampler.filterMode(glt::TextureSampler::FilterNearest);
    particle_sampler.clampMode(glt::TextureSampler::ClampRepeat);
    particle_sampler.bind(0);
    GL_CALL(glTexImage1D,
            GL_TEXTURE_1D,
            0,
            GL_RGBA32F,
            GLsizei(n * 2),
            0,
            GL_RGBA,
            GL_FLOAT,
            ps);
    ASSERT(this->shader->validate());
    this->shader->use();

    glt::RenderManager &rm = ev.info.engine.renderManager();
    glt::GeometryTransform &gt = rm.geometryTransform();
    auto sp = gt.save();

    gt.scale(math::vec3(math::recip(this->opts.world_size[0]),
                        math::recip(this->opts.world_size[1]),
                        math::real(1)));

    glt::Uniforms us(*this->shader);
    us.optional("mvpMatrix", gt.mvpMatrix());
    us.optional("instanceData", glt::Sampler(particle_sampler, 0));

    this->circle_mesh->drawInstanced(n);
}

#endif
