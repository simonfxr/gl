#ifndef SIM_UTILS_HPP
#define SIM_UTILS_HPP 1

#include "ge/Engine.hpp"
#include "math/vec2.hpp"
#include "glt/color.hpp"

#include "glt/Mesh.hpp"
#include "glt/utils.hpp"

struct Vertex2D {
    math::vec2_t position;
};

struct ParticleRenderer {

    struct Opts {
        vec2_t world_size;
        Opts() {}
    };
    
    struct Particle {
        math::vec2_t position;
        math::real radius;
        glt::color color;
    };

    bool init(ge::Engine*, const Opts&);
    void renderParticles(const ge::Event<ge::RenderEvent>&, defs::size, const Particle *);

private:
    ge::Engine *engine;
    Opts opts;
    Ref<glt::Mesh<Vertex2D>> circle_mesh;
    Ref<glt::ShaderProgram> shader;
};

DEFINE_VERTEX_DESC(Vertex2D, VERTEX_ATTR(Vertex2D, position));

#define DEF_SHADER(name, ...) const std::string name = AS_STRING(__VA_ARGS__);

DEF_SHADER(PARTICLE_VERTEX_SHADER,
  uniform mat4 mvpMatrix;
  in vec2 position;
  void main() {
      gl_Position = mvpMatrix * vec4(position, 0, 1);
  } 
);

DEF_SHADER(PARTICLE_FRAGMENT_SHADER,
  uniform vec4 color;
  out vec4 fragColor;
  void main() {
      fragColor = color;
  } 
);

#undef DEF_SHADER

bool ParticleRenderer::init(ge::Engine *e, const ParticleRenderer::Opts& opts) {

    this->engine = e;
    this->opts = opts;

    this->shader = makeRef(new glt::ShaderProgram(e->shaderManager()));
    this->shader->addShaderSrc(PARTICLE_VERTEX_SHADER, glt::ShaderManager::VertexShader);
    this->shader->addShaderSrc(PARTICLE_FRAGMENT_SHADER, glt::ShaderManager::FragmentShader);
    this->shader->bindAttributes<Vertex2D>();

    if (!this->shader->tryLink())
        return false;
    
    this->circle_mesh = makeRef(new glt::Mesh<Vertex2D>());
    this->circle_mesh->primType(GL_TRIANGLE_FAN);


    Vertex2D vertex;
    vertex.position = math::vec2(0.f);
    this->circle_mesh->addVertex(vertex);
    
    const defs::size N = 100;    
    for (defs::index i = 0; i < N; ++i) {
        math::real phi = math::real(i) / (N - 1) * 2 * math::PI;
        real x, y;
        math::sincos(phi, x, y);
        vertex.position = math::vec2(x, y);
        this->circle_mesh->addVertex(vertex);
    }

    this->circle_mesh->send();
    return true;
}

void ParticleRenderer::renderParticles(const ge::Event<ge::RenderEvent>& ev, defs::size n, const ParticleRenderer::Particle *ps) {

    ASSERT(this->shader->validate());
    this->shader->use();

    glt::RenderManager& rm = ev.info.engine.renderManager();
    glt::GeometryTransform& gt = rm.geometryTransform();
    

    for (defs::index i = 0; i < n; ++i) {
        const Particle& p = ps[i];
        glt::SavePoint sp(gt.save());

        vec2_t orig = p.position / this->opts.world_size;
        gt.translate(vec3(orig[0], orig[1], 0));
        gt.scale(math::vec3(p.radius));

        glt::Uniforms us(*this->shader);
        us.optional("mvpMatrix", gt.mvpMatrix());
        us.optional("color", p.color);

        this->circle_mesh->draw();        
    }
}

#endif
