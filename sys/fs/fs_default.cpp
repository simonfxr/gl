#include "sys/fs/fs.hpp"

#include <cstdio>

namespace sys {

namespace fs {

namespace def {

std::string lookup(const std::vector<std::string>& dirs, const std::string& name) {
    std::string suffix = "/" + name;

    for (uint32 i = 0; i < dirs.size(); ++i) {
        std::string filename = dirs[i] + suffix;
        FILE *file = fopen(filename.c_str(), "r");
        if (file != 0) {
            fclose(file);
            return filename;
        }
    }

    return "";
}

} // namespace def

} // namespace fs

} // namespace sys
