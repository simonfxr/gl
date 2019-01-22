#ifndef GLT_TEXTURE_SAMPLER_HPP
#define GLT_TEXTURE_SAMPLER_HPP

#include "bl/shared_ptr.hpp"
#include "glt/GLObject.hpp"
#include "glt/TextureData.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"
#include "util/enum_flags.hpp"

namespace glt {

struct GLT_API TextureSampler
{
private:
    bl::shared_ptr<TextureData> _data;
    GLSamplerObject _sampler;

public:
    enum FilterMode : uint8_t
    {
        FilterNearest,
        FilterLinear
    };

    enum Filter : uint8_t
    {
        FilterMin = 1,
        FilterMag = 2
    };

    enum ClampMode : uint8_t
    {
        ClampToEdge,
        ClampRepeat
    };

    enum Axis : uint8_t
    {
        S = 1,
        T = 2,
        R = 4
    };

    TextureSampler() : _data(new TextureData), _sampler(0) {}
    explicit TextureSampler(const bl::shared_ptr<TextureData> &data)
      : _data(data), _sampler(0)
    {}
    ~TextureSampler();

    void free();

    const GLSamplerObject &sampler() const { return _sampler; }
    const bl::shared_ptr<TextureData> &data() const { return _data; }
    bl::shared_ptr<TextureData> &data() { return _data; }
    GLSamplerObject &ensureSampler();

    void filterMode(FilterMode, Filter filter = Filter(FilterMin | FilterMag));
    void clampMode(ClampMode, Axis axis = Axis(S | T | R));

    void bind(uint32_t idx, bool set_active_idx = true);
    void unbind(uint32_t idx, bool set_active_idx = true);

    static Axis availableAxes(TextureType);

    TextureSampler(const TextureSampler &) = delete;
    TextureSampler &operator=(const TextureSampler &) = delete;
};

DEF_ENUM_BIT_OPS(TextureSampler::FilterMode);
DEF_ENUM_BIT_OPS(TextureSampler::Axis);

} // namespace glt

#endif
