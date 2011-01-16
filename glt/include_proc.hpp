#ifndef INCLUDE_PROC_HPP
#define INCLUDE_PROC_HPP

#include "defs.h"
#include <string>
#include <vector>

#include <GL/gl.h>

namespace glt {

struct FileContents {
    GLsizei nsegments;
    const GLchar **segments;
    GLint *lengths;

    uint32 nchunks;
    const char **chunks;
};

void deleteFileContents(FileContents& contents);

bool readAndProcFile(const std::string& filePath, const std::vector<std::string>& includeDirs, FileContents& contents);

} // namespace glt


#endif
