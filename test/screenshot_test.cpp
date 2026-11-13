#include "ge/Screenshot.hpp"
#include "opengl.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stb_image_write.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int width = 7, height = 5;
constexpr std::array<GLenum, 8> state_names = {
    GL_READ_FRAMEBUFFER_BINDING, GL_DRAW_FRAMEBUFFER_BINDING,
    GL_READ_BUFFER, GL_PIXEL_PACK_BUFFER_BINDING, GL_PACK_ALIGNMENT,
    GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS
};

bool
check(bool passed, const char *message)
{
    if (!passed)
        std::fprintf(stderr, "screenshot_test: %s\n", message);
    return passed;
}

std::array<GLint, state_names.size()>
readState()
{
    std::array<GLint, state_names.size()> state{};
    for (size_t i = 0; i < state.size(); ++i)
        glGetIntegerv(state_names[i], &state[i]);
    return state;
}

bool
run(const std::filesystem::path &directory)
{
    // An odd-sized, asymmetric region exposes RGB row padding and vertical flips.
    // Leave the rest of the default back buffer black.
    glDrawBuffer(GL_BACK);
    glDisable(GL_DITHER);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    const std::array<std::array<float, 3>, 4> colors = {{
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}
    }};
    for (int i = 0; i < 4; ++i) {
        const bool right = (i % 2) != 0, top = i >= 2;
        glScissor(right ? 2 : 0, top ? 3 : 0,
                  right ? width - 2 : 2, top ? height - 3 : 3);
        glClearColor(colors[i][0], colors[i][1], colors[i][2], 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);

    // The helper must read the default back buffer, not this complete read FBO,
    // and must restore both FBOs' independent read-buffer selections.
    glReadBuffer(GL_FRONT);
    GLuint framebuffer = 0, renderbuffer = 0, pbo = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, renderbuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    if (!check(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) ==
                 GL_FRAMEBUFFER_COMPLETE, "incomplete test read framebuffer"))
        return false;

    std::array<unsigned char, 2048> sentinel;
    sentinel.fill(0xa5);
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, sentinel.size(), sentinel.data(),
                 GL_STATIC_READ);
    glPixelStorei(GL_PACK_ALIGNMENT, 8);
    glPixelStorei(GL_PACK_ROW_LENGTH, width + 7);
    glPixelStorei(GL_PACK_SKIP_ROWS, 2);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 3);
    const auto expected_state = readState();
    if (!check(glGetError() == GL_NO_ERROR, "GL error during test setup"))
        return false;

    const auto filename = (directory / "capture.png").string();
    bool passed = check(ge::writeFramebufferPNG(filename.c_str(), width, height),
                        "capture failed");
    passed &= check(readState() == expected_state, "capture changed GL state");

    // Encode independently constructed top-down pixels with the same
    // deterministic encoder, then compare the complete PNG. This tests capture
    // dimensions, channels, row packing, colors and orientation without adding
    // a PNG decoder dependency.
    std::array<unsigned char, width * height * 3> expected_pixels{};
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const int quadrant = (height - 1 - y >= 3 ? 2 : 0) +
                                 (x >= 2 ? 1 : 0);
            for (int c = 0; c < 3; ++c)
                expected_pixels[(y * width + x) * 3 + c] =
                  static_cast<unsigned char>(colors[quadrant][c] * 255);
        }
    std::vector<unsigned char> expected_png;
    const auto append = [](void *context, void *data, int size) {
        auto &bytes = *static_cast<std::vector<unsigned char> *>(context);
        auto *begin = static_cast<unsigned char *>(data);
        bytes.insert(bytes.end(), begin, begin + size);
    };
    passed &= check(stbi_write_png_to_func(append, &expected_png, width, height,
                                           3, expected_pixels.data(), width * 3)
                      != 0,
                    "could not encode expected PNG");
    std::ifstream input(filename, std::ios::binary);
    const std::vector<unsigned char> actual_png{
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()
    };
    passed &= check(actual_png == expected_png,
                    "incorrect PNG dimensions, colors or orientation");

    for (const auto &size : std::array<std::array<int, 2>, 5>{{
             {0, height}, {width, 0}, {-1, height}, {width, -1},
             {std::numeric_limits<int>::max(), height}
         }}) {
        passed &= check(!ge::writeFramebufferPNG(filename.c_str(), size[0],
                                                 size[1]),
                        "invalid dimensions accepted");
        passed &= check(readState() == expected_state,
                        "invalid dimensions changed GL state");
    }
    // A missing parent is unwritable even when the test runs as root.
    const auto unwritable = (directory / "missing" / "capture.png").string();
    passed &= check(!ge::writeFramebufferPNG(unwritable.c_str(), width, height),
                    "unwritable output path accepted");
    passed &= check(readState() == expected_state,
                    "failed output changed GL state");

    std::array<unsigned char, 2048> actual{};
    glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, actual.size(), actual.data());
    passed &= check(actual == sentinel, "capture modified caller's pack buffer");
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    GLint default_read_buffer = 0;
    glGetIntegerv(GL_READ_BUFFER, &default_read_buffer);
    passed &= check(default_read_buffer == GL_FRONT,
                    "capture changed default framebuffer's read buffer");
    passed &= check(glGetError() == GL_NO_ERROR, "GL error after capture");

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers(1, &pbo);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &renderbuffer);
    return passed;
}

} // namespace

int
main()
{
    glfwSetErrorCallback([](int, const char *message) {
        std::fprintf(stderr, "GLFW: %s\n", message);
    });
    if (!check(glfwInit() != 0, "GLFW init failed (requires a display)"))
        return 1;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    auto *window = glfwCreateWindow(64, 64, "Screenshot regression", nullptr,
                                    nullptr);
    if (!check(window != nullptr, "window creation failed")) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    bool passed = check(gladLoadGL(glfwGetProcAddress) != 0, "GLAD init failed");
    int framebuffer_width = 0, framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    passed &= check(framebuffer_width >= width && framebuffer_height >= height,
                    "window framebuffer too small");

    std::error_code error;
    auto directory = std::filesystem::temp_directory_path(error);
    passed &= check(!error, "could not locate temporary directory");
    directory /= "gl-screenshot-test-" + std::to_string(
      std::chrono::steady_clock::now().time_since_epoch().count());
    const bool created = !error && std::filesystem::create_directory(directory,
                                                                     error);
    passed &= check(created, "could not create temporary directory");
    if (passed)
        passed = run(directory);
    if (created) {
        std::filesystem::remove_all(directory, error);
        passed &= check(!error, "could not remove temporary files");
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    if (passed) {
        std::puts("screenshot_test: passed");
        std::fflush(stdout);
    }
    return passed ? 0 : 1;
}
