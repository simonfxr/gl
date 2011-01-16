#include <cstdio>
#include <cstring>
#include <set>
#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <dirent.h>

#include "defs.h"
#include "glt/utils.hpp"
#include "glt/include_proc.hpp"

namespace glt {

void deleteFileContents(FileContents& contents) {
    
    delete[] contents.lengths;
    delete[] contents.segments;

    contents.nsegments = 0;
    contents.lengths = 0;
    contents.segments = 0;

    for (uint32 i = 0; i < contents.nchunks; ++i)
        delete[] contents.chunks[i];

    delete[] contents.chunks;
        
    contents.nchunks = 0;
    contents.chunks = 0;
}

namespace {

bool readContents(const std::string& path, char * &file_contents, int32 &file_size) {
    FILE *in = fopen(path.c_str(), "rb");
    if (in == 0)
        return false;
    if (fseek(in, 0, SEEK_END) == -1)
        return false;
    int64 size = ftell(in);
    if (size < 0)
        return false;
    if (fseek(in, 0, SEEK_SET) == -1)
        return false;
    char *contents = new char[size + 1];
    if (fread(contents, size, 1, in) != 1) {
        delete[] contents;
        return false;
    }
    contents[size] = '\0';
    file_contents = contents;
    file_size = size;
    return true;
}

bool fileExists(const std::string& file) {
    FILE *fp = fopen(file.c_str(), "r");
    if (fp == NULL) {
        return false;
    } else {
        fclose(fp);
        return true;
    }
}

std::string lookupInclude(const std::vector<std::string>& paths, const std::vector<std::string>& parts) {

    std::stringstream baseName;
    for (uint32 i = 0; i < parts.size(); ++i)
        baseName << "/" << parts[i];

    std::string baseFilePath = baseName.str();

    for (uint32 i = 0; i < paths.size(); ++i) {
        std::string path = paths[i] + baseFilePath;
        if (fileExists(path))
            return path;
    }

    return "";
}

bool parseIncludePath(const char *str, const char *end, std::vector<std::string>& pathSegments) {

    const char *begin = str;

    while (str < end && isspace(*str))
        ++str;

    if (str == begin || str >= end) return false;

    char term  = *str == '"' ? '"' :
                 *str == '<' ? '>' :
                                0;

    if (term == 0) return false;

    ++str;
    const char *seg = str;

    while (str < end && *str != term) {
        while (str < end && *str != term && *str != '/')
            ++str;

        const char *segEnd = str;
        
        while (str < end && *str == '/')
            ++str;

        if (str >= end) return false;

        pathSegments.push_back(std::string(seg, segEnd - seg));
        seg = str;
    }

    while (str < end && isspace(*str))
        ++str;

    return str >= end && !pathSegments.empty();
}

struct State {
    const std::vector<std::string>& dirs;
    std::set<std::string> included;
    std::vector<const GLchar *> strings;
    std::vector<GLsizei> lengths;
    std::vector<const char *> chunks;

    State(const std::vector<std::string>& _dirs) :
        dirs(_dirs)
        {}
};

static const int32 MAX_INCLUDE_DEPTH = 16;

bool process(State& s, const char *str, const int32 len, int32 depth) {

    if (depth >= MAX_INCLUDE_DEPTH) {
        ERROR("MAX_INCLUDE_DEPTH exceeded");
        return false;
    }

    const char *begin = str;
    const char *last = str + len;

    static const char * const KEY = "#include";

    const char *match;
    const char *segBegin = str;

    while (str < last && (match = strstr(str, KEY)) != 0) {

        if (match != begin && match[-1] != '\n') {
            str = match + strlen(KEY);
            continue;
        }
        
        const char *eol = strchr(match, '\n');
        eol = eol == 0 ? last : eol + 1;

        std::vector<std::string> pathSegs;
        if (parseIncludePath(match + strlen(KEY), eol, pathSegs)){
            ERROR("couldnt parse include directive");
        } else {

            std::string includedFile = lookupInclude(s.dirs, pathSegs);
            
            if (includedFile.empty()) {
                ERROR("couldnt find included file");
                return false;
            }

            char *includedContents = 0;
            int32 size = 0;

            if (s.included.count(includedFile) == 0 &&
                !readContents(includedFile, includedContents, size)) {
                
                ERROR("couldnt read file");
                return false;
            }

            GLsizei seglen = match - segBegin;
            if (seglen > 0) {
                s.strings.push_back(segBegin);
                s.lengths.push_back(seglen);
            }
            
            segBegin = eol;

            if (includedContents != 0) {
                s.included.insert(includedFile);
                s.chunks.push_back(includedContents);
                if (!process(s, includedContents, size, depth + 1))
                    return false;
            }
        }

        str = eol;
    }

    GLsizei seglen = last - segBegin;
    if (seglen > 0) {
        s.strings.push_back(segBegin);
        s.lengths.push_back(seglen);
    }

    return true;
}

} // namespace anon

bool readAndProcFile(const std::string& filePath, const std::vector<std::string>& includeDirs, FileContents& contents) {

    char *str;
    int32 len;

    if (!readContents(filePath, str, len))
        return false;

    State s(includeDirs);
    s.included.insert(filePath);
    s.chunks.push_back(str);
    
    if (!process(s, str, len, 1)) {

        for (uint32 i = 0; i < s.chunks.size(); ++i)
            delete[] s.chunks[i];

        return false;
    }

    contents.nsegments = s.strings.size();
    contents.segments = new const GLchar *[contents.nsegments];
    std::copy(s.strings.begin(), s.strings.end(), contents.segments);
    contents.lengths = new GLsizei[contents.nsegments];
    std::copy(s.lengths.begin(), s.lengths.end(), contents.lengths);
    contents.nchunks = s.chunks.size();
    contents.chunks = new const char *[contents.nchunks];
    std::copy(s.chunks.begin(), s.chunks.end(), contents.chunks);

    // std::cerr << "file contents of " << filePath << ":" << std::endl;

    // int line = 0;
    // std::cerr << line << "\t:";
    // for (GLsizei i = 0; i < contents.nsegments; ++i) {
    //     for (GLint k = 0; k < contents.lengths[i]; ++k) {
    //         putchar(contents.segments[i][k]);
    //         if (contents.segments[i][k] == '\n') {
    //             ++line;
    //             std::cerr << line << "\t:";
    //         }
    //     }
        
    // }

    // std::cerr << "END" << std::endl;

    return true;
}

} // namespace glt
