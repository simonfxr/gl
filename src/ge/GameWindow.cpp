#include "ge/GameWindow.hpp"

#define GLFW_INCLUDE_NONE 1

#include "err/err.hpp"
#include "ge/Event.hpp"
#include "ge/module.hpp"
#include "glt/utils.hpp"
#include "math/real.hpp"
#include "opengl.hpp"
#include "util/string.hpp"

#include <bl/vec.hpp>

#include <GLFW/glfw3.h>
#include <limits>

using namespace bl::literals;

namespace ge {

struct GameWindow::Impl : GameWindow
{
    GLFWwindow *win;
    double mouse_x{};
    double mouse_y{};
    std::shared_ptr<WindowRenderTarget> renderTarget{};
    WindowEvents events;
    GLContextInfo context_info;
    bool show_mouse_cursor{ true };
    bool accum_mouse_moves{ false };
    const bool owning_win;
    bool have_focus{ true };
    bool vsync{ false };

    Impl(bool owns_win, GLFWwindow *rw) : win(rw), owning_win(owns_win) {}

    void init(const WindowOptions &opts);
    static void handleInputEvents();
    void setMouse(int16_t x, int16_t y);

    static GLFWwindow *makeWindow(const WindowOptions &opts);
    static Impl *getUserPointer(GLFWwindow * /*w*/);
    static void glfw_window_size_callback(GLFWwindow *win, int w, int h);
    static void glfw_window_close_callback(GLFWwindow *win);
    static void glfw_window_refresh_callback(GLFWwindow *win);
    static void glfw_window_focus_callback(GLFWwindow *win, int focus_gained);
    static void glfw_window_pos_callback(GLFWwindow *win, int x, int y);
    static void glfw_window_iconify_callback(GLFWwindow *win, int iconified);
    static void glfw_framebuffer_size_callback(GLFWwindow *win, int w, int h);
    static void glfw_cursor_pos_callback(GLFWwindow *win, double x, double y);
    static void glfw_key_callback(GLFWwindow *win,
                                  int key,
                                  int scancode,
                                  int action,
                                  int mods);
    static void glfw_mouse_button_callback(GLFWwindow *win,
                                           int button,
                                           int action,
                                           int mods);
    static void glfw_mouse_scroll_callback(GLFWwindow *win,
                                           double xoffset,
                                           double yoffset);

    ~Impl()
    {
        renderTarget.reset();
        glfwMakeContextCurrent(nullptr);
        if (owning_win)
            glfwDestroyWindow(win);
    }
};

BL_DEFINE_RUNTIME_SIZED(GameWindow, GameWindow::Impl);

namespace {

void
glfw_error_callback(int error, const char *desc)
{
    UNUSED(error);
    ERR(scat("GLFW error: "_sv, bl::from_cstring(desc)));
}

void
resizeRenderTarget(const Event<WindowResized> &e)
{
    e.info.window.renderTarget()->resized();
}

KeyCode
convertGLFWKey(int key)
{

    if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z)
        return KeyCode(key - GLFW_KEY_A + static_cast<int>(KeyCode::A));
    if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9)
        return KeyCode(key - GLFW_KEY_0 + static_cast<int>(KeyCode::Num0));
    if (GLFW_KEY_KP_0 <= key && key <= GLFW_KEY_KP_9)
        return KeyCode(key - GLFW_KEY_KP_0 +
                       static_cast<int>(KeyCode::Numpad0));
    if (GLFW_KEY_F1 <= key && key <= GLFW_KEY_F15)
        return KeyCode(key - GLFW_KEY_F1 + static_cast<int>(KeyCode::F1));
    if (GLFW_KEY_F16 <= key && key <= GLFW_KEY_F25) {
        ERR("invalid function key > F15 pressed");
        return KeyCode::Space;
    }

#define K(a, b)                                                                \
    case GLFW_KEY_##a:                                                         \
        return KeyCode::b;                                                     \
        break

    switch (key) {
        K(SPACE, Space);
        K(APOSTROPHE, Quote);
        K(COMMA, Comma);
        K(MINUS, Dash);
        K(PERIOD, Period);
        K(SLASH, Slash);
        K(SEMICOLON, SemiColon);
        K(EQUAL, Equal);
        K(LEFT_BRACKET, LBracket);
        K(BACKSLASH, BackSlash);
        K(RIGHT_BRACKET, RBracket);
        K(GRAVE_ACCENT, Tilde); // TODO: add key
        K(WORLD_1, Tilde);      // TODO: add key
        K(WORLD_2, Tilde);      // TODO: add key
        K(ESCAPE, Escape);
        K(ENTER, Return);
        K(TAB, Tab);
        K(BACKSPACE, Back);
        K(INSERT, Insert);
        K(DELETE, Delete);
        K(RIGHT, Right);
        K(LEFT, Left);
        K(DOWN, Down);
        K(UP, Up);
        K(PAGE_UP, PageUp);
        K(PAGE_DOWN, PageDown);
        K(HOME, Home);
        K(END, End);
        K(CAPS_LOCK, Tilde);    // TODO: add key
        K(SCROLL_LOCK, Tilde);  // TODO: add key
        K(NUM_LOCK, Tilde);     // TODO: add key
        K(PRINT_SCREEN, Tilde); // TODO: add key
        K(PAUSE, Pause);
        K(KP_DECIMAL, Period);
        K(KP_DIVIDE, Slash);
        K(KP_MULTIPLY, Multiply);
        K(KP_SUBTRACT, Dash);
        K(KP_ADD, Add);
        K(KP_ENTER, Return);
        K(KP_EQUAL, Equal);
        K(LEFT_SHIFT, LShift);
        K(LEFT_CONTROL, LControl);
        K(LEFT_ALT, LAlt);
        K(LEFT_SUPER, LSystem);
        K(RIGHT_SHIFT, RShift);
        K(RIGHT_CONTROL, RControl);
        K(RIGHT_ALT, RAlt);
        K(RIGHT_SUPER, RSystem);
        K(MENU, Menu);
    default:
        ERR("invalid key");
        return KeyCode::Tilde;
    }

#undef K
}

KeyCode
convertGLFWMouseButton(int button)
{
#define B(a, b)                                                                \
    case GLFW_MOUSE_BUTTON_##a:                                                \
        return KeyCode::b;                                                     \
        break

    switch (button) {
        B(1, MLeft);
        B(2, MRight);
        B(3, MMiddle);
        B(4, MXButton1);
        B(5, MXButton2);
        B(6, MXButton1);
        B(7, MXButton2);
        B(8, MXButton1);
    default:
        ERR("invalid button");
        return KeyCode::MXButton1;
    }
#undef B
}

} // namespace

GLFWwindow *
GameWindow::Impl::makeWindow(const WindowOptions &opts)
{
    glfwWindowHint(GLFW_SAMPLES,
                   static_cast<int>(opts.settings.antialiasingLevel));

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,
                   static_cast<int>(opts.settings.majorVersion));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,
                   static_cast<int>(opts.settings.minorVersion));

    int profile;
    if (opts.settings.majorVersion > 3 ||
        (opts.settings.majorVersion == 3 && opts.settings.minorVersion >= 2))
        profile = opts.settings.coreProfile ? GLFW_OPENGL_CORE_PROFILE
                                            : GLFW_OPENGL_COMPAT_PROFILE;
    else
        profile = GLFW_OPENGL_ANY_PROFILE;
    glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,
                   opts.settings.debugContext ? GL_TRUE : GL_FALSE);

    GLFWwindow *win = glfwCreateWindow(static_cast<int>(opts.width),
                                       static_cast<int>(opts.height),
                                       opts.title.c_str(),
                                       nullptr,
                                       nullptr);
    return win;
}

GameWindow::Impl *
GameWindow::Impl::getUserPointer(GLFWwindow *w)
{
    return static_cast<Impl *>(glfwGetWindowUserPointer(w));
}

void
GameWindow::Impl::glfw_window_size_callback(GLFWwindow *win, int w, int h)
{
    auto *me = getUserPointer(win);
    me->events.windowResized.raise(Event(WindowResized(*me, w, h)));
}

void
GameWindow::Impl::glfw_window_close_callback(GLFWwindow *win)
{
    auto *me = getUserPointer(win);
    me->events.windowClosed.raise(Event(WindowEvent(*me)));
}

void
GameWindow::Impl::glfw_window_refresh_callback(GLFWwindow *win)
{
    UNUSED(win);
    // NOOP
}

void
GameWindow::Impl::glfw_window_focus_callback(GLFWwindow *win, int focus_gained)
{
    auto *me = getUserPointer(win);

    if (focus_gained == GL_TRUE && !me->have_focus) {
        me->have_focus = true;
        me->events.focusChanged.raise(Event(FocusChanged(*me, true)));
    } else if (focus_gained == GL_FALSE && me->have_focus) {
        me->have_focus = false;
        me->events.focusChanged.raise(Event(FocusChanged(*me, false)));
    }
}

void
GameWindow::Impl::glfw_window_pos_callback(GLFWwindow *win, int x, int y)
{
    UNUSED(win);
    UNUSED(x);
    UNUSED(y);
    // NOOP
}

void
GameWindow::Impl::glfw_window_iconify_callback(GLFWwindow *win, int iconified)
{
    UNUSED(win);
    UNUSED(iconified);
    // NOOP
}

void
GameWindow::Impl::glfw_framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    auto *me = getUserPointer(win);
    me->events.framebufferResized.raise(Event(FramebufferResized(*me, w, h)));
}

// static void GameWindow::Impl::glfw_char_callback(GLFWwindow *win, unsigned
// int codepoint) {

// }

// static void GameWindow::Impl::glfw_cursor_enter_callback(GLFWwindow *win, int
// entered) {

// }

void
GameWindow::Impl::glfw_cursor_pos_callback(GLFWwindow *win, double x, double y)
{
    auto *me = getUserPointer(win);

    auto dx = static_cast<int16_t>(-(x - me->mouse_x));
    auto dy = static_cast<int16_t>(y - me->mouse_y);

    me->mouse_x = x;
    me->mouse_y = y;

    if (dx != 0 || dy != 0)
        me->events.mouseMoved.raise(Event(MouseMoved(
          *me, dx, dy, static_cast<int16_t>(x), static_cast<int16_t>(y))));
}

void
GameWindow::Impl::glfw_key_callback(GLFWwindow *win,
                                    int key,
                                    int scancode,
                                    int action,
                                    int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    if (action == GLFW_REPEAT)
        return;

    auto *me = getUserPointer(win);
    auto k = Key(action == GLFW_PRESS ? KeyState::Pressed : KeyState::Released,
                 convertGLFWKey(key));
    me->events.keyChanged.raise(Event(KeyChanged(*me, k)));
}

void
GameWindow::Impl::glfw_mouse_button_callback(GLFWwindow *win,
                                             int button,
                                             int action,
                                             int mods)
{
    UNUSED(mods);
    auto *me = getUserPointer(win);
    auto b = Key(action == GLFW_PRESS ? KeyState::Pressed : KeyState::Released,
                 convertGLFWMouseButton(button));
    me->events.mouseButton.raise(
      Event(MouseButton(*me,
                        static_cast<int16_t>(me->mouse_x),
                        static_cast<int16_t>(me->mouse_y),
                        b)));
}

void
GameWindow::Impl::glfw_mouse_scroll_callback(GLFWwindow *win,
                                             double xoffset,
                                             double yoffset)
{
    UNUSED(win);
    UNUSED(xoffset);
    UNUSED(yoffset);
    // NOOP
}

void
GameWindow::Impl::init(const WindowOptions &opts)
{

    ASSERT(renderTarget == nullptr);
    renderTarget = std::make_shared<WindowRenderTarget>(*this);

    glfwSetWindowUserPointer(win, this);

    glfwSetWindowSizeCallback(win, glfw_window_size_callback);
    glfwSetWindowCloseCallback(win, glfw_window_close_callback);
    glfwSetWindowRefreshCallback(win, glfw_window_refresh_callback);
    glfwSetWindowFocusCallback(win, glfw_window_focus_callback);
    glfwSetWindowPosCallback(win, glfw_window_pos_callback);
    glfwSetWindowIconifyCallback(win, glfw_window_iconify_callback);
    glfwSetFramebufferSizeCallback(win, glfw_framebuffer_size_callback);
    glfwSetCursorPosCallback(win, glfw_cursor_pos_callback);
    glfwSetKeyCallback(win, glfw_key_callback);
    glfwSetMouseButtonCallback(win, glfw_mouse_button_callback);
    glfwSetScrollCallback(win, glfw_mouse_scroll_callback);

    glfwMakeContextCurrent(win);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        FATAL_ERR("GLAD initialization failed");
    GL_CHECK_ERRORS();

    vsync = opts.vsync;
    //    win->setVerticalSyncEnabled(opts.vsync);

    events.windowResized.reg(resizeRenderTarget);

    context_info = opts.settings;
    context_info.majorVersion =
      glfwGetWindowAttrib(win, GLFW_CONTEXT_VERSION_MAJOR);
    context_info.minorVersion =
      glfwGetWindowAttrib(win, GLFW_CONTEXT_VERSION_MINOR);
    context_info.debugContext =
      glfwGetWindowAttrib(win, GLFW_OPENGL_DEBUG_CONTEXT) == GL_TRUE;
    context_info.coreProfile =
      glfwGetWindowAttrib(win, GLFW_OPENGL_PROFILE) == GLFW_OPENGL_CORE_PROFILE;

    if (!glfwExtensionSupported("GL_ARB_multisample")) {
        context_info.antialiasingLevel = 0;
    } else {
        GLint samples;
        glGetIntegerv(GL_SAMPLES_ARB, &samples);
        context_info.antialiasingLevel = samples;
    }
}

void
GameWindow::Impl::setMouse(int16_t x, int16_t y)
{
    glfwSetCursorPos(win, x, y);
    mouse_x = x;
    mouse_y = y;
}

GameWindow &
GameWindow::construct_at(void *p, const WindowOptions &opts) noexcept
{
    auto &it = construct_impl(p, true, Impl::makeWindow(opts));
    it.init(opts);
    return it;
}

bool
GameWindow::init()
{
    return true;
}

void
GameWindow::Impl::handleInputEvents()
{
    glfwPollEvents();
}

void
GameWindow::showMouseCursor(bool show)
{
    if (show != m().show_mouse_cursor) {
        m().show_mouse_cursor = show;
        glfwSetInputMode(
          m().win, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}

bool
GameWindow::showMouseCursor() const
{
    return m().show_mouse_cursor;
}

void
GameWindow::vsync(bool enable)
{
    if (m().vsync != enable) {
        m().vsync = enable;
        ERR("VSYNC not implemented");
    }
}

bool
GameWindow::vsync()
{
    return m().vsync;
}

bool
GameWindow::focused() const
{
    return m().have_focus;
}

const std::shared_ptr<WindowRenderTarget> &
GameWindow::renderTarget() const
{
    return m().renderTarget;
}

WindowEvents &
GameWindow::events()
{
    return m().events;
}

void
GameWindow::registerHandlers(EngineEvents &evnts)
{
    evnts.handleInput.reg([this](const Event<InputEvent> & /*unused*/) {
        m().handleInputEvents();
    });
}

size_t
GameWindow::windowWidth() const
{
    size_t w;

    size_t h;
    windowSize(w, h);
    return w;
}

size_t
GameWindow::windowHeight() const
{
    size_t w;

    size_t h;
    windowSize(w, h);
    return h;
}

void
GameWindow::windowSize(size_t &width, size_t &height) const
{
    int w;

    int h;
    glfwGetWindowSize(m().win, &w, &h);
    width = w;
    height = h;
}

void
GameWindow::setMouse(int16_t x, int16_t y)
{
    m().setMouse(x, y);
}

void
GameWindow::swapBuffers()
{
    glfwSwapBuffers(m().win);
}

GLContextInfo
GameWindow::contextInfo() const
{
    return m().context_info;
}

GameWindowInit::GameWindowInit()
{
    if (!glfwInit()) {
        ERR("Couldnt initialize GLFW");
    } else {
        glfwSetErrorCallback(glfw_error_callback);
    }
}

GameWindowInit::~GameWindowInit()
{
    glfwTerminate();
}

} // namespace ge
