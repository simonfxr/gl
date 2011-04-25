#include "GLSLPreprocessor.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>

namespace glt {

namespace {

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

ShaderContents::~ShaderContents() {
    for (uint32 i = 0; i < contents.size(); ++i) {
        delete[] contents[i].contents;
    }
}

void DependencyHandler::directiveEncountered(const Preprocessor::DirectiveContext& ctx) {
    const char *arg;
    uint32 len;
    
    if (!parseFileArg(ctx, arg, len)) {
        ctx.content.processor.err()
            << ctx.content.name
            << ": invalid parameter in #need-directive" << std::endl;
        ctx.content.processor.setError();
    }

    deps.push_back(std::string(arg, len));

    Include inc;
    inc.offset = ctx.lineOffset;
    inc.directiveLineLength = ctx.lineLength;
    inc.fileToInclude = "";

    patches->push_back(inc);
}

void IncludeHandler::beginProcessing(const Preprocessor::ContentContext& ctx) {
    UNUSED(ctx);
    includes.clear();
}

void IncludeHandler::directiveEncountered(const Preprocessor::DirectiveContext& ctx) {
    const char *arg;
    uint32 len;

    if (!parseFileArg(ctx, arg, len)) {
        ctx.content.processor.err()
            << ctx.content.name
            << ": invalid parameter in #include-directive" << std::endl;
        ctx.content.processor.setError();
    }

    Include inc;
    inc.offset = ctx.lineOffset;
    inc.directiveLineLength = ctx.lineLength;
    inc.fileToInclude = std::string(arg, len);

    includes.push_back(inc);
}

bool preprocess(const ShaderManager& sm, Preprocessor& proc, const std::string& file, std::vector<Include> *includeBuffer, ShaderContents& shadersrc) {

    std::auto_ptr<std::vector<Include> > localIncBuf(0);

    if (includeBuffer == 0) {
        includeBuffer = new std::vector<Include>;
        localIncBuf = std::auto_ptr<std::vector<Include> >(includeBuffer);
    }

    char *contents;
    uint32 size;
    std::string name = sm.readFile(file, contents, size);
    if (contents == 0)
        return false;

    {
        FileContents fc; fc.contents = contents; fc.size = size;
        shadersrc.contents.push_back(fc);
        fc.contents = 0; // dont let the destructor delete our content
        fc.size = 0;
    }

    proc.process(contents, size);
    if (proc.wasError())
        return false;

    std::vector<Include> includes = *includeBuffer;
    const char *pos = contents;
    includeBuffer->clear();

    for (uint32 i = 0; i < includes.size(); ++i) {
        Include& inc = includes[i];

        uint32 seglen = contents + inc.offset - pos;
        if (seglen > 0) {
            shadersrc.segments.push_back(pos);
            shadersrc.segLengths.push_back(seglen);
        }
        
        pos = contents + inc.offset + inc.directiveLineLength;
        
        if (!inc.fileToInclude.empty()) {
            std::string realname = sm.lookupPath(inc.fileToInclude);
            if (realname.empty() || !preprocess(sm, proc, realname, includeBuffer, shadersrc))
                return false;
        }
    }

    if (pos < contents + size) {
        shadersrc.segments.push_back(pos);
        shadersrc.segLengths.push_back(contents + size - pos);
    }

    return true;
}

} // namespace glt
