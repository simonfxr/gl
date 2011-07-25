#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"

#include "error/error.hpp"

#include "sys/clock.hpp"
#include "sys/fs/fs.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace ge {

struct Engine::Data EXPLICIT : public GameLoop::Game {
    Engine& theEngine; // owning engine
    GameWindow *window;
    GameLoop gameLoop;
    CommandProcessor commandProcessor;
    KeyHandler keyHandler;

    bool initialized;
    const EngineOptions *opts; // only during init
    
    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;
    
    EngineEvents events;

    Data(Engine& engine) :
        theEngine(engine),
        gameLoop(30),
        commandProcessor(engine),
        keyHandler(commandProcessor),
        initialized(false),
        opts(0)
    {}

    bool init(const EngineOptions& opts);
                             
    void tick() OVERRIDE;
    void render(float interpolation) OVERRIDE;
    void handleInputEvents() OVERRIDE;
    float now() OVERRIDE;
    void sleep(float secs) OVERRIDE;
    void exit(int32 exit_code) OVERRIDE;

    void registerHandlers();
    bool execCommand(std::vector<CommandArg>& args);
};

namespace {

bool runInit(EventSource<InitEvent>& source, const Event<InitEvent>& e);

} // namespace anon

#define SELF ASSERT_EXPR(self != 0, self)

Engine::Engine() : self(new Data(*this)) {}

Engine::~Engine() { delete self; }

GameWindow& Engine::window() {
    ASSERT_MSG(SELF->window != 0, "window not available, too early init phase");
    return *SELF->window;
}

GameLoop& Engine::gameLoop() { return SELF->gameLoop; }

CommandProcessor& Engine::commandProcessor() { return SELF->commandProcessor; }

KeyHandler& Engine::keyHandler() { return SELF->keyHandler; }

glt::ShaderManager& Engine::shaderManager() { return SELF->shaderManager; }

glt::RenderManager& Engine::renderManager() { return SELF->renderManager; }

EngineEvents& Engine::events() { return SELF->events; }

float Engine::now() { return SELF->now(); }

bool Engine::loadScript(const std::string& name, bool quiet) {

    std::string file = sys::fs::lookup(commandProcessor().scriptDirectories(), name);
    if (file.empty())
        goto not_found;

    {
        std::ifstream inp(file.c_str());
        if (!inp.is_open() || !inp.good())
            goto not_found;
        
        std::cerr << "loading script: " << file << std::endl;
        return loadStream(inp, file);
    }

not_found:
    
    std::cerr << "loading script: " << name << " -> not found" << std::endl;
    
    if (!quiet)
        ERR(("opening script: " + name).c_str());

    return false;
}

bool Engine::loadStream(std::istream& inp, const std::string& inp_name) {
    bool ok = true;
    std::vector<CommandArg> args;
    ParseState state(inp, inp_name);
    bool done = false;
    while (!done) {
        
        ok = tokenize(state, args);
        if (!ok) {
            if (!state.eof)
                ERR("parsing failed: " + state.filename);
            else
                ok = true;
            done = true;
            goto next;
        }
        
        ok = self->execCommand(args);
        if (!ok) {
            ERR("executing command");
            done = true;
            goto next;
        }
        
    next:;
        for (uint32 i = 0; i < args.size(); ++i)
            args[i].free();
        args.clear();
    }

    return ok;
}

bool Engine::evalCommand(const std::string& cmd) {
    std::istringstream inp(cmd);
    return loadStream(inp, "<unknown>");
}

int32 Engine::run(const EngineOptions& opts) {
    if (self->initialized) {
        ERR("Engine already initialized: called run multiply times?");
        return -1;
    }
    
    if (opts.mode == EngineOptions::Help) {
        EngineOptions::printHelp();
        return 0;
    }

    if (!opts.workingDirectory.empty())
        sys::fs::cwd(opts.workingDirectory);

    for (uint32 i = 0; i < opts.scriptDirs.size(); ++i) {
        if (!commandProcessor().addScriptDirectory(opts.scriptDirs[i])) {
            ERR("script directory not found: " + opts.scriptDirs[i]);
            return 1;
        }
    }

    for (uint32 i = 0; i < opts.shaderDirs.size(); ++i) {
        if (!shaderManager().addShaderDirectory(opts.shaderDirs[i])) {
            ERR("shader directory not found: " + opts.shaderDirs[i]);
            return 1;
        }
    }

    Event<InitEvent> initEv = makeEvent(InitEvent(*this));
    if (!runInit(opts.inits.preInit0, initEv))
        return 1;

    if (!self->init(opts))
        return 1;

    if (!runInit(opts.inits.preInit1, initEv))
        return 1;

    if (!opts.initScript.empty()) {
        loadScript(opts.initScript);
    }

    for (uint32 i = 0; i < opts.commands.size(); ++i) {
        bool ok;
        std::string command;
        if (opts.commands[i].first == EngineOptions::Script) {
            ok = loadScript(opts.commands[i].second);
            if (!ok)
                command = "script";
        } else {
            ok = evalCommand(opts.commands[i].second);
            if (!ok)
                command = "command";
        }

        if (!ok) {
            ERR(command + " failed: " + opts.commands[i].second);
            return 1;
        }
    }

    
    if (!runInit(opts.inits.init, initEv))
        return 1;

    if (!runInit(opts.inits.postInit, initEv))
        return 1;

    opts.inits.preInit0.clear();
    opts.inits.preInit1.clear();
    opts.inits.init.clear();
    opts.inits.postInit.clear();

    self->opts = 0;

    return self->gameLoop.run(*self);
}

void Engine::Data::tick() {
    events.animate.raise(makeEvent(EngineEvent(theEngine)));
}

void Engine::Data::render(float interpolation) {
    events.beforeRender.raise(makeEvent(RenderEvent(theEngine, interpolation)));
    renderManager.beginScene();
    events.render.raise(makeEvent(RenderEvent(theEngine, interpolation)));
    renderManager.endScene();
    events.afterRender.raise(makeEvent(RenderEvent(theEngine, interpolation)));
}

void Engine::Data::handleInputEvents() {
    events.handleInput.raise(makeEvent(EngineEvent(theEngine)));
}

float Engine::Data::now() {
    return float(sys::queryTimer());
}

void Engine::Data::sleep(float secs) {
    sys::sleep(secs);
}

void Engine::Data::exit(int32 exit_code) {
    events.exit.raise(makeEvent(ExitEvent(theEngine, exit_code)));
}

bool Engine::Data::execCommand(std::vector<CommandArg>& args) {
    if (args.size() == 0)
        return true;
    
    Array<CommandArg> com_args(&args[0], args.size());

    // ON_DEBUG(std::cerr << "executing command: ");
    // ON_DEBUG(prettyCommandArgs(std::cerr, com_args));
    // ON_DEBUG(std::cerr << std::endl);
    
    bool ok = commandProcessor.exec(com_args);

    if (!ok) {
        std::ostringstream err;
        err << "executing command failed: ";
        prettyCommandArgs(err, com_args);
        err << std::endl;
        ERR(err.str());
    }
    
    return ok;
}

bool Engine::Data::init(const EngineOptions& opts) {
    this->opts = &opts;
    window = new GameWindow(opts.window);
    renderManager.setDefaultRenderTarget(&window->renderTarget());
    registerHandlers();
    initialized = true;
    return true;
}

void Engine::addInit(RunLevel lvl, const Ref<EventHandler<InitEvent> >& comm) {
    
    if (SELF->opts == 0) {
        ERR("cannot register init handler, already initialized");
        return;
    }

    SELF->opts->inits.reg(lvl, comm);
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

void sigExit(Engine *engine, const Event<WindowEvent>&) {
    engine->gameLoop().exit(0);
}

void keyChanged(KeyHandler *handler, const Event<KeyChanged>& ev) {
    KeyCode code = fromSFML(ev.info.key.Code);
    if (ev.info.pressed)
        handler->keyPressed(code);
    else
        handler->keyReleased(code);
}

void focusChanged(KeyHandler *handler, const Event<FocusChanged>& ev) {
    if (!ev.info.focused) // lost focus, reset key states
        handler->clearStates();
}

void mouseButtonChanged(KeyHandler *handler, const Event<MouseButton>& ev) {
    KeyCode code = fromSFML(ev.info.button.Button);
    if (ev.info.pressed)
        handler->keyPressed(code);
    else
        handler->keyReleased(code);
}

void handleKeyBindings(KeyHandler *handler, const Event<EngineEvent>&) {
    handler->handleCommands();
}

void updateProjectionMatrix(Engine *eng, const Event<WindowResized>& e) {
    eng->renderManager().updateProjection(float(e.info.width) / float(e.info.height));
}

} // namespace handlers

} // namespace anon

void Engine::Data::registerHandlers() {
    window->registerHandlers(events);
    window->events().windowClosed.reg(makeEventHandler(handlers::sigExit, &theEngine));
    window->events().keyChanged.reg(makeEventHandler(handlers::keyChanged, &keyHandler));
    window->events().focusChanged.reg(makeEventHandler(handlers::focusChanged, &keyHandler));
    window->events().mouseButton.reg(makeEventHandler(handlers::mouseButtonChanged, &keyHandler));
    window->events().windowResized.reg(makeEventHandler(handlers::updateProjectionMatrix, &theEngine));
    theEngine.events().handleInput.reg(makeEventHandler(handlers::handleKeyBindings, &keyHandler));
}

} // namespace ge
