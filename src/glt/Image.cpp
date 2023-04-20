#include "glt/Image.hpp"

namespace glt {

Image::Image(math::ivec2 dim, ImageFormat format) : _dim(dim), _format(format)
{
    allocBytes(size_bytes());
}

Image::Image(math::ivec2 dim,
             ImageFormat format,
             std::shared_ptr<std::byte[]> data)
  : _dim(dim), _format(format), _data(bl::move(data))
{
    if (empty())
        *this = Image{};
}

Image::Image(math::ivec2 dim, ImageFormat format, std::byte *data)
  : Image(dim,
          format,
          std::unique_ptr<std::byte, noop_deleter>{ data, noop_deleter{} })
{
    if (empty())
        *this = Image{};
}

Image::~Image() = default;

} // namespace glt
