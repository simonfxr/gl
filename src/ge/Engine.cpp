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
#include "util/string.hpp"

#include <bl/box.hpp>

using namespace bl::literals;

namespace ge {

using math::real;

struct Engine::Impl
  : Engine
  , GameLoop::Game
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

    bl::nullable_owned_ptr<GameWindow> window;
    GameLoop gameLoop;
    bl::box<CommandProcessor> commandProcessor;
    bl::box<KeyHandler> keyHandler;
    bl::box<ReplServer> replServer;
    bool skipRender{ false };
    bl::string programName;
    bl::string develDataDir;

    sys::io::OutStream out;

    bool initialized{ false };
    const EngineOptions *opts{}; // only during init

    bl::box<glt::ShaderManager> shaderManager;
    bl::box<glt::RenderManager> renderManager;

    EngineEvents events;

    FixedPluginSet<> emptyPluginSet;
    PluginSet &pluginSet;

    explicit Impl(PluginSet *ps)
      : gameLoop(60, 5, 120)
      , commandProcessor(std::in_place, *this)
      , keyHandler(std::in_place, *commandProcessor)
      , replServer(std::in_place, *this)
      , out(sys::io::stdout())
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
    bool execCommand(bl::vec<CommandArg> &args);

    ~Impl() override
    {
        renderManager->shutdown();
        shaderManager->shutdown();
    }
};

BL_DEF_RUNTIME_SIZED(Engine, Engine::Impl);

namespace {
bool
runInit(EventSource<InitEvent> &source, const Event<InitEvent> &e);
} // namespace

Engine &
Engine::construct_at(void *p, PluginSet &ps) noexcept
{
    return construct_impl(p, &ps);
}

Engine &
Engine::construct_at(void *p) noexcept
{
    return construct_impl(p, nullptr);
}

GameWindow &
Engine::window()
{
    ASSERT(m().window, "window not available, too early init phase");
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
    return *m().commandProcessor;
}

KeyHandler &
Engine::keyHandler()
{
    return *m().keyHandler;
}

ReplServer &
Engine::replServer()
{
    return *m().replServer;
}

glt::ShaderManager &
Engine::shaderManager()
{
    return *m().shaderManager;
}

glt::RenderManager &
Engine::renderManager()
{
    return *m().renderManager;
}

EngineEvents &
Engine::events()
{
    return m().events;
}

sys::io::OutStream
Engine::out()
{
    return m().out;
}

PluginSet &
Engine::pluginSet()
{
    return m().pluginSet;
}

void
Engine::out(sys::io::OutStream s)
{
    m().out = s;
    shaderManager().out(s);
}

math::real
Engine::now()
{
    return static_cast<math::real>(m().now());
}

const bl::string &
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

    m().shaderManager->dumpShadersEnable(opts.dumpShaders);

    m().skipRender = opts.disableRender;

    bl::string wd = opts.workingDirectory;
    if (opts.workingDirectory.empty() && opts.defaultCD)
        wd = sys::fs::dirname(opts.binary);

    m().programName = sys::fs::dropExtension(sys::fs::basename(opts.binary));

    if (!wd.empty()) {
        if (!sys::fs::cwd(wd)) {
            ERR(scat("couldnt change into directory: "_k, wd));
            return 1;
        }
    }

    if (!m().develDataDir.empty()) {
        commandProcessor().addScriptDirectory(
          sys::fs::join(m().develDataDir, "scripts"_k));
        commandProcessor().addScriptDirectory(
          sys::fs::join(m().develDataDir, ".."_k, ".."_k, "scripts"_k));
        shaderManager().addShaderDirectory(
          sys::fs::join(m().develDataDir, "shaders"_k));
        shaderManager().addShaderDirectory(
          sys::fs::join(m().develDataDir, ".."_k, ".."_k, "shaders"_k));
    }

    for (const auto &scriptDir : opts.scriptDirs) {
        if (!commandProcessor().addScriptDirectory(scriptDir)) {
            ERR(scat("script directory not found: "_k, scriptDir));
            //            return 1;
        }
    }

    for (const auto &shaderDir : opts.shaderDirs) {
        if (!shaderManager().addShaderDirectory(shaderDir)) {
            ERR(scat("shader directory not found: "_k, shaderDir));
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
        bl::string script = sys::fs::basename(opts.binary);
        if (!script.empty())
            script = sys::fs::dropExtension(script);

        if (script.empty() ||
            !commandProcessor().loadScript(scat(script, ".script"_k)))
            return 1;
    }

    for (const auto &i : opts.commands) {
        bool ok;
        bl::string command;
        if (i.first == EngineOptions::Script) {
            ok = commandProcessor().loadScript(i.second);
            if (!ok)
                command = "script"_k;
        } else {
            ok = commandProcessor().evalCommand(i.second);
            if (!ok)
                command = "command"_k;
        }

        if (!ok) {
            ERR(scat(command, " failed: "_k, i.second));
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
Engine::Impl::tick()
{
    events.animate.raise(Event(AnimationEvent(m())));
}

void
Engine::Impl::render(double interpolation)
{
    events.beforeRender.raise(
      Event(RenderEvent(m(), static_cast<real>(interpolation))));
    if (!skipRender) {
        GL_TRACE("BEGIN_SCENE");
        renderManager->beginScene();
        events.render.raise(
          Event(RenderEvent(m(), static_cast<real>(interpolation))));
        renderManager->endScene();
        GL_TRACE("END_SCENE");
    }
    events.afterRender.raise(
      Event(RenderEvent(m(), static_cast<real>(interpolation))));
}

void
Engine::Impl::handleInputEvents()
{
    events.handleInput.raise(Event(InputEvent(m())));
}

GameLoop::time
Engine::Impl::now() const
{
    return static_cast<GameLoop::time>(sys::queryTimer());
}

void
Engine::Impl::sleep(GameLoop::time secs)
{
    sys::sleep(secs);
}

void
Engine::Impl::atExit(int32_t exit_code)
{
    events.exit.raise(Event(ExitEvent(m(), exit_code)));
}

bool
Engine::Impl::init(const EngineOptions &eopts)
{
    opts = &eopts;
    window = bl::new_obj<GameWindow>(eopts.window);
    renderManager->setDefaultRenderTarget(window->renderTarget());
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
Engine::Impl::registerHandlers()
{
    window->registerHandlers(events);

    window->events().windowClosed.reg(
      [this](const Event<WindowEvent> & /*unused*/) { gameLoop.exit(0); });

    window->events().keyChanged.reg([this](const Event<KeyChanged> &ev) {
        keyHandler->keyEvent(ev.info.key);
    });

    window->events().focusChanged.reg([this](const Event<FocusChanged> &ev) {
        if (!ev.info.focused)
            keyHandler->clearStates();
    });

    window->events().mouseButton.reg([this](const Event<MouseButton> &ev) {
        keyHandler->keyEvent(ev.info.button);
    });

    window->events().windowResized.reg([this](const Event<WindowResized> &ev) {
        renderManager->updateProjection(
          static_cast<math::real>(ev.info.width) /
          static_cast<math::real>(ev.info.height));
    });

    events.handleInput.reg([this](const Event<InputEvent> & /*unused*/) {
        keyHandler->handleCommands();
    });
}

void
Engine::enablePlugin(Plugin &p)
{
    p.registerWith(*this);
    p.registerCommands(commandProcessor());
}

void
Engine::setDevelDataDir(const bl::string &dir)
{
    if (!sys::fs::directoryExists(dir)) {
        WARN("not a directory");
        return;
    }
    m().develDataDir = dir;
}

} // namespace ge
