#include "glt/GLSLPreprocessor.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>

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

} // namespace anon

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

bool preprocess(const ShaderManager& sm, Preprocessor& proc, const std::string& file, std::vector<Include> *includeBuffer, const std::string& prefixSrc, ShaderContents& shadersrc) {

    std::auto_ptr<std::vector<Include> > localIncBuf(0);

    if (includeBuffer == 0) {
        includeBuffer = new std::vector<Include>;
        localIncBuf = std::auto_ptr<std::vector<Include> >(includeBuffer);
    }

    char *contents;
    uint32 size;
    
    if (!readFile(sm.err(), file, contents, size))
        return false;

    if (!prefixSrc.empty()) {
        FileContents fc;
        fc.contents = strdup(prefixSrc.c_str());
        fc.size = prefixSrc.length();
        shadersrc.segments.push_back(fc.contents);
        shadersrc.segLengths.push_back(prefixSrc.length());
        shadersrc.contents.push_back(fc);
        fc.contents = 0;
        fc.size = 0;
    }

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
            
            if (realname.empty()) {
                ERR(("couldnt include file: not found: " + inc.fileToInclude).c_str());
                return false;
            }
            
            if (!preprocess(sm, proc, realname, includeBuffer, "", shadersrc))
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
