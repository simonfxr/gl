#include "glt/TextureSampler.hpp"
#include "glt/utils.hpp"

namespace glt {

TextureSampler::~TextureSampler() {
    free();
}

GLuint TextureSampler::ensureSampler() {
    if (_sampler == 0)
        GL_CHECK(glGenSamplers(1, &_sampler));
    return _sampler;
}

void TextureSampler::free() {
    if (_sampler != 0) {
        GL_CHECK(glDeleteSamplers(1, &_sampler));
        _sampler = 0;
    }
}

void TextureSampler::filterMode(FilterMode mode, Filter filter) {
    ensureSampler();

    GLint glmode;
    switch (mode) {
    case FilterNearest: glmode = GL_NEAREST; break;
    case FilterLinear: glmode = GL_LINEAR; break;
    default: ASSERT_FAIL_MSG("invalid FilterMode");
    }

    if ((int(filter) & FilterMin) != 0)
        GL_CHECK(glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, glmode));
    if ((int(filter) & FilterMag) != 0)
        GL_CHECK(glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, glmode));
}

void TextureSampler::clampMode(ClampMode mode, Axis axis) {
    ensureSampler();

    GLint glmode;
    switch (mode) {
    case ClampToEdge: glmode = GL_CLAMP_TO_EDGE; break;
    case ClampRepeat: glmode = GL_REPEAT; break;
    default: ASSERT_FAIL_MSG("invalid ClampMode");
    }

    axis = Axis(int(axis) & availableAxes(_data->type()));

    if ((int(axis) & S))
        GL_CHECK(glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, glmode));
    if ((int(axis) & T))
        GL_CHECK(glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, glmode));
    if ((int(axis) & R))
        GL_CHECK(glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_R, glmode));
}

void TextureSampler::bind(uint32 idx, bool set_active_idx) {
    ensureSampler();
    GL_CHECK(glBindSampler(idx, _sampler));
    _data->bind(idx, set_active_idx);
}

void TextureSampler::unbind(uint32 idx, bool set_active_idx) {
    GL_CHECK(glBindSampler(idx, 0));
    _data->unbind(idx, set_active_idx);
}

TextureSampler::Axis TextureSampler::availableAxes(TextureType t) {
    switch (t) {
    case Texture1D: return S;
    case Texture2D: return S | T;
    case Texture3D: return S | T | R;
    default: ASSERT_FAIL_MSG("invalid texture type");
    }
}

} // namespace glt
