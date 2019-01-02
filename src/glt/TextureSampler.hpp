#ifndef GLT_TEXTURE_SAMPLER_HPP
#define GLT_TEXTURE_SAMPLER_HPP

#include "glt/GLObject.hpp"
#include "glt/TextureData.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"

#include <memory>

namespace glt {

using namespace defs;

struct GLT_API TextureSampler
{
private:
    std::shared_ptr<TextureData> _data;
    GLSamplerObject _sampler;

public:
    enum FilterMode
    {
        FilterNearest,
        FilterLinear
    };

    enum Filter
    {
        FilterMin = 1,
        FilterMag = 2
    };

    enum ClampMode
    {
        ClampToEdge,
        ClampRepeat
    };

    enum Axis
    {
        S = 1,
        T = 2,
        R = 4
    };

    TextureSampler() : _data(new TextureData), _sampler(0) {}
    explicit TextureSampler(const std::shared_ptr<TextureData> &data)
      : _data(data), _sampler(0)
    {}
    ~TextureSampler();

    void free();

    const GLSamplerObject &sampler() const { return _sampler; }
    const std::shared_ptr<TextureData> &data() const { return _data; }
    std::shared_ptr<TextureData> &data() { return _data; }
    GLSamplerObject &ensureSampler();

    void filterMode(FilterMode, Filter filter = Filter(FilterMin | FilterMag));
    void clampMode(ClampMode, Axis axis = Axis(S | T | R));

    void bind(uint32 idx, bool set_active_idx = true);
    void unbind(uint32 idx, bool set_active_idx = true);

    static Axis availableAxes(TextureType);

    TextureSampler(const TextureSampler &) = delete;
    TextureSampler &operator=(const TextureSampler &) = delete;
};

DEF_ENUM_BITOR(TextureSampler::FilterMode);
DEF_ENUM_BITOR(TextureSampler::Axis);

} // namespace glt

#endif
