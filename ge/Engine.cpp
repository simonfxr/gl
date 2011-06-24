#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"

#include "error/error.hpp"

#include "fs/fs.hpp"

#include <SFML/System.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

namespace ge {

struct Engine::Data EXPLICIT : public GameLoop::Game {
    Engine& theEngine; // owning engine
    GameWindow *window;
    GameLoop gameLoop;
    CommandProcessor commandProcessor;
    KeyHandler keyHandler;

    const WindowOpts *winOpts; // only during init
    
    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;
    
    EngineEvents events;
    sf::Clock clock;

    Data(Engine& engine) :
        theEngine(engine),
        gameLoop(30),
        commandProcessor(engine),
        keyHandler(commandProcessor),
        winOpts(0)
    {}

    void init(const Event<InitEvent>&);
                             
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

#define SELF ({ ASSERT(self != 0); self; })

Engine::Engine() : self(0) {}

Engine::~Engine() { delete self; }

GameWindow& Engine::window() { return *SELF->window; }

GameLoop& Engine::gameLoop() { return SELF->gameLoop; }

CommandProcessor& Engine::commandProcessor() { return SELF->commandProcessor; }

KeyHandler& Engine::keyHandler() { return SELF->keyHandler; }

glt::ShaderManager& Engine::shaderManager() { return SELF->shaderManager; }

glt::RenderManager& Engine::renderManager() { return SELF->renderManager; }

EngineEvents& Engine::events() { return SELF->events; }

float Engine::now() { return SELF->now(); }

bool Engine::loadScript(const std::string& file) {
    std::ifstream inp(file.c_str());
    if (!inp.is_open()) {
        ERR(("opening file: " + file).c_str());
        return false;
    }

    bool ok = true;

    std::vector<CommandArg> args;
    ParseState state(inp, commandProcessor(), file);
    bool done = false;
    while (inp.good() && !done) {
        
        ok = tokenize(state, args);
        if (!ok) {
            ERR("parsing failed: " + state.filename);
            done = true;
            goto next;
        }
        
        ok = !self->execCommand(args);
        if (!ok) {
//            ERR("executing command");
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
    std::vector<CommandArg> args;
    std::istringstream inp(cmd);
    ParseState state(inp, commandProcessor(), "<unknown>");
    bool ok = tokenize(state, args) && self->execCommand(args);
    for (uint32 i = 0; i < args.size(); ++i)
        args[i].free();
    return ok;
}

int32 Engine::run(const EngineOpts& opts) {
    ASSERT(self == 0);

    if (opts.mode == EngineOpts::Help) {
        // TODO: help();
        return 0;
    }

    if (!opts.workingDirectory.empty())
        fs::cwd(opts.workingDirectory);

    self = new Data(*this);

    self->winOpts = &opts.window;
    opts.inits.reg(PreInit0, makeEventHandler(self, &Data::init));

    Event<InitEvent> initEv = makeEvent(InitEvent(*this));
    if (!runInit(opts.inits.preInit0, initEv))
        return 1;

    if (!runInit(opts.inits.preInit1, initEv))
        return 1;

    for (uint32 i = 0; i < opts.commands.size(); ++i) {
        bool ok;
        std::string command;
        if (opts.commands[i].first == EngineOpts::Script) {
            ok = loadScript(opts.commands[i].second);
            if (!ok)
                command = "script";
        } else {
            ok = evalCommand(opts.commands[i].second);
            if (!ok)
                command = "command";
        }

        if (!ok) {
//            ERR(command + " failed: " + opts.commands[i].second);
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

    self->winOpts = 0;

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
    return float(clock.GetElapsedTime()) / 1000.f;
}

void Engine::Data::sleep(float secs) {
    sf::Sleep(uint32(secs * 1000.f));
}

void Engine::Data::exit(int32 exit_code) {
    events.exit.raise(ExitEvent(theEngine, exit_code));
}

bool Engine::Data::execCommand(std::vector<CommandArg>& args) {
    Array<CommandArg> com_args(&args[0], args.size());
    return commandProcessor.exec(com_args);
}

void Engine::Data::init(const Event<InitEvent>& e) {
    window = new GameWindow(*winOpts);
    registerHandlers();
    e.info.success = true;
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

} // namespace handlers

} // namespace anon

void Engine::Data::registerHandlers() {
    window->registerHandlers(events);
    window->events().windowClosed.reg(makeEventHandler(handlers::sigExit, &theEngine));
    window->events().keyChanged.reg(makeEventHandler(handlers::keyChanged, &keyHandler));
    window->events().mouseButton.reg(makeEventHandler(handlers::mouseButtonChanged, &keyHandler));
    theEngine.events().handleInput.reg(makeEventHandler(handlers::handleKeyBindings, &keyHandler));
}

static bool str_eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

#define CMDWARN(msg) WARN(std::string("parsing options: ") + (msg))

EngineOpts& EngineOpts::parseOpts(int *argcp, char ***argvp) {
    int& argc = *argcp;
    char **argv = *argvp;
    
    if (workingDirectory.empty() && argc > 0) {
        std::string binary(argv[0]);
        size_t pos = binary.rfind('/');
        if (pos != std::string::npos)
            workingDirectory = binary.substr(0, pos);
    }

    int nargs = argc;
    int dest = 1;
    bool done = false;
    for (int i = 1; i < nargs && !done; ++i) {
        int arg_begin = i;
        int arg_end = i + 1;
        const char *arg1 = argv[i];
        
        if (str_eq(arg1, "--")) {
            done = true;
        } else if (str_eq(arg1, "--help")) {
            mode = Help;
        } else {
            if (++i >= nargs || str_eq(argv[i], "--")) {
                if (i < nargs)
                    ++arg_end;
                done = true;
                goto next;
            }

            const char *arg2 = argv[i];
            ++arg_end;
            
            if (str_eq(arg1, "--eval")) {
                commands.push_back(std::make_pair(Command, std::string(arg2)));
            } else if (str_eq(arg1, "--script")) {
                commands.push_back(std::make_pair(Script, std::string(arg2)));
            } else {
                --arg_end;
            }
        }

    next:;

        for (int j = arg_begin; j < arg_end; ++j)
            argv[dest++] = argv[j];
    }

    argc = dest;
    argv[argc] = 0;

    return *this;
}

} // namespace ge
