#include "glt/glt.hpp"
#include "glt/utils.hpp"

#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"
#include "ge/ge.hpp"

#include "err/err.hpp"

#include "sys/clock.hpp"
#include "sys/fs.hpp"
#include "sys/sys.hpp"

namespace ge {

struct Engine::Data : public GameLoop::Game
{

    struct Modules
    {
        Modules()
        {
            sys::moduleInit();
            glt::moduleInit();
            ge::moduleInit();
        }

        ~Modules()
        {
            ge::moduleExit();
            glt::moduleExit();
            sys::moduleExit();
        }
    };

    Modules __modules;

    Engine &theEngine; // owning engine
    GameWindow *window;
    GameLoop gameLoop;
    CommandProcessor commandProcessor;
    KeyHandler keyHandler;
    ReplServer replServer;
    bool skipRender;
    std::string programName;

    sys::io::OutStream *out;

    bool initialized;
    const EngineOptions *opts; // only during init

    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;

    EngineEvents events;

    Data(Engine &engine)
      : theEngine(engine)
      , gameLoop(60, 5, 120)
      , commandProcessor(engine)
      , keyHandler(commandProcessor)
      , replServer(engine)
      , out(&sys::io::stdout())
      , initialized(false)
      , opts(0)
    {}

    ~Data() {}

    bool init(const EngineOptions &opts);

    virtual void tick() final override;
    virtual void render(double interpolation) final override;
    virtual void handleInputEvents() final override;
    virtual GameLoop::time now() final override;
    virtual void sleep(GameLoop::time secs) final override;
    virtual void atExit(int32 exit_code) final override;

    void registerHandlers();
    bool execCommand(std::vector<CommandArg> &args);
};

namespace {

bool
runInit(EventSource<InitEvent> &source, const Event<InitEvent> &e);

} // namespace

// #define SELF ASSERT_EXPR(self != 0, self)
#define SELF self

Engine::Engine() : self(new Data(*this)) {}

Engine::~Engine()
{
    self->renderManager.shutdown();
    self->shaderManager.shutdown();
    delete self->window;
    delete self;
}

GameWindow &
Engine::window()
{
    ASSERT_MSG(SELF->window != 0, "window not available, too early init phase");
    return *SELF->window;
}

GameLoop &
Engine::gameLoop()
{
    return SELF->gameLoop;
}

CommandProcessor &
Engine::commandProcessor()
{
    return SELF->commandProcessor;
}

KeyHandler &
Engine::keyHandler()
{
    return SELF->keyHandler;
}

ReplServer &
Engine::replServer()
{
    return SELF->replServer;
}

glt::ShaderManager &
Engine::shaderManager()
{
    return SELF->shaderManager;
}

glt::RenderManager &
Engine::renderManager()
{
    return SELF->renderManager;
}

EngineEvents &
Engine::events()
{
    return SELF->events;
}

sys::io::OutStream &
Engine::out()
{
    return *self->out;
}

void
Engine::out(sys::io::OutStream &s)
{
    self->out = &s;
    shaderManager().out(s);
}

math::real
Engine::now()
{
    return math::real(SELF->now());
}

const std::string
Engine::programName() const
{
    return self->programName;
}

int32
Engine::run(const EngineOptions &opts)
{
    if (self->initialized) {
        ERR("Engine already initialized: called run multiply times?");
        return 1;
    }

    if (opts.mode == EngineOptions::Help) {
        EngineOptions::printHelp();
        return 0;
    }

    if (opts.traceOpenGL) {
#ifdef GLDEBUG
        glt::printOpenGLCalls(true);
#else
        WARN("cannot enable OpenGL tracing: not compiled with GLDEBUG");
#endif
    }

    self->skipRender = opts.disableRender;

    std::string wd = opts.workingDirectory;
    if (opts.workingDirectory.empty() && opts.defaultCD)
        wd = sys::fs::dirname(opts.binary);

    self->programName = sys::fs::basename(opts.binary);

    if (!wd.empty()) {
        if (!sys::fs::cwd(wd)) {
            ERR("couldnt change into directory: " + wd);
            return 1;
        }
    }

    for (uint32 i = 0; i < opts.scriptDirs.size(); ++i) {
        if (!commandProcessor().addScriptDirectory(opts.scriptDirs[i])) {
            ERR("script directory not found: " + opts.scriptDirs[i]);
            //            return 1;
        }
    }

    for (uint32 i = 0; i < opts.shaderDirs.size(); ++i) {
        if (!shaderManager().addShaderDirectory(opts.shaderDirs[i])) {
            ERR("shader directory not found: " + opts.shaderDirs[i]);
            //            return 1;
        }
    }

    Event<InitEvent> initEv = makeEvent(InitEvent(*this));
    if (!runInit(opts.inits.preInit0, initEv))
        return 1;

    if (!self->init(opts))
        return 1;

    if (!runInit(opts.inits.preInit1, initEv))
        return 1;

    if (!opts.inhibitInitScript) {
        std::string script = sys::fs::basename(opts.binary);
        if (!script.empty())
            script = sys::fs::dropExtension(script);

        if (script.empty() ||
            !commandProcessor().loadScript(script + ".script"))
            return 1;
    }

    for (uint32 i = 0; i < opts.commands.size(); ++i) {
        bool ok;
        std::string command;
        if (opts.commands[i].first == EngineOptions::Script) {
            ok = commandProcessor().loadScript(opts.commands[i].second);
            if (!ok)
                command = "script";
        } else {
            ok = commandProcessor().evalCommand(opts.commands[i].second);
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

void
Engine::Data::tick()
{
    events.animate.raise(makeEvent(AnimationEvent(theEngine)));
}

void
Engine::Data::render(double interpolation)
{
    events.beforeRender.raise(makeEvent(RenderEvent(theEngine, interpolation)));
    if (!skipRender) {
        GL_TRACE("BEGIN_SCENE");
        renderManager.beginScene();
        events.render.raise(makeEvent(RenderEvent(theEngine, interpolation)));
        renderManager.endScene();
        GL_TRACE("END_SCENE");
    }
    events.afterRender.raise(makeEvent(RenderEvent(theEngine, interpolation)));
}

void
Engine::Data::handleInputEvents()
{
    events.handleInput.raise(makeEvent(InputEvent(theEngine)));
}

GameLoop::time
Engine::Data::now()
{
    return GameLoop::time(sys::queryTimer());
}

void
Engine::Data::sleep(GameLoop::time secs)
{
    sys::sleep(secs);
}

void
Engine::Data::atExit(int32 exit_code)
{
    events.exit.raise(makeEvent(ExitEvent(theEngine, exit_code)));
}

bool
Engine::Data::init(const EngineOptions &opts)
{
    this->opts = &opts;
    window = new GameWindow(opts.window);
    renderManager.setDefaultRenderTarget(&window->renderTarget());
    registerHandlers();
    initialized = true;
    return true;
}

void
Engine::addInit(RunLevel lvl, const Ref<EventHandler<InitEvent>> &comm)
{

    if (SELF->opts == 0) {
        ERR("cannot register init handler, already initialized");
        return;
    }

    SELF->opts->inits.reg(lvl, comm);
}

namespace {

bool
runInit(EventSource<InitEvent> &source, const Event<InitEvent> &e)
{
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

void
sigExit(Engine *engine, const Event<WindowEvent> &)
{
    engine->gameLoop().exit(0);
}

void
keyChanged(KeyHandler *handler, const Event<KeyChanged> &ev)
{
    handler->keyEvent(ev.info.key);
}

void
focusChanged(KeyHandler *handler, const Event<FocusChanged> &ev)
{
    if (!ev.info.focused) // lost focus, reset key states
        handler->clearStates();
}

void
mouseButtonChanged(KeyHandler *handler, const Event<MouseButton> &ev)
{
    handler->keyEvent(ev.info.button);
}

void
handleKeyBindings(KeyHandler *handler, const Event<InputEvent> &)
{
    handler->handleCommands();
}

void
updateProjectionMatrix(Engine *eng, const Event<WindowResized> &e)
{
    eng->renderManager().updateProjection(math::real(e.info.width) /
                                          math::real(e.info.height));
}

} // namespace handlers

} // namespace

void
Engine::Data::registerHandlers()
{
    window->registerHandlers(events);
    window->events().windowClosed.reg(
      makeEventHandler(handlers::sigExit, &theEngine));
    window->events().keyChanged.reg(
      makeEventHandler(handlers::keyChanged, &keyHandler));
    window->events().focusChanged.reg(
      makeEventHandler(handlers::focusChanged, &keyHandler));
    window->events().mouseButton.reg(
      makeEventHandler(handlers::mouseButtonChanged, &keyHandler));
    window->events().windowResized.reg(
      makeEventHandler(handlers::updateProjectionMatrix, &theEngine));
    theEngine.events().handleInput.reg(
      makeEventHandler(handlers::handleKeyBindings, &keyHandler));
}

void
Engine::enablePlugin(Plugin &p)
{
    p.registerWith(*this);
    p.registerCommands(commandProcessor());
}

} // namespace ge
