#include <iostream>

#include "ge/Commands.hpp"
#include "ge/Engine.hpp"
#include "ge/CommandParams.hpp"

#include "glt/utils.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/ShaderCompiler.hpp"

namespace ge {

namespace {

void runPrintContextInfo(const Event<CommandEvent>& e) {
    const sf::ContextSettings& c = e.info.engine.window().window().GetSettings();
    std::cerr << "OpenGL Context Information"<< std::endl
              << "  Version:\t" << c.MajorVersion << "." << c.MinorVersion << std::endl
              << "  DepthBits:\t" << c.DepthBits << std::endl
              << "  StencilBits:\t" << c.StencilBits << std::endl
              << "  Antialiasing:\t" << c.AntialiasingLevel << std::endl
              << "  CoreProfile:\t" << (c.CoreProfile ? "yes" : "no") << std::endl
              << "  DebugContext:\t" << (c.DebugContext ? "yes" : "no") << std::endl
              << std::endl;
}

void runPrintMemInfo(const Event<CommandEvent>&) {
    glt::GLMemInfo info;
    if (!glt::getMemInfo(&info)) {
        ERR("couldnt query amount of free memory");
        return;
    }

    float fracVBO = float(info.current.freeVBO) / float(info.initial.freeVBO);
    float fracTex = float(info.current.freeTexture) / float(info.initial.freeTexture);
    float fracRBO = float(info.current.freeRenderbuffer) / float(info.initial.freeRenderbuffer);
    std::cerr << "OpenGL Free Memory: " << std::endl
              << "  VBO: " << info.current.freeVBO << " kbyte (" << (fracVBO * 100) << "%)" << std::endl
              << "  Texture: " << info.current.freeVBO << " kbyte (" << (fracTex * 100) << "%)" << std::endl
              << "  Renderbuffer: " << info.current.freeVBO << " kbyte (" << (fracRBO * 100) << "%)" << std::endl;
}

void runReloadShaders(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    if (args.size() == 0) {
        std::cerr << "reloading shaders" << std::endl;
        e.info.engine.shaderManager().reloadShaders();
        // std::cerr << "all shaders reloaded" << std::endl;
    } else {
        ERR("reloadShaders: selectively shader reloading not yet implemented");
    }
}

void runListCachedShaders(const Event<CommandEvent>& e) {
    Ref<glt::ShaderCache> cache = e.info.engine.shaderManager().globalShaderCache();
    
    if (!cache) {
        std::cerr << "no shader cache available" << std::endl;
        return;
    }

    uint32 n = 0;    
    for (glt::ShaderCacheEntries::iterator it = cache->entries.begin();
         it != cache->entries.end(); ++it) {
        std::cerr << "cached shader: " << it->first << std::endl;
        ++n;
    }

    std::cerr << n << " shaders cached" << std::endl;
}

void runListBindings(const Event<CommandEvent>&) {
    ERR("list bindings not yet implemented");
}

struct BindKey : public Command {
    BindKey() :
        Command(KEY_COM_PARAMS, "bindKey", "bind a command to a key combination") {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        Ref<Command>& comm = *args[1].command.ref;
        if (!comm) {
            ERR("cannot bind key: null command");
            return;
        }
        KeyBinding bind = args[0].keyBinding->clone();
        bind.setDelete(false);
        Ref<KeyBinding> binding(new KeyBinding(&bind[0], bind.size(), KeyBinding::Owned));
        e.info.engine.keyHandler().registerBinding(binding, comm);
    }
};

void runHelp(const Event<CommandEvent>&) {
    ERR("help not yet implemented");
}

void runBindShader(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    if (args.size() == 0) {
        ERR("bindShader: need at least one argument");
        return;
    }

    if (!args[0].type == String) {
        ERR("bindShader: first argument not a string");
        return;
    }

    Ref<glt::ShaderProgram> prog(new glt::ShaderProgram(e.info.engine.shaderManager()));

    index i;
    for (i = 1; i < args.size() && args[i].type == String; ++i) {
        if (!prog->addShaderFile(*args[i].string)) {
            ERR("bindShader: compilation failed");
            return;
        }
    }

    for (; i + 1 < args.size() && args[i].type == Integer && args[i + 1].type == String; i += 2) {
        if (args[i].integer < 0) {
            ERR("bindShader: negative index");
        }
        if (!prog->bindAttribute(*args[i + 1].string, GLuint(args[i].integer))) {
            ERR("bindShader: couldnt bind attribute");
            return;
        }
    }

    if (i != args.size()) {
        ERR("bindShader: invalid argument");
        return;
    }

    if (prog->wasError() || !prog->tryLink()) {
        ERR("bindShader: linking failed");
        return;
    }

    std::cerr << "bound program: " << *args[0].string << std::endl;

    e.info.engine.shaderManager().addProgram(*args[0].string, prog);
}

void runInitGLDebug(const Event<CommandEvent>& e) {
    if (e.info.engine.window().window().GetSettings().DebugContext) {
        glt::initDebug();
    } else {
        std::cerr << "cannot initialize OpenGL debug output: no debug context" << std::endl;
    }
}

void runDescribe(const Event<CommandEvent>&, const Array<CommandArg>&) {
    ERR("not yet implemented");
}

void runEval(const Event<CommandEvent>&, const Array<CommandArg>&) {
    ERR("not yet implemented");
}

void runLoad(const Event<CommandEvent>& ev, const Array<CommandArg>& args) {
    for (defs::index i = 0; i < SIZE(args.size()); ++i) {
        ev.info.engine.loadScript(*args[i].string);
    }
}

void runAddShaderPath(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    for (defs::index i = 0; i < args.size(); ++i)
        if (!e.info.engine.shaderManager().addShaderDirectory(*args[i].string, true))
            ERR("not a directory: " + *args[i].string);
}

void runPrependShaderPath(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    for (defs::index i = args.size(); i > 0; --i) {
        if (!e.info.engine.shaderManager().prependShaderDirectory(*args[i - 1].string, true))
            ERR("not a directory: " + *args[i - 1].string);
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

    describe(makeListCommand(runDescribe, "describe", "print a description of its parameters")),

    eval(makeStringListCommand(runEval, "eval", "parse a string and execute it")),

    load(makeStringListCommand(runLoad, "load", "execute a script file")),

    addShaderPath(makeStringListCommand(runAddShaderPath, "addShaderPath", "add directories to the shader path")),

    prependShaderPath(makeStringListCommand(runPrependShaderPath, "prependShaderPath", "add directories to the front of the shader path")),

    removeShaderPath(makeStringListCommand(runRemoveShaderPath, "removeShaderPath", "remove directories from the shader path")),

    togglePause(makeCommand(runTogglePause, "togglePause", "toggle the pause state")),

    perspectiveProjection(new PerspectiveProjection),

    postInit(makeCommand(runPostInit, COM_PARAMS, "postInit", "execute its argument command in the postInit hook"))
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

