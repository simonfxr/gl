#ifndef GLT_IMAGEVIEW_HPP
#define GLT_IMAGEVIEW_HPP

#include "glt/conf.hpp"

#include "glt/Image.hpp"
#include "glt/color.hpp"

namespace glt {

template<ImageFormat>
struct valid_image_format : std::false_type
{};

template<ImageFormat Format>
struct ImageView2D;

template<ImageFormat Format>
struct PixelRef
{
    using view = ImageView2D<Format>;
    std::byte *location;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    inline constexpr explicit(false) operator color() const noexcept;
    inline constexpr PixelRef &operator=(color color);

    constexpr color operator*() const noexcept
    {
        return static_cast<color>(*this);
    }
};

template<ImageFormat Format>
struct ImageView2D
{
    using reference = PixelRef<Format>;

    math::ivec2 _dim;
    std::byte *_data;

    constexpr glt::color operator[](math::ivec2 ij) const
    {
        return load(_data + offset(ij));
    }

    constexpr glt::color operator()(int i, int j) const
    {
        return (*this)[math::ivec2(i, j)];
    }

    constexpr reference operator[](math::ivec2 ij)
    {
        return { _data + offset(ij) };
    }

    constexpr reference operator()(int i, int j)
    {
        return (*this)[math::ivec2(i, j)];
    }

    static constexpr void storePixel(std::byte *p, color col)
    {
        auto rgba = col.u8vec4().cast<std::byte>();
        if constexpr (Format == ImageFormat::RGB) {
            memcpy(p, rgba.data(), 3);
        } else if constexpr (Format == ImageFormat::RGBA) {
            memcpy(p, rgba.data(), 4);
        } else {
            static_assert(valid_image_format<Format>::value);
        }
    }

    static constexpr color loadPixel(std::byte *p)
    {
        math::vec<std::byte, 4> rgba{};
        if constexpr (Format == ImageFormat::RGB) {
            memcpy(rgba.data(), p, 3);
            rgba[3] = static_cast<std::byte>(0xFF);
        } else if constexpr (Format == ImageFormat::RGBA) {
            memcpy(rgba.data(), p, 4);
        } else {
            static_assert(valid_image_format<Format>::value);
        }
        return glt::color(rgba.cast<uint8_t>());
    }

    [[nodiscard]] constexpr math::ivec2 dim() const { return _dim; }

    [[nodiscard]] constexpr bool empty() const { return !width() || !heigth(); }

    [[nodiscard]] constexpr int width() const { return dim()[0]; }

    [[nodiscard]] constexpr int heigth() const { return dim()[1]; }

    [[nodiscard]] constexpr size_t pixelSize() const
    {
        return Image::pixelSize(format());
    }

    [[nodiscard]] constexpr size_t size_bytes() const
    {
        return static_cast<size_t>(width() * heigth()) * pixelSize(format());
    }

    [[nodiscard]] constexpr ImageFormat format() const { return Format; }

    [[nodiscard]] const std::byte *data() const { return _data; }

    [[nodiscard]] std::byte *data() { return _data; }

private:
    [[nodiscard]] constexpr size_t offset(math::ivec2 ij) const
    {
        constexpr auto pixel_size = Image::pixelSize(Format);
        return (ij[0] * _dim[0] + ij[1]) * pixel_size;
    }
};

template<ImageFormat Format>
constexpr PixelRef<Format>::operator color() const noexcept
{
    return view::loadPixel(location);
};

template<ImageFormat Format>
constexpr PixelRef<Format> &
PixelRef<Format>::operator=(color color)
{
    view::storePixel(location, color);
    return *this;
}

} // namespace glt

#endif // GLT_IMAGEVIEW_HPP
