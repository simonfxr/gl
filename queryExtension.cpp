#include "opengl.h"

#include "ge/Engine.hpp"
#include "glt/utils.hpp"

#include <iostream>
#include <cstring>

#include <GL/glxew.h>

namespace {

struct State {
    int argc;
    char **argv;
};

void animate(State *state, const ge::Event<ge::AnimationEvent>& ev) {
    ge::Engine& e = ev.info.engine;

    e.commandProcessor().exec("printContextInfo", ge::NULL_ARGS);

    for (int i = 1; i < state->argc; ++i) {
        if (strncmp(state->argv[i], "GL_", 3) == 0) {
            bool ok = glt::isExtensionSupported(state->argv[i]);
            ok = ok || glxewIsSupported(state->argv[i]);
            std::cerr << "extension " << state->argv[i] << ": " << (ok ? "yes" : "no") << std::endl;
        } else {
            bool ok = glXGetProcAddress(glt::gl_str(state->argv[i])) != 0;
            std::cerr << "function " << state->argv[i] << ": " << (ok ? "yes" : "no") << std::endl;
        }        
    }
    
    e.gameLoop().exit(0);
}

} // namespace anon

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    ge::Engine engine;
    
    opts.parse(&argc, &argv);
    for (int i = 1; i < argc; ++i)
        if (argv[i] != 0 && argv[i][0] == '-')
            std::cerr << "unknown option: " << argv[i] << std::endl;

    State state;
    state.argc = argc;
    state.argv = argv;
    engine.events().animate.reg(makeEventHandler(animate, &state));

    int32 ret = engine.run(opts);

    return ret;
}
