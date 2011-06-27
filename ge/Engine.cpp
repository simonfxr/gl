#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"

#include "error/error.hpp"

#include "sys/fs/fs.hpp"

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

    const EngineOpts *opts; // only during init
    
    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;
    
    EngineEvents events;
    sf::Clock clock;

    Data(Engine& engine) :
        theEngine(engine),
        gameLoop(30),
        commandProcessor(engine),
        keyHandler(commandProcessor),
        opts(0)
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

bool Engine::loadScript(const std::string& file, bool quiet) {
    std::ifstream inp(file.c_str());
    if (!inp.is_open() || !inp.good()) {
        std::cerr << "loading script: " << file << " -> not found" << std::endl;

        if (!quiet)
            ERR(("opening file: " + file).c_str());
        return false;
    }

    std::cerr << "loading script: " << file << std::endl;
    return loadStream(inp, file);
}

bool Engine::loadStream(std::istream& inp, const std::string& inp_name) {
    bool ok = true;
    std::vector<CommandArg> args;
    ParseState state(inp, commandProcessor(), inp_name);
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

int32 Engine::run(const EngineOpts& opts) {
    ASSERT(self == 0);

    if (opts.mode == EngineOpts::Help) {
        // TODO: help();
        return 0;
    }

    if (!opts.workingDirectory.empty())
        sys::fs::cwd(opts.workingDirectory);

    self = new Data(*this);

    self->opts = &opts;
    opts.inits.reg(PreInit0, makeEventHandler(self, &Data::init));

    Event<InitEvent> initEv = makeEvent(InitEvent(*this));
    if (!runInit(opts.inits.preInit0, initEv))
        return 1;

    if (!runInit(opts.inits.preInit1, initEv))
        return 1;

    if (!opts.initScript.empty()) {
        loadScript(opts.initScript, true);
    }

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
    if (args.size() == 0)
        return true;
    
    Array<CommandArg> com_args(&args[0], args.size());

    ON_DEBUG(std::cerr << "executing command: ");
    ON_DEBUG(prettyCommandArgs(std::cerr, com_args));
    ON_DEBUG(std::cerr << std::endl);
    
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

void Engine::Data::init(const Event<InitEvent>& e) {
    window = new GameWindow(opts->window);
    renderManager.setDefaultRenderTarget(&window->renderTarget());
    registerHandlers();
    e.info.success = true;
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
    window->events().mouseButton.reg(makeEventHandler(handlers::mouseButtonChanged, &keyHandler));
    window->events().windowResized.reg(makeEventHandler(handlers::updateProjectionMatrix, &theEngine));
    theEngine.events().handleInput.reg(makeEventHandler(handlers::handleKeyBindings, &keyHandler));
}

static bool str_eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

#define CMDWARN(msg) WARN(std::string("parsing options: ") + (msg))

EngineOpts& EngineOpts::parseOpts(int *argcp, char ***argvp) {
    int& argc = *argcp;
    char **argv = *argvp;

    bool no_cd = false;

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
        } else if (str_eq(arg1, "--no-init-script")) {
            initScript = "";
        } else if (str_eq(arg1, "--no-cd")) {
            no_cd = true;        
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
            } else if (str_eq(arg1, "--gl-version")) {
                int maj, min;
                if (sscanf(arg2, "%d.%d", &maj, &min) != 2 || maj < 0 || min < 0 || maj > 9 || min > 9) {
                    CMDWARN("invalid OpenGL Version: " + std::string(arg2));
                } else {
                    window.settings.MajorVersion = maj;
                    window.settings.MinorVersion = min;
                }
            } else if (str_eq(arg1, "--gl-profile")) {
                if (str_eq(arg2, "core")) {
                    window.settings.CoreProfile = true;
                } else if (str_eq(arg2, "compat") || str_eq(arg2, "compatibility")) {
                    window.settings.CoreProfile = false;
                } else {
                    CMDWARN("invalid OpenGL Profile type: " + std::string(arg2));
                }
            } else if (str_eq(arg1, "--gl-debug")) {
                if (str_eq(arg2, "yes")) {
                    window.settings.DebugContext = true;
                } else if (str_eq(arg2, "no")) {
                    window.settings.DebugContext = false;
                } else {
                    CMDWARN("--gl-debug: not a boolean option");
                }
            } else if (str_eq(arg1, "--cwd")) {
                workingDirectory = arg2;
            } else {
                --arg_end;
            }
        }

    next:;

        for (int j = arg_begin; j < arg_end; ++j)
            argv[dest++] = argv[j];
    }

    if (!no_cd && workingDirectory.empty() && argc > 0) {
        std::string bin(argv[0]);
        workingDirectory = sys::fs::dirname(bin);
        std::string exec = sys::fs::basename(bin);
        if (!exec.empty())
            initScript = exec + ".script";
    }

    argc = dest;
    argv[argc] = 0;

    return *this;
}

} // namespace ge
