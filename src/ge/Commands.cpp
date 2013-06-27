#define ERROR_NO_IMPLICIT_OUT

#include "ge/Commands.hpp"
#include "ge/Engine.hpp"
#include "ge/CommandParams.hpp"

#include "glt/utils.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/ShaderCompiler.hpp"
#include "glt/GLObject.hpp"

namespace ge {

using namespace math;

namespace {

void runPrintContextInfo(const Event<CommandEvent>& e) {
    ge::GLContextInfo c;
    e.info.engine.window().contextInfo(c);
    e.info.engine.out()
        << "OpenGL Context Information" << sys::io::endl
        << "  Version:\t" << c.majorVersion << "." << c.minorVersion << sys::io::endl
        << "  DepthBits:\t" << c.depthBits << sys::io::endl
        << "  StencilBits:\t" << c.stencilBits << sys::io::endl
        << "  Antialiasing:\t" << c.antialiasingLevel << sys::io::endl
        << "  CoreProfile:\t" << (c.coreProfile ? "yes" : "no") << sys::io::endl
        << "  DebugContext:\t" << (c.debugContext ? "yes" : "no") << sys::io::endl
        << "  VSync:\t" << (e.info.engine.window().vsync() ? "yes" : "no") << sys::io::endl
        << sys::io::endl;
}

void runPrintGLInstanceStats(const Event<CommandEvent>& e) {
    glt::printStats(e.info.engine.out());
}

void runPrintMemInfo(const Event<CommandEvent>& e) {
    bool success = false;
    glt::GLMemInfoATI info;
    if (glt::GLMemInfoATI::info(&info)) {
        success = true;
        float fracVBO = float(info.current.freeVBO) / float(info.initial.freeVBO);
        float fracTex = float(info.current.freeTexture) / float(info.initial.freeTexture);
        float fracRBO = float(info.current.freeRenderbuffer) / float(info.initial.freeRenderbuffer);
        e.info.engine.out()
            << "OpenGL Free Memory: " << sys::io::endl
            << "  VBO: " << info.current.freeVBO << " kbyte (" << (fracVBO * 100) << "%)" << sys::io::endl
            << "  Texture: " << info.current.freeVBO << " kbyte (" << (fracTex * 100) << "%)" << sys::io::endl
            << "  Renderbuffer: " << info.current.freeVBO << " kbyte (" << (fracRBO * 100) << "%)" << sys::io::endl;
    }

    glt::GLMemInfoNV info_nv;
    if (!success && glt::GLMemInfoNV::info(&info_nv)) {
        success = true;

        e.info.engine.out()
            << "OpenGL Memory Information:" << sys::io::endl
            << "  total: " << info_nv.total << " kbyte, dedicated: " << info_nv.total_dedicated << " kbyte" << sys::io::endl
            << "  current: " << info_nv.current << " kbyte" << sys::io::endl
            << "  evicted: " << info_nv.evicted << " kbyte, " << info_nv.num_evictions << " evictions" << sys::io::endl;
    }

    if (!success)
        ERR(e.info.engine.out(), "couldnt query amount of free memory");
}

void runReloadShaders(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    
    if (args.size() == 0) {
        e.info.engine.out() << "reloading shaders" << sys::io::endl;
        e.info.engine.shaderManager().reloadShaders();
        // std::cerr << "all shaders reloaded" << sys::io::endl;
    } else {
        glt::ShaderManager& sm = e.info.engine.shaderManager();
        for (index i = 0; i < args.size(); ++i) {
            Ref<glt::ShaderProgram> prog = sm.program(*args[i].string);
            if (!prog) {
                WARN(e.info.engine.out(), "reloadShaders: " + *args[i].string + " not defined");
            } else {
                if (!prog->reload())
                    WARN(e.info.engine.out(), "reloadShaders: failed to reload program " + *args[i].string);
            }
        }
    }
}

void runListCachedShaders(const Event<CommandEvent>& e) {
    Ref<glt::ShaderCache> cache = e.info.engine.shaderManager().globalShaderCache();
    
    if (!cache) {
        e.info.engine.out() << "no shader cache available" << sys::io::endl;
        return;
    }

    uint32 n = 0;    
    for (glt::ShaderCacheEntries::iterator it = cache->entries.begin();
         it != cache->entries.end(); ++it) {
        e.info.engine.out() << "cached shader: " << it->first << sys::io::endl;
        ++n;
    }

    e.info.engine.out() << n << " shaders cached" << sys::io::endl;
}

void runListBindings(const Event<CommandEvent>& e) {
    e.info.engine.keyHandler().handleListBindings(e);
}

struct BindKey : public Command {
    BindKey() :
        Command(KEY_COM_PARAMS, "bindKey", "bind a command to a key combination") {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        Ref<Command>& comm = *args[1].command.ref;
        if (!comm) {
            ERR(e.info.engine.out(), "cannot bind key: null command");
            return;
        }
        Ref<KeyBinding> binding(new KeyBinding(*args[0].keyBinding));
        e.info.engine.keyHandler().registerBinding(binding, comm);
    }
};

void runHelp(const Event<CommandEvent>& ev) {
    ge::CommandProcessor& cp = ev.info.engine.commandProcessor();
    ev.info.engine.out() << "list of Commands: " << sys::io::endl;
    
    for (CommandMap::const_iterator it = cp.commands.begin();
         it != cp.commands.end(); ++it) {
        ev.info.engine.out() << "    " << it->first << sys::io::endl;
    }

    ev.info.engine.out()
        << sys::io::endl
        << "use describe <command name> to get more information about a specific command" << sys::io::endl;
}

void runBindShader(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    if (args.size() == 0) {
        ERR(e.info.engine.out(), "bindShader: need at least one argument");
        return;
    }

    if (!args[0].type == String) {
        ERR(e.info.engine.out(), "bindShader: first argument not a string");
        return;
    }

    Ref<glt::ShaderProgram> prog(new glt::ShaderProgram(e.info.engine.shaderManager()));

    index i;
    for (i = 1; i < args.size() && args[i].type == String; ++i) {
        if (!prog->addShaderFile(*args[i].string)) {
            ERR(e.info.engine.out(), "bindShader: compilation failed");
            return;
        }
    }

    for (; i + 1 < args.size() && args[i].type == Integer && args[i + 1].type == String; i += 2) {
        if (args[i].integer < 0) {
            ERR(e.info.engine.out(), "bindShader: negative index");
            return;
        }
        if (!prog->bindAttribute(*args[i + 1].string, GLuint(args[i].integer))) {
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

    e.info.engine.out() << "bound program: " << *args[0].string << sys::io::endl;

    e.info.engine.shaderManager().addProgram(*args[0].string, prog);
}

void runInitGLDebug(const Event<CommandEvent>& e) {
    ge::GLContextInfo c;
    e.info.engine.window().contextInfo(c);
    if (c.debugContext) {
        glt::initDebug();
    } else {
        e.info.engine.out() << "cannot initialize OpenGL debug output: no debug context" << sys::io::endl;
    }
}

void runIgnoreGLDebugMessage(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    const std::string& vendor_str = *args[0].string;
    GLint id = static_cast<GLint>(args[1].integer);
    glt::OpenGLVendor vendor;

    if (vendor_str == "Nvidia")
        vendor = glt::glvendor::Nvidia;
    else if (vendor_str == "ATI")
        vendor = glt::glvendor::ATI;
    else if (vendor_str == "Intel")
        vendor = glt::glvendor::Intel;
    else
        vendor = glt::glvendor::Unknown;

    if (vendor != glt::glvendor::Unknown) {
        glt::ignoreDebugMessage(vendor, id);
    } else {
        e.info.engine.out() << "invalid opengl vendor: " << vendor_str;
    }
}

void runDescribe(const Event<CommandEvent>& e, const Array<CommandArg>& args) {

    CommandProcessor& proc = e.info.engine.commandProcessor();
    
    for (index i = 0; i < args.size(); ++i) {
        Ref<Command> com = proc.command(*args[i].string);
        if (!com) {
            e.info.engine.out() << "unknown command: " << *args[i].string << sys::io::endl;
        } else {
            e.info.engine.out() << com->interactiveDescription() << sys::io::endl;
        }
    }
}

void runEval(const Event<CommandEvent>& e, const Array<CommandArg>&) {
    ERR(e.info.engine.out(), "not yet implemented");
}

void runLoad(const Event<CommandEvent>& ev, const Array<CommandArg>& args) {
    for (defs::index i = 0; i < SIZE(args.size()); ++i) {
        ev.info.engine.commandProcessor().loadScript(*args[i].string);
    }
}

void runAddShaderPath(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    for (defs::index i = 0; i < args.size(); ++i)
        if (!e.info.engine.shaderManager().addShaderDirectory(*args[i].string, true))
            ERR(e.info.engine.out(), "not a directory: " + *args[i].string);
}

void runPrependShaderPath(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    for (defs::index i = args.size(); i > 0; --i) {
        if (!e.info.engine.shaderManager().prependShaderDirectory(*args[i - 1].string, true))
            ERR(e.info.engine.out(), "not a directory: " + *args[i - 1].string);
    }
}

void runRemoveShaderPath(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    for (defs::index i = 0; i < args.size(); ++i)
        e.info.engine.shaderManager().removeShaderDirectory(*args[i].string);
}

void runTogglePause(const Event<CommandEvent>& e) {
    Engine& eng = e.info.engine;
    eng.gameLoop().pause(!eng.gameLoop().paused());
}

struct PerspectiveProjection : public Command {
    PerspectiveProjection() :
        Command(NUM_NUM_NUM_PARAMS, "perspectiveProjection", "set parameters for perspective projection") {}

    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        real fovDeg = real(args[0].number);
        real zn = real(args[1].number);
        real zf = real(args[2].number);

        glt::RenderManager& rm = e.info.engine.renderManager();

        rm.setDefaultProjection(glt::Projection::mkPerspective(math::degToRad(fovDeg), zn, zf));
        glt::RenderTarget *tgt = rm.activeRenderTarget();
        if (tgt != 0)
            rm.updateProjection(float(tgt->width()) / float(tgt->height()));
    }
};

struct InitCommandHandler : public EventHandler<InitEvent> {
    Ref<Command> handler;
    InitCommandHandler(const Ref<Command>& hndlr) :
        handler(hndlr) {}
    void handle(const Event<InitEvent>& e) {
        e.info.success = true;
        handler->handle(makeEvent(CommandEvent(e.info.engine, e.info.engine.commandProcessor())));
    }
};

void runPostInit(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    e.info.engine.addInit(PostInit, Ref<EventHandler<InitEvent> >(new InitCommandHandler(*args[0].command.ref)));
}

void runStartReplServer(const Event<CommandEvent>& ev, const Array<CommandArg>& args) {
    Engine& e = ev.info.engine;
    ReplServer& serv = e.replServer();

    if (serv.running()) {
        ERR(ev.info.engine.out(), "REPL server already running");
        return;
    }

    int64 port = args[0].integer;
    uint16 p16 = uint16(port);
    
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

} // namespace anon

Commands::Commands() :
    printContextInfo(makeCommand(runPrintContextInfo,
                                 "printContextInfo",
                                 "prints information about the current OpenGL context")),

    printMemInfo(makeCommand(runPrintMemInfo,
                             "printMemInfo",
                             "prints information about current (approximatly) amout of free memory on the GPU")),

    
    reloadShaders(makeStringListCommand(runReloadShaders,
                                        "reloadShaders", "reload ShaderPrograms")),
    
    listCachedShaders(makeCommand(runListCachedShaders, "listCachedShaders", "list all shader cache entries")),
    
    listBindings(makeCommand(runListBindings, "listBindings", "list all key bindings")),
    
    bindKey(new BindKey),
                                                                             
    help(makeCommand(runHelp, "help", "help and command overview")),
                                                                             
    bindShader(makeListCommand(runBindShader,
                               "bindShader",
                               "compile and linke a ShaderProgram and give it a name")),

    initGLDebug(makeCommand(runInitGLDebug, "initGLDebug", "initialize OpenGL debug output")),

    ignoreGLDebugMessage(makeCommand(runIgnoreGLDebugMessage, STR_INT_PARAMS, "ignoreGLDebugMessage",
                                     "for opengl vendor <vendor> add the message <id> to the list of ignored messages")),

    describe(makeListCommand(runDescribe, "describe", "print a description of its parameters")),

    eval(makeStringListCommand(runEval, "eval", "parse a string and execute it")),

    load(makeStringListCommand(runLoad, "load", "execute a script file")),

    addShaderPath(makeStringListCommand(runAddShaderPath, "addShaderPath", "add directories to the shader path")),

    prependShaderPath(makeStringListCommand(runPrependShaderPath, "prependShaderPath", "add directories to the front of the shader path")),

    removeShaderPath(makeStringListCommand(runRemoveShaderPath, "removeShaderPath", "remove directories from the shader path")),

    togglePause(makeCommand(runTogglePause, "togglePause", "toggle the pause state")),

    perspectiveProjection(new PerspectiveProjection),

    postInit(makeCommand(runPostInit, COM_PARAMS, "postInit", "execute its argument command in the postInit hook")),

    startReplServer(makeCommand(runStartReplServer, INT_PARAMS, "startReplServer", "start a REPL server on the given port")),

    printGLInstanceStats(makeCommand(runPrintGLInstanceStats, "printGLInstanceStats", "print number of the allocated opengl object per type"))

    
{}

const Commands& commands() {
    // static Commands *comms = 0;
    // if (comms == 0) {
    //     comms = new Commands;
    // }

    // return *comms;

    static Commands commands;
    return commands;
}

} // namespace ge

