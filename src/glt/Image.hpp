#ifndef GLT_IMAGE_HPP
#define GLT_IMAGE_HPP

#include "glt/conf.hpp"

#include "err/err.hpp"
#include "math/ivec.hpp"

#include <bl/ptr.hpp>

#include <functional>
#include <memory>

namespace glt {

enum class ImageFormat
{
    RGBA,
    RGB,
};

struct GLT_API Image
{
    Image() = default;

    explicit Image(math::ivec2 dim) : Image(dim, ImageFormat::RGBA) {}

    Image(math::ivec2 dim, ImageFormat format);

    Image(math::ivec2 dim,
          ImageFormat format,
          std::shared_ptr<bl::byte[]> data);

    Image(math::ivec2 dim, ImageFormat format, bl::byte *data);

    template<typename Elem, typename Deleter>
    Image(math::ivec2 dim,
          ImageFormat format,
          bl::owned_ptr<Elem, Deleter> data)
      : _dim(dim), _format(format)
    {
        if (empty()) {
            *this = Image{};
            return;
        }
        _data = data_ptr{ data.get(), [p = bl::move(data)](bl::byte *) mutable {
                             auto drop = bl_mov(p);
                             (void) drop;
                         } };
    }

    Image(Image &&img) noexcept { bl::swap(*this, img); }

    Image(const Image &img) = delete;

    Image &operator=(const Image &img) = delete;

    Image &operator=(Image &&img) noexcept
    {
        auto tmp = bl::move(img);
        bl::swap(*this, tmp);
        return *this;
    }

    friend void swap(Image &lhs, Image &rhs)
    {
        using std::swap;
        swap(lhs._dim, rhs._dim);
        swap(lhs._format, rhs._format);
        swap(lhs._data, rhs._data);
    }

    ~Image();

    [[nodiscard]] constexpr math::ivec2 dim() const { return _dim; }

    [[nodiscard]] constexpr bool empty() const { return !width() || !heigth(); }

    [[nodiscard]] constexpr int width() const { return dim()[0]; }

    [[nodiscard]] constexpr int heigth() const { return dim()[1]; }

    [[nodiscard]] constexpr size_t pixelSize() const
    {
        return pixelSize(format());
    }

    [[nodiscard]] constexpr size_t size_bytes() const
    {
        return static_cast<size_t>(width() * heigth()) * pixelSize(format());
    }

    [[nodiscard]] constexpr ImageFormat format() const { return _format; }

    [[nodiscard]] const bl::byte *data() const { return _data.get(); }

    [[nodiscard]] bl::byte *data() { return _data.get(); }

    static constexpr size_t pixelSize(ImageFormat format)
    {
        switch (format) {
        case ImageFormat::RGBA:
            return 4;
        case ImageFormat::RGB:
            return 3;
        }
        UNREACHABLE;
    }

private:
    using data_ptr = std::shared_ptr<bl::byte[]>;
    using noop_deleter = decltype([](bl::byte *) { /* noop */ });

    void allocBytes(size_t n) { _data = std::make_shared<bl::byte[]>(n); }

    math::ivec2 _dim{ 0, 0 };
    ImageFormat _format{};
    data_ptr _data;
};
} // namespace glt

#endif // GLT_IMAGE_HPP
