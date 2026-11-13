#include "ge/Screenshot.hpp"

#include "glt/utils.hpp"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stb_image_write.h>

#include <algorithm>
#include <cstdio>
#include <limits>
#include <vector>

namespace ge {

bool
writeFramebufferPNG(const char *filename, int width, int height)
{
    if (width <= 0 || height <= 0 ||
        width > std::numeric_limits<int>::max() / 3)
        return false;
    const int stride = width * 3;
    if (size_t(height) > std::numeric_limits<size_t>::max() / size_t(stride))
        return false;

    std::vector<unsigned char> pixels(size_t(stride) * size_t(height));

    // Do not inherit a renderer's PBO, row stride, or pixel-pack offsets.
    GLint framebuffer, buffer, alignment, row_length, skip_rows, skip_pixels;
    GL_CALL(glGetIntegerv, GL_READ_FRAMEBUFFER_BINDING, &framebuffer);
    GL_CALL(glGetIntegerv, GL_PIXEL_PACK_BUFFER_BINDING, &buffer);
    GL_CALL(glGetIntegerv, GL_PACK_ALIGNMENT, &alignment);
    GL_CALL(glGetIntegerv, GL_PACK_ROW_LENGTH, &row_length);
    GL_CALL(glGetIntegerv, GL_PACK_SKIP_ROWS, &skip_rows);
    GL_CALL(glGetIntegerv, GL_PACK_SKIP_PIXELS, &skip_pixels);

    GL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, 0);
    GLint read_buffer;
    GL_CALL(glGetIntegerv, GL_READ_BUFFER, &read_buffer);
    GL_CALL(glReadBuffer, GL_BACK);
    GL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, 0);
    GL_CALL(glPixelStorei, GL_PACK_ALIGNMENT, 1);
    GL_CALL(glPixelStorei, GL_PACK_ROW_LENGTH, 0);
    GL_CALL(glPixelStorei, GL_PACK_SKIP_ROWS, 0);
    GL_CALL(glPixelStorei, GL_PACK_SKIP_PIXELS, 0);

    // Read synchronously and check the error ourselves: GL_CALL consumes it
    // in debug builds, but a failed read must not produce a bogus screenshot.
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    const GLenum error = glGetError();

    GL_CALL(glPixelStorei, GL_PACK_ALIGNMENT, alignment);
    GL_CALL(glPixelStorei, GL_PACK_ROW_LENGTH, row_length);
    GL_CALL(glPixelStorei, GL_PACK_SKIP_ROWS, skip_rows);
    GL_CALL(glPixelStorei, GL_PACK_SKIP_PIXELS, skip_pixels);
    GL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, GLuint(buffer));
    GL_CALL(glReadBuffer, GLenum(read_buffer));
    GL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, GLuint(framebuffer));
    if (error != GL_NO_ERROR)
        return false;

    // OpenGL starts at the bottom left; PNG rows start at the top left.
    for (int y = 0; y < height / 2; ++y) {
        auto *top = pixels.data() + size_t(y) * size_t(stride);
        auto *bottom = pixels.data() + size_t(height - 1 - y) * size_t(stride);
        std::swap_ranges(top, top + stride, bottom);
    }

    auto *file = std::fopen(filename, "wb");
    if (!file)
        return false;
    const auto write = [](void *context, void *data, int size) {
        std::fwrite(data, 1, size_t(size), static_cast<std::FILE *>(context));
    };
    const bool encoded = stbi_write_png_to_func(
      write, file, width, height, 3, pixels.data(), stride) != 0;
    const bool written = std::ferror(file) == 0;
    const bool closed = std::fclose(file) == 0;
    return encoded && written && closed;
}

} // namespace ge
