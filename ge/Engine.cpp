#include "ge/Engine.hpp"

#include "error/error.hpp"

#include "fs/fs.hpp"

#include <SFML/System/Clock.hpp>


#ifdef SYSTEM_UNIX
#define HAVE_UNISTD
#include <unistd.h>
#endif

namespace ge {

struct Engine::Data EXPLICIT : public GameLoop::Game {
    Engine& theEngine; // owning engine
    GameWindow window;
    GameLoop gameLoop;
    CommandProcessor commandProcessor;
    
    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;
    
    EngineEvents events;
    sf::Clock clock;

    Data(Engine& engine, const EngineOpts& opts) :
        theEngine(engine),
        window(opts.window),
        gameLoop(30)
    {}
                             
    void tick() OVERRIDE;
    void render(float interpolation) OVERRIDE;
    void handleInputEvents() OVERRIDE;
    float now() OVERRIDE;
    void sleep(float secs) OVERRIDE;
    void exit(int32 exit_code) OVERRIDE;

    void registerHandlers();
};

namespace {

bool runInit(EventSource<InitEvent>& source, const Event<InitEvent>& e);

} // namespace anon

#define SELF ({ ASSERT(self != 0); self; })

Engine::Engine() : self(0) {}

Engine::~Engine() { delete self; }

GameWindow& Engine::window() { return SELF->window; }

GameLoop& Engine::gameLoop() { return SELF->gameLoop; }

CommandProcessor& Engine::commandProcessor() { return SELF->commandProcessor; }

glt::ShaderManager& Engine::shaderManager() { return SELF->shaderManager; }

glt::RenderManager& Engine::renderManager() { return SELF->renderManager; }

EngineEvents& Engine::events() { return SELF->events; }

float Engine::now() { return SELF->now(); }

bool Engine::loadScript(const std::string& file) {
    ERR_ONCE("not yet implemented");
    return true;
}

bool Engine::evalCommand(const std::string& cmd) {
    ERR_ONCE("not yet implemented");
    return true;
}

int32 Engine::run(const EngineOpts& opts) {
    ASSERT(self == 0);

    if (opts.mode == EngineOpts::Help) {
        return 0;
    }

    if (!opts.workingDirectory.empty())
        fs::cwd(opts.workingDirectory);
    
    self = new Data(*this, opts);

    self->registerHandlers();

    Event<InitEvent> initEv = makeEvent(InitEvent(*this));
    if (!runInit(opts.inits.preInit, initEv))
        return 1;

    for (uint32 i = 0; i < opts.commands.size(); ++i) {
        bool ok;
        if (opts.commands[i].first == EngineOpts::Script)
            ok = loadScript(opts.commands[i].second);
        else
            ok = evalCommand(opts.commands[i].second);
        
        if (!ok) {
            ERR("initialization failed: script returned an error");
            return 1;
        }
    }

    if (!runInit(opts.inits.init, initEv))
        return 1;

    if (!runInit(opts.inits.postInit, initEv))
        return 1;

    opts.inits.preInit.clearHandlers();
    opts.inits.init.clearHandlers();
    opts.inits.postInit.clearHandlers();

    return self->gameLoop.run(*self);
}

void Engine::Data::tick() {
    events.animate.raise(EngineEvent(theEngine));
}

void Engine::Data::render(float interpolation) {
    events.beforeRender.raise(RenderEvent(theEngine, interpolation));
    events.render.raise(RenderEvent(theEngine, interpolation));
    events.afterRender.raise(RenderEvent(theEngine, interpolation));
}

void Engine::Data::handleInputEvents() {
    events.handleInput.raise(EngineEvent(theEngine));
}

float Engine::Data::now() {
    return clock.GetElapsedTime();
}

void Engine::Data::sleep(float secs) {
#ifdef HAVE_UNISTD
    usleep(uint32(secs * 1e6f));
#else
    ERR_ONCE("not yet implemented");
#endif
}

void Engine::Data::exit(int32 exit_code) {
    events.exit.raise(ExitEvent(theEngine, exit_code));
}

namespace {

bool runInit(EventSource<InitEvent>& source, const Event<InitEvent>& e) {
    for (uint32 i = 0; i < source.handlers.size(); ++i) {
        e.info.success = false;
        source.handlers[i]->handle(e);
        if (!e.info.success) {
            ERR("initialization failed: an initializer failed");
            return false;
        }
    }
    return true;
}

namespace handlers {

void sigExit(Engine *engine, const Event<WindowEvent>& e) {
    engine->gameLoop().exit(0);
}

} // namespace handlers

} // namespace anon

void Engine::Data::registerHandlers() {
    window.registerHandlers(events);
    window.events().windowClosed.registerHandler(makeEventHandler(handlers::sigExit, &theEngine));
}

EngineOpts& EngineOpts::parseOpts(int *argcp, char ***argvp) {
    int& argc = *argcp;
    char **(&argv) = *argvp;
    
    if (workingDirectory.empty() && argc > 0) {
        std::string binary(argv[0]);
        size_t pos = binary.rfind('/');
        if (pos != std::string::npos)
            workingDirectory = binary.substr(0, pos);
    }

    return *this;
}

} // namespace ge

