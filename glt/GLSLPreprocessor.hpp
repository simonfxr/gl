#ifndef GLT_GLSLPREPROCESSOR_HPP
#define GLT_GLSLPREPROCESSOR_HPP

#include <vector>
#include <string>
#include <ostream>
#include <memory>

#include "glt/Preprocessor.hpp"
#include "glt/ShaderManager.hpp"

namespace glt {

struct FileContents {
    const char *contents;
    uint32 size;
};

struct ShaderContents {
    std::vector<uint32> segLengths;
    std::vector<const char *> segments;

    std::vector<FileContents> contents;

    ~ShaderContents();
};

struct Include {
    uint32 offset;
    uint32 directiveLineLength;

    // if the string is empty no file gets included,
    // as a side effect the range [offset;offset+directiveLineLength)
    // will not be included in the resulting ShaderContents
    std::string fileToInclude;    
};

struct IncludeHandler : public Preprocessor::DirectiveHandler {
    std::vector<Include> includes;
    
    void beginProcessing(const Preprocessor::ContentContext& proc);
    void directiveEncountered(const Preprocessor::DirectiveContext& ctx);
};

struct DependencyHandler : public Preprocessor::DirectiveHandler {
    std::vector<std::string> deps;
    std::vector<Include> *patches; // used to delete lines containing directives
    
    void directiveEncountered(const Preprocessor::DirectiveContext& ctx);
};

bool preprocess(const ShaderManager& sm, Preprocessor& proc, const std::string& file, std::vector<Include> *includeBuffer, ShaderContents& shadersrc);

} // namespace glt

#endif
