#ifndef GE_SCREENSHOT_HPP
#define GE_SCREENSHOT_HPP

#include "ge/conf.hpp"

namespace ge {

// Capture the default framebuffer's back buffer, before swapping buffers.
// Width and height are framebuffer pixels, not logical window coordinates.
GE_API bool writeFramebufferPNG(const char *filename, int width, int height);

} // namespace ge

#endif
