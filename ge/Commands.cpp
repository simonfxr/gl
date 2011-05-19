#include "ge/Commands.hpp"
#include "ge/Engine.hpp"

#include "glt/ShaderProgram.hpp"

namespace ge {

namespace commands {

typedef void (*CommandHandler)(const Event<CommandEvent>&);

struct SimpleCommand : public Command {
private:
    CommandHandler handler;
public:
    SimpleCommand(CommandHandler hndlr, const std::string& desc = "") :
        Command(NULL_PARAMS, desc), handler(hndlr) {}
    void handle(const Event<CommandEvent>& e) { handler(e); }
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>&) {
        handler(e);
    }
};


Ref<Command> makeCommand(CommandHandler handler, const std::string& desc = "") {
    return Ref<Command>(new SimpleCommand(handler, desc));
}

typedef void (*ListCommandHandler)(const Event<CommandEvent>&, const Array<CommandArg>&);

DEFINE_CONST_ARRAY(MANY_PARAMS, CommandParamType, ListParam);

struct StringListCommand : public Command {
private:
    ListCommandHandler handler;
public:
    StringListCommand(ListCommandHandler hndlr, const std::string& desc = "") :
        Command(MANY_PARAMS, desc), handler(hndlr) {}
    void handle(const Event<CommandEvent>& e) { handler(e, NULL_ARGS); }
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        for (uint32 i = 0; i < args.size(); ++i) {
            if (args[i].type != String) {
                ERR("expected String argument");
                return;
            }
        }
        
        handler(e, args);
    }
};

Ref<Command> makeStringListCommand(ListCommandHandler handler, const std::string& desc = "") {
    return Ref<Command>(new StringListCommand(handler, desc));
}

struct ListCommand : public Command {
private:
    ListCommandHandler handler;
public:
    ListCommand(ListCommandHandler hndlr, const std::string& desc = "") :
        Command(MANY_PARAMS, desc), handler(hndlr) {}
    void handle(const Event<CommandEvent>& e) { handler(e, NULL_ARGS); }
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        handler(e, args);
    }
};

Ref<Command> makeListCommand(ListCommandHandler handler, const std::string& desc = "") {
    return Ref<Command>(new ListCommand(handler, desc));
}

static void runPrintContextInfo(const Event<CommandEvent>& e) {
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

const Ref<Command> printContextInfo = makeCommand(runPrintContextInfo,
                                                  "prints information about the current OpenGL context");

static void runReloadShaders(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    if (args.size() == 0) {
        e.info.engine.shaderManager().reloadShaders();
    } else {
        ERR("reloadShaders: selectively shader reloading not yet implemented");
    }
}

const Ref<Command> reloadShaders = makeStringListCommand(runReloadShaders, "reload ShaderPrograms");

const Ref<Command> listBindings;

const Ref<Command> bindKey;

const Ref<Command> help;

static void runBindShader(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
    if (args.size() == 0) {
        ERR("bindShader: need at least one argument");
        return;
    }

    if (!args[1].type == String) {
        ERR("bindShader: first argument not a string");
        return;
    }

    Ref<glt::ShaderProgram> prog(new glt::ShaderProgram(e.info.engine.shaderManager()));

    uint32 i;
    for (i = 0; i < args.size() && args[i].type == String; ++i) {
        if (!prog->addShaderFile(*args[i].string)) {
            ERR("bindShader: compilation failed");
            return;
        }
    }

    for (; i + 1 < args.size() && args[i].type == Integer && args[i + 1].type == String; i += 2) {
        if (!prog->bindAttribute(*args[i].string, args[i].integer)) {
            ERR("bindShader: couldnt bind attribute");
            return;
        }
    }

    if (i != args.size()) {
        ERR("bindShader: invalid argument");
        return;
    }

    if (!prog->tryLink()) {
        ERR("bindShader: linking failed");
        return;
    }

    e.info.engine.shaderManager().addProgram(*args[0].string, prog);
}

const Ref<Command> bindShader = makeStringListCommand(runBindShader,
                                                      "compile and linke a ShaderProgram and give it a name");

struct InteractiveCommand : public Command {
    void handle(const Event<CommandEvent>&) { ERR("cannot execute Command non interactively"); }
    InteractiveCommand(const Array<CommandParamType>& ts, const std::string& desc = "") :
        Command(ts, desc) {}
};


DEFINE_CONST_ARRAY(scriptParams, CommandParamType, StringParam);

struct ScriptCommand : public InteractiveCommand {
    ScriptCommand() :
        InteractiveCommand(scriptParams, "load and execute a script file") {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        e.info.engine.loadScript(*args[0].string);
    }
};

const Ref<Command> loadScript = Ref<Command>(new ScriptCommand);

} // namespace commands

} // namespace ge

