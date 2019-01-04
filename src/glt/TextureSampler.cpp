#include "glt/TextureSampler.hpp"
#include "glt/utils.hpp"

namespace glt {

TextureSampler::~TextureSampler()
{
    free();
}

GLSamplerObject &
TextureSampler::ensureSampler()
{
    return _sampler.ensure();
}

void
TextureSampler::free()
{
    _sampler.release();
}

void
TextureSampler::filterMode(FilterMode mode, Filter filter)
{
    ensureSampler();

    GLint glmode;
    switch (mode) {
    case FilterNearest:
        glmode = GL_NEAREST;
        break;
    case FilterLinear:
        glmode = GL_LINEAR;
        break;
    }

    if ((int(filter) & FilterMin) != 0)
        GL_CALL(glSamplerParameteri, *_sampler, GL_TEXTURE_MIN_FILTER, glmode);
    if ((int(filter) & FilterMag) != 0)
        GL_CALL(glSamplerParameteri, *_sampler, GL_TEXTURE_MAG_FILTER, glmode);
}

void
TextureSampler::clampMode(ClampMode mode, Axis axis)
{
    ensureSampler();

    GLint glmode;
    switch (mode) {
    case ClampToEdge:
        glmode = GL_CLAMP_TO_EDGE;
        break;
    case ClampRepeat:
        glmode = GL_REPEAT;
        break;
    }

    axis = Axis(int(axis) & availableAxes(_data->type()));

    if ((int(axis) & S))
        GL_CALL(glSamplerParameteri, *_sampler, GL_TEXTURE_WRAP_S, glmode);
    if ((int(axis) & T))
        GL_CALL(glSamplerParameteri, *_sampler, GL_TEXTURE_WRAP_T, glmode);
    if ((int(axis) & R))
        GL_CALL(glSamplerParameteri, *_sampler, GL_TEXTURE_WRAP_R, glmode);
}

void
TextureSampler::bind(uint32_t idx, bool set_active_idx)
{
    ensureSampler();
    GL_CALL(glBindSampler, idx, *_sampler);
    _data->bind(idx, set_active_idx);
}

void
TextureSampler::unbind(uint32_t idx, bool set_active_idx)
{
    GL_CALL(glBindSampler, idx, 0);
    _data->unbind(idx, set_active_idx);
}

TextureSampler::Axis
TextureSampler::availableAxes(TextureType t)
{
    switch (t) {
    case Texture1D:
        return S;
    case Texture2D:
        return S | T;
    case Texture3D:
        return S | T | R;
    }
    CASE_UNREACHABLE;
}

} // namespace glt
