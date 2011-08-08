#include "glt/GLSLPreprocessor.hpp"
#include "sys/fs/fs.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <ifstream>
#include <set>
#include <stack>

namespace glt {

namespace {

bool readFile(std::ostream& err, const std::string& fn, char *& file_contents, uint32& file_size) {

    FILE *in = fopen(fn.c_str(), "r");
    if (in == 0)
        goto fail;
    
    file_contents = 0;
    file_size = 0;
    
    if (fseek(in, 0, SEEK_END) == -1)
        goto fail;

    {
        int64 size = ftell(in);
        if (size < 0)
            goto fail;
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
    
    err << "unable to read file: " << fn << std::endl;
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
                                0;

    if (term == 0) return false;

    begin++;
    arg = begin;

    while (begin < end && *begin != term)
        ++begin;

    if (begin < end && arg <= begin) {
        len = begin - arg;
        return true;
    } else {
        arg = 0;
        return false;
    }
}

} // namespace anon

struct FileContenxt {
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
    includePath(_incPath), includes(incs), dependencies(deps),
    segLengths(), segments(), fileContents(),
    state(0), includeHandler(), dependencyHandler()
{
    this->installHandler("include", includeHandler);
    this->installHandler("need", dependencyHandler);
}

GLSLPreprocessor::~GLSLPreprocessor() {
    for (index_t i = 0; i < fileContents.size(); ++i)
        delete[] fileContents[i].contents;
}

void GLSLPreprocessor::appendString(const std::string& str) {
    if (!str.empty()) {
        fileContents.push_back(FileContents(strdup(str.c_str()), str.length));
        segments.push_back(fc.contents);
        segLengths.push_back(str.length());
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

bool GLSLPreprocessor::processRecursively(const std::string& src) {
    processRecursively(src.c_str(), src.length());
}

bool GLSLPreprocessor::processRecursively(const char *contents, uint32 size) {

    if (this->wasError())
        return false;
    
    ASSERT(visitingFiles == 0);
    
    std::set<std::string> visiting;
    visitingFiles = &visiting;    
    bool success = process(contents, size);
    visitingFiles = 0;
    return success;
}

void GLSLPreprocessor::advanceSegments(const Preprocessor::DirectiveContext& ctx) {
    FileContext& frame = proc.state->stack.top();

    int32 seglen = ctx.content.data + ctx.lineOffset;

    if (seglen > 0) {
        segments.push_back(frame.pos);
        segLengths.push_back(seglen);
    }

    frame.pos = ctx.content.data + ctx.lineOffset + ctx.lineLength;
}

bool GLSLPreprocessor::processFileRecursively(const std::string& file, std::string *absolute, sys::fs::MTime *mtime) {

    if (this->wasError())
        return false;

    sys::fs::Stat filestat = sys::fs::stat(file);

    if (mtime != 0)
        *mtime = filestat.mtime;

    const char *contents;
    uint32 size;
    
    if (!readFile(this->err(), file, contents, size))
        return false;

    fileContents.push_back(FileContents(contents, size));

    this->name(filestat.absolute);
    return processRecursively(contents, size);
}

void DependencyHandler::directiveEncountered(const Preprocessor::DirectiveContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLProcessor&>(ctx.processor);
    
    const char *arg;
    uint32 len;

    // FIXME: add an option to specify ShaderType explicitly
    
    if (!parseFileArg(ctx, arg, len)) {
        proc.err() << ctx.content.name
                   << ": #need-directive: invalid parameter" << std::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookupPath(proc.includePath, file);
    if (realPath.empty()) {
        proc.err() << ctx.content.name
                   << ": #need-directive: cannot find file: " << file
                   << std::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    ShaderProgram::ShaderType stype;
    if (!ShaderCompiler::guessShaderType(file, &stype)) {
        proc.err() << ctx.content.name
                   << ": #need-directive: cannot guess shader type based on name: " << file
                   << std::endl;
        proc.setError();
        return;
    }

    std::string absPath = sys::fs::absolutePath(realPath);
    if (proc.state->deps.insert(absPath).second)
        proc.dependencies.push_back(ShaderDependency(stype, absPath));
}

void IncludeHandler::beginProcessing(const ContentContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLProcessor&>(ctx.processor);

    if (proc.state == 0)
        proc->state = new ProcessingState;

    if (!ctx.name.empty())
        proc.state->visitingFiles.insert(ctx.name);

    FileContext frame;
    frame.pos = ctx.data;
    proc.state->stack.push(frame);        
}

void IncludeHandler::directiveEncountered(const Preprocessor::DirectiveContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLProcessor&>(ctx.processor);

    const char *arg;
    uint32 len;

    if (!parseFileArg(ctx, arg, len) || len == 0) {
        proc << ctx.content.name
             << ": #include-directive: invalid parameter" << std::endl;
        proc.setError();
        return;
    }

    std::string file(arg, len);
    std::string realPath = sys::fs::lookupPath(proc.includePath, file);
    if (realPath.empty()) {
        proc.err() << ctx.content.name
                   << ": #include-directive: cannot find file: " << file
                   << std::endl;
        proc.setError();
        return;
    }

    proc.advanceSegments(ctx);

    sys::fs::Stat filestat = sys::fs::stat(realPath);
    proc.state->incs.insert(filestat.absolute);

    if (proc.state->visitingFiles.count(filestat.absolute) == 0) {
        proc.includes.push_back(ShaderInclude(filestat.absolute, filestat.mtime));
        proc.name(filestat.absolute);

        char *contents;
        uint32 size;

        if (!readFile(proc.err(), filestat.absolute, contents, size)) {
            proc.setError();
            return;
        }

        proc.fileContents.push_back(FileContents(contents, size));
        proc.name(filestat.absolute);
        proc.process(contents, size);
    }
}

void IncludeHandler::endProcessing(const ContentContext& ctx) {
    GLSLPreprocessor& proc = static_cast<GLSLProcessor&>(ctx.processor);

    FileContext& frame = proc.state->stack.top();
    int32 seglen = ctx.data + ctx.size - frame.pos;
    
    if (seglen > 0) {
        segments.push_back(frame.pos);
        segLengths.push_back(seglen);
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
