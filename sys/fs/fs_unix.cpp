#include "sys/fs/fs_unix.hpp"
#include "sys/fs/fs.hpp"
#include "error/error.hpp"

#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

namespace sys {

namespace fs {

bool cwd(const std::string& dir) {
    if (chdir(dir.c_str()) < 0) {
        ERR(strerror(errno));
        return false;
    }
    return true;
}

static size_t dropTrailingSlashes(const std::string& path) {
    if (path.empty())
        return std::string::npos;
    size_t pos = path.length() - 1;

    while (pos > 0 && path[pos] == '/')
        --pos;
    
    return pos;   
}

std::string dirname(const std::string& path) {
    if (path.empty())
        return ".";
    
    size_t pos = path.rfind('/', dropTrailingSlashes(path));
    
    if (pos != std::string::npos) {
        while (pos > 1 && path[pos - 1] == '/')
            --pos;
    } else {
        pos = 0;
    }

    if (pos == 0)
        return "/";
    else
        return path.substr(0, pos);
}

std::string basename(const std::string& path) {

    if (path.empty())
        return "";
    
    size_t pos = path.rfind('/', dropTrailingSlashes(path));
    size_t end = std::string::npos;

    if (pos != std::string::npos) {
        pos += 1;
        end = path.find('/', pos + 1);
    } else {
        pos = 0;
    }

    if (end == std::string::npos)
        end = path.length();
        
    return path.substr(pos, end - pos);
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

} // namespace sys
