#include "sys/fs/fs_unix.hpp"
#include "sys/fs.hpp"
#include "err/err.hpp"

#include <string>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sys {

namespace fs {

STATIC_ASSERT(sizeof(time_t) == sizeof(ModificationTime::time_t));
    
const ModificationTime MIN_MODIFICATION_TIME(0);

bool cwd(const std::string& dir) {

    if (chdir(dir.c_str()) < 0) {
        ERR(strerror(errno));
        return false;
    }
    return true;
}

std::string cwd() {
    char path[1024];
    char *wd = path;
    
    if (!getcwd(path, sizeof path)) {

        size_t size = sizeof path;
        wd = 0;
        bool success = false;

        while (errno == ERANGE) {
            size *= 2;
            delete[] wd;
            wd = new char[size];

            if (getcwd(wd, size)) {
                success = true;
                break;
            }
        }

        if (!success) {
            delete[] wd;
            return "";
        }
    }

    std::string dir(wd);
    if (wd != path)
        delete[] wd;

    return dir;
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

std::string dropExtension(const std::string& path) {
    return def::dropExtension(path);
}

std::string extension(const std::string& path) {
    size_t pos = path.rfind('/');
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return "";
    return path.substr(pos + 1);
}

bool isAbsolute(const std::string& path) {
    return !path.empty() && path[0] == '/';
}

std::string absolutePath(const std::string& path) {
    if (isAbsolute(path))
        return path;

    std::string dir = cwd();
    if (dir.empty())
        return "";

    return dir + "/" + path;
}

bool stat(const std::string& path, sys::fs::Stat *stat) {
    ASSERT(stat);

    stat->absolute = absolutePath(path);
    if (stat->absolute.empty())
        return false;

    return modificationTime(path, &stat->mtime);
}

bool modificationTime(const std::string& path, sys::fs::ModificationTime *mtime) {
    ASSERT(mtime);
    
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        ERR(strerror(errno));
        return false;
    }

    mtime->timestamp = st.st_mtime;
    return true;
}

std::string lookup(const std::vector<std::string>& dirs, const std::string& name) {
    return def::lookup(dirs, name);
}

bool exists(const std::string& path, ObjectType *type) {
    ASSERT(type);
    struct stat info;
    if (stat(path.c_str(), &info) == -1)
        return false;
    int objtype = info.st_mode & S_IFMT;
    *type = objtype == S_IFDIR ? Directory : File;
    return true;
}

bool operator ==(const ModificationTime& a, const ModificationTime& b) {
    return a.timestamp == b.timestamp;
}

bool operator <(const ModificationTime& a, const ModificationTime& b) {
    return a.timestamp < b.timestamp;
}

} // namespace fs

} // namespace sys
