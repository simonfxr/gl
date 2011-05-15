#include "fs/fs_unix.hpp"
#include "fs/fs.hpp"
#include "error/error.hpp"

#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

namespace fs {

bool cwd(const std::string& dir) {
    if (chdir(dir.c_str()) < 0) {
        ERR(strerror(errno));
        return false;
    }
    return true;
}

bool cwdBasenameOf(const std::string& file) {
    size_t pos = file.rfind('/');
    if (pos == std::string::npos) // relative filename already in correct dir
        return true;
    return cwd(file.substr(0, pos));
}

MTime getMTime(const std::string& file) {
    struct stat st;
    MTime mt;
    if (stat(file.c_str(), &st) != 0) {
        ERR(strerror(errno));
        mt.timestamp = ~0;
    } else {
        mt.timestamp = st.st_mtime;
    }
    return mt;
}

bool operator ==(const MTime& a, const MTime& b) {
    return a.timestamp == b.timestamp;
}

bool operator <(const MTime& a, const MTime& b) {
    return a.timestamp < b.timestamp;
}

} // namespace fs
