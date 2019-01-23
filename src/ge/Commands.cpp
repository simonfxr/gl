#include "ge/Commands.hpp"

#include "bl/shared_ptr.hpp"
#include "bl/vector.hpp"
#include "ge/Engine.hpp"
#include "glt/GLObject.hpp"
#include "glt/ShaderCompiler.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/utils.hpp"
#include "util/string.hpp"

#include <cassert>

namespace ge {

using namespace math;

namespace {

struct CommandDecl
{
    const char *name;
    const char *descr;
};

template<typename F>
inline bl::shared_ptr<Command>
operator+(CommandDecl &&decl, F f)
{
    return makeCommand(decl.name, decl.descr, decay_stateless_functor(f));
}

struct InitCommandHandler : public EventHandler<InitEvent>
{
    CommandPtr handler;
    explicit InitCommandHandler(CommandPtr hndlr) : handler(std::move(hndlr)) {}
    void handle(const Event<InitEvent> &e) override
    {
        e.info.success = true;
        handler->handle(
          Event(CommandEvent(e.info.engine, e.info.engine.commandProcessor())));
    }
};

#define BEGIN_COMMANDS                                                         \
    bl::array_view<const bl::shared_ptr<Command>> predefinedCommands()         \
    {                                                                          \
        BEGIN_NO_WARN_GLOBAL_DESTRUCTOR static const bl::shared_ptr<Command>   \
          predefinedCommandArray[] = {

#define END_COMMANDS                                                           \
    }                                                                          \
    ;                                                                          \
    END_NO_WARN_GLOBAL_DESTRUCTOR return bl::array_view<                       \
      const bl::shared_ptr<Command>>(predefinedCommandArray,                   \
                                     ARRAY_LENGTH(predefinedCommandArray));    \
    }

#define COMMAND0(name, descr) CommandDecl{ name, descr } + []

#define COMMAND(name, descr) , COMMAND0(name, descr)

BEGIN_COMMANDS

COMMAND0("printContextInfo",
         "prints information about the current OpenGL context")
(const Event<CommandEvent> &e)
{
    auto c = e.info.engine.window().contextInfo();
    e.info.engine.out() << "OpenGL Context Information" << sys::io::endl
                        << "  Version:\t" << c.majorVersion << "."
                        << c.minorVersion << sys::io::endl
                        << "  DepthBits:\t" << c.depthBits << sys::io::endl
                        << "  StencilBits:\t" << c.stencilBits << sys::io::endl
                        << "  Antialiasing:\t" << c.antialiasingLevel
                        << sys::io::endl
                        << "  CoreProfile:\t" << (c.coreProfile ? "yes" : "no")
                        << sys::io::endl
                        << "  DebugContext:\t"
                        << (c.debugContext ? "yes" : "no") << sys::io::endl
                        << "  VSync:\t"
                        << (e.info.engine.window().vsync() ? "yes" : "no")
                        << sys::io::endl
                        << sys::io::endl;
} // namespace

COMMAND("printMemInfo",
        "prints information about current (approximatly) amout of free memory "
        "on the GPU")
(const Event<CommandEvent> &e)
{
    bool success = false;
    glt::GLMemInfoATI info{};
    if (glt::GLMemInfoATI::info(&info)) {
        success = true;
        math::real fracVBO =
          math::real(info.current.freeVBO) / math::real(info.initial.freeVBO);
        math::real fracTex = math::real(info.current.freeTexture) /
                             math::real(info.initial.freeTexture);
        math::real fracRBO = math::real(info.current.freeRenderbuffer) /
                             math::real(info.initial.freeRenderbuffer);
        e.info.engine.out()
          << "OpenGL Free Memory: " << sys::io::endl
          << "  VBO: " << info.current.freeVBO << " kbyte (" << (fracVBO * 100)
          << "%)" << sys::io::endl
          << "  Texture: " << info.current.freeVBO << " kbyte ("
          << (fracTex * 100) << "%)" << sys::io::endl
          << "  Renderbuffer: " << info.current.freeVBO << " kbyte ("
          << (fracRBO * 100) << "%)" << sys::io::endl;
    }

    glt::GLMemInfoNV info_nv{};
    if (!success && glt::GLMemInfoNV::info(&info_nv)) {
        success = true;

        e.info.engine.out()
          << "OpenGL Memory Information:" << sys::io::endl
          << "  total: " << info_nv.total
          << " kbyte, dedicated: " << info_nv.total_dedicated << " kbyte"
          << sys::io::endl
          << "  current: " << info_nv.current << " kbyte" << sys::io::endl
          << "  evicted: " << info_nv.evicted << " kbyte, "
          << info_nv.num_evictions << " evictions" << sys::io::endl;
    }

    if (!success)
        ERR(e.info.engine.out(), "couldnt query amount of free memory");
}

COMMAND("printGLInstanceStats",
        "print number of the allocated opengl object per type")
(const Event<CommandEvent> &e)
{
    glt::printStats(e.info.engine.out());
}

COMMAND("reloadShaders", "reload ShaderPrograms")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{

    if (args.size() == 0) {
        e.info.engine.out() << "reloading shaders" << sys::io::endl;
        e.info.engine.shaderManager().reloadShaders();
    } else {
        glt::ShaderManager &sm = e.info.engine.shaderManager();
        for (size_t i = 0; i < args.size(); ++i) {
            auto prog = sm.program(args[i].string);
            if (!prog) {
                WARN(e.info.engine.out(),
                     string_concat(
                       "reloadShaders: ", args[i].string, " not defined"));
            } else {
                if (!prog->reload())
                    WARN(
                      e.info.engine.out(),
                      string_concat("reloadShaders: failed to reload program ",
                                    args[i].string));
            }
        }
    }
}

COMMAND("listCachedShaders", "list all shader cache entries")
(const Event<CommandEvent> &e)
{
    auto cache = e.info.engine.shaderManager().globalShaderCache();

    if (!cache) {
        e.info.engine.out() << "no shader cache available" << sys::io::endl;
        return;
    }

    uint32_t n = 0;
    for (auto &ent : cache->cacheEntries()) {
        e.info.engine.out() << "cached shader: " << ent.key << sys::io::endl;
        ++n;
    }

    e.info.engine.out() << n << " shaders cached" << sys::io::endl;
}

COMMAND("listBindings", "list all key bindings")
(const Event<CommandEvent> &e)
{
    e.info.engine.keyHandler().handleListBindings(e);
}

COMMAND("bindKey", "bind a command to a key combination")
(const Event<CommandEvent> &e,
 const KeyBinding &keyBinding,
 const bl::shared_ptr<Command> &comm)
{
    if (!comm) {
        ERR(e.info.engine.out(), "cannot bind key: null command");
        return;
    }
    e.info.engine.keyHandler().registerBinding(
      bl::make_shared<KeyBinding>(keyBinding), comm);
}

COMMAND("help", "help and command overview")(const Event<CommandEvent> &ev)
{
    ge::CommandProcessor &cp = ev.info.engine.commandProcessor();
    ev.info.engine.out() << "list of Commands: " << sys::io::endl;

    for (const auto &com : cp.commands())
        ev.info.engine.out() << "    " << com->name() << sys::io::endl;

    ev.info.engine.out() << sys::io::endl
                         << "use describe <command name> to get more "
                            "information about a specific command"
                         << sys::io::endl;
}

COMMAND("bindShader", "compile and linke a ShaderProgram and give it a name")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{
    if (args.size() == 0) {
        ERR(e.info.engine.out(), "bindShader: need at least one argument");
        return;
    }

    if (args[0].type() != CommandArgType::String) {
        ERR(e.info.engine.out(), "bindShader: first argument not a string");
        return;
    }

    auto prog =
      bl::make_shared<glt::ShaderProgram>(e.info.engine.shaderManager());

    size_t i;
    for (i = 1; i < args.size() && args[i].type() == CommandArgType::String;
         ++i) {
        if (!prog->addShaderFile(args[i].string)) {
            ERR(e.info.engine.out(), "bindShader: compilation failed");
            return;
        }
    }

    for (; i + 1 < args.size() && args[i].type() == CommandArgType::Integer &&
           args[i + 1].type() == CommandArgType::String;
         i += 2) {
        if (args[i].integer < 0) {
            ERR(e.info.engine.out(), "bindShader: negative size_t");
            return;
        }
        if (!prog->bindAttribute(args[i + 1].string, GLuint(args[i].integer))) {
            ERR(e.info.engine.out(), "bindShader: couldnt bind attribute");
            return;
        }
    }

    if (i != args.size()) {
        ERR(e.info.engine.out(), "bindShader: invalid argument");
        return;
    }

    if (prog->wasError() || !prog->tryLink()) {
        ERR(e.info.engine.out(), "bindShader: linking failed");
        return;
    }

    e.info.engine.out() << "bound program: " << args[0].string << sys::io::endl;

    e.info.engine.shaderManager().addProgram(args[0].string, prog);
}

COMMAND("initGLDebug", "initialize OpenGL debug output")
(const Event<CommandEvent> &e)
{
    if (e.info.engine.window().contextInfo().debugContext) {
        glt::initDebug();
    } else {
        e.info.engine.out()
          << "cannot initialize OpenGL debug output: no debug context"
          << sys::io::endl;
    }
}

COMMAND("ignoreGLDebugMessage",
        "for opengl vendor <vendor> add the message <id> to the list of "
        "ignored messages")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{
    const bl::string &vendor_str = args[0].string;
    auto id = static_cast<GLint>(args[1].integer);
    glt::OpenGLVendor vendor;

    if (vendor_str == "Nvidia")
        vendor = glt::OpenGLVendor::Nvidia;
    else if (vendor_str == "ATI")
        vendor = glt::OpenGLVendor::ATI;
    else if (vendor_str == "Intel")
        vendor = glt::OpenGLVendor::Intel;
    else
        vendor = glt::OpenGLVendor::Unknown;

    if (vendor != glt::OpenGLVendor::Unknown) {
        glt::ignoreDebugMessage(vendor, id);
    } else {
        e.info.engine.out() << "invalid opengl vendor: " << vendor_str;
    }
}

COMMAND("describe", "print a description of its parameters")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{

    CommandProcessor &proc = e.info.engine.commandProcessor();

    for (const auto &arg : args) {
        CommandPtr com = proc.command(arg.string);
        if (!com) {
            e.info.engine.out()
              << "unknown command: " << arg.string << sys::io::endl;
        } else {
            e.info.engine.out()
              << com->interactiveDescription() << sys::io::endl;
        }
    }
}

COMMAND("eval", "parse a string and execute it")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> /*unused*/)
{
    ERR(e.info.engine.out(), "not yet implemented");
}

COMMAND("load", "execute a script file")
(const Event<CommandEvent> &ev, bl::array_view<const CommandArg> args)
{
    for (const auto &arg : args)
        ev.info.engine.commandProcessor().loadScript(arg.string);
}

COMMAND("addShaderPath", "add directories to the shader path")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{
    for (const auto &arg : args)
        if (!e.info.engine.shaderManager().addShaderDirectory(arg.string, true))
            ERR(e.info.engine.out(), "not a directory: " + arg.string);
}

COMMAND("prependShaderPath", "add directories to the front of the shader path")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{
    for (size_t i = args.size(); i > 0; --i) {
        if (!e.info.engine.shaderManager().prependShaderDirectory(
              args[i - 1].string, true))
            ERR(e.info.engine.out(), "not a directory: " + args[i - 1].string);
    }
}

COMMAND("removeShaderPath", "remove directories from the shader path")
(const Event<CommandEvent> &e, bl::array_view<const CommandArg> args)
{
    for (const auto &arg : args)
        e.info.engine.shaderManager().removeShaderDirectory(arg.string);
}

COMMAND("togglePause", "toggle the pause state")
(const Event<CommandEvent> &e)
{
    Engine &eng = e.info.engine;
    eng.gameLoop().pause(!eng.gameLoop().paused());
}

COMMAND("perspectiveProjection", "set parameters for perspective projection")
(const Event<CommandEvent> &e, double fovDeg, double zn, double zf)
{
    glt::RenderManager &rm = e.info.engine.renderManager();

    rm.setDefaultProjection(glt::Projection::mkPerspective(
      math::degToRad(math::real(fovDeg)), math::real(zn), math::real(zf)));
    glt::RenderTarget *tgt = rm.activeRenderTarget();
    if (tgt != nullptr)
        rm.updateProjection(math::real(tgt->width()) /
                            math::real(tgt->height()));
}

COMMAND("postInit", "execute its argument command in the postInit hook")
(const Event<CommandEvent> &e, const bl::shared_ptr<Command> &comm)
{
    e.info.engine.addInit(PostInit, bl::make_shared<InitCommandHandler>(comm));
}

COMMAND("startReplServer", "start a REPL server on the given port")
(const Event<CommandEvent> &ev, bl::array_view<const CommandArg> args)
{
    Engine &e = ev.info.engine;
    ReplServer &serv = e.replServer();

    if (serv.running()) {
        ERR(ev.info.engine.out(), "REPL server already running");
        return;
    }

    auto port = args[0].integer;
    auto p16 = uint16_t(port);

    if (port < 0 || p16 != port) {
        ERR(ev.info.engine.out(), "invalid port");
        return;
    }

    if (!serv.start(sys::io::IPA_LOCAL, p16)) {
        ERR(ev.info.engine.out(), "failed to start REPL server");
        return;
    }

    INFO(ev.info.engine.out(), "REPL server started");

    e.events().handleInput.reg(serv.ioHandler());
}

END_COMMANDS
} // namespace

void
registerCommands(CommandProcessor &proc)
{
    for (const auto &comm : predefinedCommands()) {
        assert(comm);
        assert(comm->name().size() > 0);
        proc.define(comm);
    }

    // auto &out = sys::io::stdout();
    // out << "registeredCommands: " << sys::io::endl;
    // auto coms = proc.commands();
    // for (auto& com : coms) {
    //     out << com->name() << ", nparams: " <<
    // }
}

} // namespace ge
