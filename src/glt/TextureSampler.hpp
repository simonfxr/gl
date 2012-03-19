#ifndef GLT_TEXTURE_SAMPLER_HPP
#define GLT_TEXTURE_SAMPLER_HPP

#include "opengl.hpp"
#include "data/Ref.hpp"
#include "glt/TextureData.hpp"

namespace glt {

using namespace defs;

struct TextureSampler {
private:
    Ref<TextureData> _data;
    GLuint _sampler;

public:

    enum FilterMode {
        FilterNearest,
        FilterLinear
    };

    enum Filter {
        FilterMin = 1,
        FilterMag = 2,
    };

    enum ClampMode {
        ClampToEdge,
        ClampRepeat
    };

    enum Axis {
        S = 1,
        T = 2,
        R = 4
    };

    TextureSampler() : _data(new TextureData), _sampler(0) {}
    TextureSampler(const Ref<TextureData>& data) : _data(data), _sampler(0) {}
    ~TextureSampler();

    void free();

    GLuint sampler() const { return _sampler; }
    const Ref<TextureData>& data() const { return _data; }
    Ref<TextureData>& data() { return _data; }
    GLuint ensureSampler();

    void filterMode(FilterMode, Filter filter = Filter(FilterMin | FilterMag));
    void clampMode(ClampMode, Axis axis = Axis(S | T | R));

    void bind(uint32 idx, bool set_active_idx = true);
    void unbind(uint32 idx, bool set_active_idx = true);

    static Axis availableAxes(TextureType);

private:
    TextureSampler(const TextureSampler&);
    TextureSampler& operator =(const TextureSampler&);
};

DEF_ENUM_BITOR(TextureSampler::FilterMode);
DEF_ENUM_BITOR(TextureSampler::Axis);

} // namespace glt

#endif
