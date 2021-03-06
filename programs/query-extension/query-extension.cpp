#include "glt/utils.hpp"
#include "opengl.hpp"

#include "ge/Engine.hpp"

#include <cstring>

#ifdef SYSTEM_UNIX
#    include <GL/glxew.h>
#    define GLEW_IS_SUPPORTED(ext) glxewIsSupported((ext))
#elif defined(SYSTEM_WINDOWS)
#    include <GL/wglew.h>
#    define GLEW_IS_SUPPORTED(ext) wglewIsSupported((ext))
#else
#    error "unknown system"
#endif

namespace {

struct State
{
    int argc;
    char **argv;
};

void
animate(State *state, const ge::Event<ge::AnimationEvent> &ev)
{
    ge::Engine &e = ev.info.engine;

    Array<ge::CommandArg> args = { 0, nullptr };
    e.commandProcessor().exec("printContextInfo", args);

    for (int i = 1; i < state->argc; ++i) {
        if (strncmp(state->argv[i], "GL_", 3) == 0) {
            bool ok = glt::isExtensionSupported(state->argv[i]);
            ok = ok || GLEW_IS_SUPPORTED(state->argv[i]);
            sys::io::stderr() << "extension " << state->argv[i] << ": "
                              << (ok ? "yes" : "no") << "\n";
        } else {
            bool ok = GLEW_IS_SUPPORTED(state->argv[i]) != 0;
            sys::io::stderr() << "function " << state->argv[i] << ": "
                              << (ok ? "yes" : "no") << "\n";
        }
    }

    e.gameLoop().exit(0);
}

} // namespace

int
main(int argc, char *argv[])
{
    ge::EngineOptions opts;
    ge::Engine engine;

    opts.parse(&argc, &argv);
    for (int i = 1; i < argc; ++i)
        if (argv[i] != 0 && argv[i][0] == '-')
            sys::io::stderr() << "unknown option: " << argv[i] << "\n";

    State state;
    state.argc = argc;
    state.argv = argv;
    engine.events().animate.reg(makeEventHandler(animate, &state));

    int32 ret = engine.run(opts);

    return ret;
}
