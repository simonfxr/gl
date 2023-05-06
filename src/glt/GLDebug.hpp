#ifndef GLT_GL_DEBUG_HPP
#define GLT_GL_DEBUG_HPP

#include "err/err.hpp"
#include "glt/conf.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"

#include <bl/noncopymove.hpp>

namespace glt {

BL_DEF_TAG_CPO(printGLDebugMessages);
BL_DEF_TAG_CPO(ignoreGLDebugMessage);

using GLDebug = bl::any_unique_t<
  bl::overload<bool(bl::self &, const err::Location &)>(printGLDebugMessages),
  bl::overload<void(bl::self &, OpenGLVendor, GLuint)>(ignoreGLDebugMessage)>;

GLDebug
makeGLDebugFallback();

GLDebug
makeGLDebug();

} // namespace glt

#endif
