#include "ge/Anim.hpp"
#include "ge/Command.hpp"
#include "ge/Event.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/ImageView.hpp"
#include "glt/Mesh.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"
#include "math/fvec.hpp"
#include "math/real.hpp"

using namespace math;

DEF_GL_MAPPED_TYPE(Vertex, (vec2, position))

static const uint32_t W = 64;
static const uint32_t H = 64;
static constexpr glt::ImageFormat IMGFMT = glt::ImageFormat::RGBA;

struct Anim final : ge::Anim<Anim>
{
    glt::CubeMesh<Vertex> quadBatch;
    glt::TextureSampler worldTexture;
    glt::Image world;

    void init(const ge::Event<ge::InitEvent> &);
    void render(const ge::Event<ge::RenderEvent> &) override;
    void animate(const ge::Event<ge::AnimationEvent> &) override;
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    {
        Vertex v{};
        v.position = vec2(-1.f, -1.f);
        quadBatch.add(v);
        v.position = vec2(1.f, -1.f);
        quadBatch.add(v);
        v.position = vec2(1.f, 1.f);
        quadBatch.add(v);
        v.position = vec2(-1.f, 1.f);
        quadBatch.add(v);

        quadBatch.send();
    }

    GL_CALL(glDisable, GL_DEPTH_TEST);

    world = glt::Image(ivec2(W, H), IMGFMT);

    ev.info.success = true;
}

void
Anim::animate(const ge::Event<ge::AnimationEvent> &)
{
    std::memset(world.data(), 255, world.size_bytes());
    auto w = glt::ImageView2D<IMGFMT>{ world.dim(), world.data() };
    w[ivec2(0, 0)] = glt::color(0xFF, 0, 0);
}

void
Anim::render(const ge::Event<ge::RenderEvent> &ev)
{
    auto &e = ev.info.engine;
    auto time = e.gameLoop().tickTime() +
                ev.info.interpolation * e.gameLoop().tickDuration();

    auto renderShader = e.shaderManager().program("render");
    ASSERT(renderShader);

    worldTexture.bind(0);

    renderShader->use();
    glt::Uniforms(*renderShader)
      .optional("world", glt::BoundTexture(GL_SAMPLER_2D, 0))
      .optional("time", time);

    quadBatch.draw();

    worldTexture.unbind(0);
}

int
main(int argc, char *argv[])
{
    Anim anim;
    anim.engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    return anim.main(argc, argv);
}
