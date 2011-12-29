#include <stdio.h>
#include <string.h>
#include <fstream>
#include <set>
#include <stack>

#include "glt/GLSLPreprocessor.hpp"
#include "glt/ShaderCompiler.hpp"
#include "sys/fs.hpp"

namespace glt {

namespace {

bool readFile(sys::io::OutStream& err, const std::string& fn, char *& file_contents, uint32& file_size) {

    FILE *in = fopen(fn.c_str(), "r");
    if (in == 0)
        goto fail;
    
    file_contents = 0;
    file_size = 0;
    
    if (fseek(in, 0, SEEK_END) == -1)
        goto fail;

    {
        int64 ssize = ftell(in);
        if (ssize < 0)
            goto fail;
        size_t size = size_t(ssize);
        if (fseek(in, 0, SEEK_SET) == -1)
            goto fail;

        {
            char *contents = new char[size + 1];
            if (fread(contents, size, 1, in) != 1) {
                delete[] contents;
                goto fail;
            }
        
            contents[size] = '\0';
            file_contents = contents;
            file_size = static_cast<uint32>(size);
            fclose(in);
        }
    }
    
    return true;

fail:
    
    err << "unable to read file: " << fn << sys::io::endl;
    return false;
}

bool parseFileArg(const Preprocessor::DirectiveContext& ctx, const char *& arg, uint32& len) {

    arg = 0; len = 0;

    const char *begin = ctx.content.data + ctx.endDirective;
    const char *end   = ctx.content.data + ctx.lineOffset + ctx.lineLength;

    while (begin < end && isspace(*begin))
        ++begin;

    if (begin >= end) return false;

    char term = *begin == '"' ? '"' :
                *begin == '<' ? '>' :
                                '\0';

    if (term == 0) return false;

    begin++;
    arg = begin;

    while (begin < end && *begin != term)
        ++begin;

    if (begin < end && arg <= begin) {
        len = uint32(begin - arg);
        return true;
    } else {
        arg = 0;
        return false;
    }
}

} // namespace anon

struct FileContext {
    const char *pos;
};

struct ProcessingState {
    std::set<std::string> visitingFiles;
    std::set<std::string> deps;
    std::set<std::string> incs;
    std::stack<FileContext> stack;
};

GLSLPreprocessor::GLSLPreprocessor(const IncludePath& incPath,
                                   ShaderIncludes& incs,
                                   ShaderDependencies& deps) :
    includePath(incPath), includes(incs), dependencies(deps),
    segLengths(), segments(), contents(),
    state(0), includeHandler(), dependencyHandler()
{
    this->installHandler("include", includeHandler);
    this->installHandler("need", dependencyHandler);
}

GLSLPreprocessor::~GLSLPreprocessor() {
    for (defs::index i = 0; i < SIZE(contents.size()); ++i)
        delete[] contents[size_t(i)];
}

void GLSLPreprocessor::appendString(const std::string& str) {
    if (!str.empty()) {
        char *data = new char[str.length()];
        memcpy(data, str.data(), str.length());
        contents.push_back(data);
        segments.push_back(data);
        segLengths.push_back(uint32(str.length()));
    }
}

void GLSLPreprocessor::addDefines(const PreprocessorDefinitions& defines) {
    for (PreprocessorDefinitions::const_iterator it = defines.begin();
         it != defines.end(); ++it) {
        std::stringstream defn;
        defn << "#define " << it->first << " " << it->second << std::endl;
        appendString(defn.str());
    }
}

void GLSLPreprocessor::advanceSegments(const Preprocessor::DirectiveContext& ctx) {
    FileContext& frame = state->stack.top();

    defs::index seglen = SIZE(ctx.content.data + ctx.lineOffset - frame.pos);

    if (seglen > 0) {
        segments.push_back(frame.pos);
        segLengths.push_back(uint32(seglen));
    }

    frame.pos = ctx.content.data + ctx.lineOffset + ctx.lineLength;
}

void GLSLPreprocessor::processFileRecursively(const std::string& file) {
    if (wasError())
        return;

    ASSERT(sys::fs::isAbsolute(file));
    char *data;
    uint32 size;
    
    if (!readFile(out(), file, data, size)) {
        setError();
        return;
    }

    contents.push_back(data);

    this->name(file);
    process(data, size);
}

void DependencyHandler::directiveEncountered(const Preprocessor::DirectiveContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLPreprocessor&>(ctx.content.processor);
    
    const char *arg;
    uint32 len;

    // FIXME: add an option to specify ShaderType explicitly
    
    if (!parseFileArg(ctx, arg, len)) {
        proc.out() << ctx.content.name
                   << ": #need-directive: invalid parameter" << sys::io::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookup(proc.includePath, file);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot find file: " << file
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    ShaderManager::ShaderType stype;
    if (!ShaderCompiler::guessShaderType(file, &stype)) {
        proc.out() << ctx.content.name
                   << ": #need-directive: cannot guess shader type based on name: " << file
                   << sys::io::endl;
        proc.setError();
        return;
    }

    std::string absPath = sys::fs::absolutePath(realPath);
    if (proc.state->deps.insert(absPath).second)
        proc.dependencies.push_back(makeRef<ShaderSource>(new FileSource(stype, absPath)));
}

void IncludeHandler::beginProcessing(const Preprocessor::ContentContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLPreprocessor&>(ctx.processor);

    if (proc.state == 0)
        proc.state = new ProcessingState;

    if (!ctx.name.empty())
        proc.state->visitingFiles.insert(ctx.name);

    FileContext frame;
    frame.pos = ctx.data;
    proc.state->stack.push(frame);        
}

void IncludeHandler::directiveEncountered(const Preprocessor::DirectiveContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLPreprocessor&>(ctx.content.processor);

    const char *arg;
    uint32 len;

    if (!parseFileArg(ctx, arg, len) || len == 0) {
        proc.out() << ctx.content.name
                   << ": #include-directive: invalid parameter" << sys::io::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookup(proc.includePath, file);
    if (realPath.empty()) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot find file: " << file
                   << sys::io::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    sys::fs::Stat filestat;
    if (!sys::fs::stat(realPath, &filestat)) {
        proc.out() << ctx.content.name
                   << ": #include-directive: cannot open file: " << realPath
                   << sys::io::endl;
        proc.setError();
        return;
    }
    proc.state->incs.insert(filestat.absolute);

    if (proc.state->visitingFiles.count(filestat.absolute) == 0) {
        proc.includes.push_back(ShaderInclude(filestat.absolute, filestat.mtime));
        proc.name(filestat.absolute);

        char *contents;
        uint32 size;

        if (!readFile(proc.out(), filestat.absolute, contents, size)) {
            proc.setError();
            return;
        }

        proc.contents.push_back(contents);
        proc.name(filestat.absolute);
        proc.process(contents, size);
    }
}

void IncludeHandler::endProcessing(const Preprocessor::ContentContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLPreprocessor&>(ctx.processor);

    FileContext& frame = proc.state->stack.top();
    defs::index seglen = SIZE(ctx.data + ctx.size - frame.pos);
    
    if (seglen > 0) {
        proc.segments.push_back(frame.pos);
        proc.segLengths.push_back(uint32(seglen));
    }

    proc.state->stack.pop();

    if (!ctx.name.empty())
        proc.state->visitingFiles.erase(ctx.name);

    if (proc.state->stack.empty()) {
        delete proc.state;
        proc.state = 0;
    }
}

} // namespace glt
