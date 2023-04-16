#include "ge/Engine.hpp"

#include <memory>

#include "err/err.hpp"
#include "ge/Tokenizer.hpp"
#include "ge/ge.hpp"
#include "glt/glt.hpp"
#include "glt/utils.hpp"
#include "sys/clock.hpp"
#include "sys/fs.hpp"
#include "sys/sys.hpp"

namespace ge {

using math::real;

struct Engine::Data : public GameLoop::Game
{
    struct Modules
    {
        BL_DISABLE_COPY_MOVE(Modules);

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

    Modules _modules;

    std::unique_ptr<GameWindow> window;
    GameLoop gameLoop;
    CommandProcessor commandProcessor;
    KeyHandler keyHandler;
    ReplServer replServer;
    bool skipRender{ false };
    std::string programName;
    std::string develDataDir;

    sys::io::OutStream *out;

    bool initialized{ false };
    const EngineOptions *opts{}; // only during init

    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;

    EngineEvents events;

    FixedPluginSet<> emptyPluginSet;
    PluginSet &pluginSet;

    explicit Data(Engine *self, PluginSet *ps)
      : gameLoop(60, 5, 120)
      , commandProcessor(*self)
      , keyHandler(commandProcessor)
      , replServer(*self)
      , out(&sys::io::stdout())
      , pluginSet(ps ? *ps : emptyPluginSet)
    {}

    bool init(const EngineOptions &opts);

    void tick() final;
    void render(double interpolation) final;
    void handleInputEvents() final;
    HU_NODISCARD GameLoop::time now() const final;
    void sleep(GameLoop::time secs) final;
    void atExit(int32_t exit_code) final;

    void registerHandlers();
    bool execCommand(std::vector<CommandArg> &args);

    Engine *self() { return reinterpret_cast<Engine *>(this); }
};

GE_API size_t
invoke(bl::runtime_size_f, bl::type_proxy<Engine>) noexcept
{
    return sizeof(Engine::Data);
}

Engine::Data &
Engine::m()
{
    return *reinterpret_cast<Engine::Data *>(this);
}

const Engine::Data &
Engine::m() const
{
    return *reinterpret_cast<const Engine::Data *>(this);
}

namespace {

bool
runInit(EventSource<InitEvent> &source, const Event<InitEvent> &e);

} // namespace

Engine *
Engine::construct_at(Engine *p, PluginSet &ps) noexcept
{
    return reinterpret_cast<Engine *>(new (p) Data(p, &ps));
}

Engine *
Engine::construct_at(Engine *p) noexcept
{
    return reinterpret_cast<Engine *>(new (p) Data(p, nullptr));
}

void
Engine::destroy_at(Engine *p) noexcept
{
    p->renderManager().shutdown();
    p->shaderManager().shutdown();
    p->m().~Data();
}

GameWindow &
Engine::window()
{
    ASSERT(m().window != nullptr, "window not available, too early init phase");
    return *m().window;
}

GameLoop &
Engine::gameLoop()
{
    return m().gameLoop;
}

CommandProcessor &
Engine::commandProcessor()
{
    return m().commandProcessor;
}

KeyHandler &
Engine::keyHandler()
{
    return m().keyHandler;
}

ReplServer &
Engine::replServer()
{
    return m().replServer;
}

glt::ShaderManager &
Engine::shaderManager()
{
    return m().shaderManager;
}

glt::RenderManager &
Engine::renderManager()
{
    return m().renderManager;
}

EngineEvents &
Engine::events()
{
    return m().events;
}

sys::io::OutStream &
Engine::out()
{
    return *m().out;
}

PluginSet &
Engine::pluginSet()
{
    return m().pluginSet;
}

void
Engine::out(sys::io::OutStream &s)
{
    m().out = &s;
    shaderManager().out(s);
}

math::real
Engine::now()
{
    return static_cast<math::real>(m().now());
}

const std::string &
Engine::programName() const
{
    return m().programName;
}

int32_t
Engine::run(const EngineOptions &opts)
{
    if (m().initialized) {
        ERR("Engine already initialized: called run multiply times?");
        return 1;
    }

    if (opts.mode == EngineOptions::Help) {
        EngineOptions::printHelp();
        return 0;
    }

    if (opts.traceOpenGL) {
#if ENABLE_GLDEBUG_P
        glt::printOpenGLCalls(true);
#else
        WARN("cannot enable OpenGL tracing: not compiled with ENABLE_GLDEBUG");
#endif
    }

    m().shaderManager.dumpShadersEnable(opts.dumpShaders);

    m().skipRender = opts.disableRender;

    std::string wd = opts.workingDirectory;
    if (opts.workingDirectory.empty() && opts.defaultCD)
        wd = sys::fs::dirname(opts.binary);

    m().programName = sys::fs::dropExtension(sys::fs::basename(opts.binary));

    if (!wd.empty()) {
        if (!sys::fs::cwd(wd)) {
            ERR("couldnt change into directory: " + wd);
            return 1;
        }
    }

    if (!m().develDataDir.empty()) {
        commandProcessor().addScriptDirectory(
          sys::fs::join(m().develDataDir, "scripts"));
        commandProcessor().addScriptDirectory(
          sys::fs::join(m().develDataDir, "..", "..", "scripts"));
        shaderManager().addShaderDirectory(
          sys::fs::join(m().develDataDir, "shaders"));
        shaderManager().addShaderDirectory(
          sys::fs::join(m().develDataDir, "..", "..", "shaders"));
    }

    for (const auto &scriptDir : opts.scriptDirs) {
        if (!commandProcessor().addScriptDirectory(scriptDir)) {
            ERR("script directory not found: " + scriptDir);
            //            return 1;
        }
    }

    for (const auto &shaderDir : opts.shaderDirs) {
        if (!shaderManager().addShaderDirectory(shaderDir)) {
            ERR("shader directory not found: " + shaderDir);
            //            return 1;
        }
    }

    auto initEv = Event(InitEvent(*this));
    if (!runInit(opts.inits.preInit0, initEv))
        return 1;

    if (!m().init(opts))
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

    for (const auto &i : opts.commands) {
        bool ok;
        std::string command;
        if (i.first == EngineOptions::Script) {
            ok = commandProcessor().loadScript(i.second);
            if (!ok)
                command = "script";
        } else {
            ok = commandProcessor().evalCommand(i.second);
            if (!ok)
                command = "command";
        }

        if (!ok) {
            ERR(command + " failed: " + i.second);
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

    m().opts = nullptr;

    return m().gameLoop.run(m());
}

void
Engine::Data::tick()
{
    events.animate.raise(Event(AnimationEvent(*self())));
}

void
Engine::Data::render(double interpolation)
{
    events.beforeRender.raise(
      Event(RenderEvent(*self(), static_cast<real>(interpolation))));
    if (!skipRender) {
        GL_TRACE("BEGIN_SCENE");
        renderManager.beginScene();
        events.render.raise(
          Event(RenderEvent(*self(), static_cast<real>(interpolation))));
        renderManager.endScene();
        GL_TRACE("END_SCENE");
    }
    events.afterRender.raise(
      Event(RenderEvent(*self(), static_cast<real>(interpolation))));
}

void
Engine::Data::handleInputEvents()
{
    events.handleInput.raise(Event(InputEvent(*self())));
}

GameLoop::time
Engine::Data::now() const
{
    return static_cast<GameLoop::time>(sys::queryTimer());
}

void
Engine::Data::sleep(GameLoop::time secs)
{
    sys::sleep(secs);
}

void
Engine::Data::atExit(int32_t exit_code)
{
    events.exit.raise(Event(ExitEvent(*self(), exit_code)));
}

bool
Engine::Data::init(const EngineOptions &eopts)
{
    opts = &eopts;
    window = std::make_unique<GameWindow>(eopts.window);
    renderManager.setDefaultRenderTarget(window->renderTarget());
    registerHandlers();
    initialized = true;
    return true;
}

void
Engine::addInit(RunLevel lvl,
                const std::shared_ptr<EventHandler<InitEvent>> &comm)
{

    if (m().opts == nullptr) {
        ERR("cannot register init handler, already initialized");
        return;
    }

    m().opts->inits.reg(lvl, comm);
}

namespace {

bool
runInit(EventSource<InitEvent> &source, const Event<InitEvent> &e)
{
    for (auto &handler : source.handlers()) {
        e.info.success = false;
        handler->handle(e);
        if (!e.info.success) {
            ERR("initialization failed: an initializer failed");
            return false;
        }
    }
    return true;
}

} // namespace

void
Engine::Data::registerHandlers()
{
    window->registerHandlers(events);

    window->events().windowClosed.reg(
      [this](const Event<WindowEvent> & /*unused*/) {
          self()->gameLoop().exit(0);
      });

    window->events().keyChanged.reg([this](const Event<KeyChanged> &ev) {
        keyHandler.keyEvent(ev.info.key);
    });

    window->events().focusChanged.reg([this](const Event<FocusChanged> &ev) {
        if (!ev.info.focused)
            keyHandler.clearStates();
    });

    window->events().mouseButton.reg([this](const Event<MouseButton> &ev) {
        keyHandler.keyEvent(ev.info.button);
    });

    window->events().windowResized.reg([this](const Event<WindowResized> &ev) {
        self()->renderManager().updateProjection(
          static_cast<math::real>(ev.info.width) /
          static_cast<math::real>(ev.info.height));
    });

    self()->events().handleInput.reg(
      [this](const Event<InputEvent> & /*unused*/) {
          keyHandler.handleCommands();
      });
}

void
Engine::enablePlugin(Plugin &p)
{
    p.registerWith(*this);
    p.registerCommands(commandProcessor());
}

void
Engine::setDevelDataDir(const std::string &dir)
{
    if (!sys::fs::directoryExists(dir)) {
        WARN("not a directory");
        return;
    }
    m().develDataDir = dir;
}

} // namespace ge
