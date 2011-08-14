#include "sys/fs/fs.hpp"
#include "err/err.hpp"

#include <Shlwapi.h>

namespace sys {

namespace fs {

const ModificationTime MIN_MODIFICATION_TIME(0, 0);

bool operator ==(const ModificationTime& a, const ModificationTime& b) {
    return a.mtime.dwHighDateTime == b.mtime.dwHighDateTime &&
        a.mtime.dwLowDateTime == b.mtime.dwLowDateTime;
}

bool operator <(const ModificationTime& a, const ModificationTime& b) {
    return a.mtime.dwHighDateTime < b.mtime.dwHighDateTime ||
            (a.mtime.dwHighDateTime == b.mtime.dwHighDateTime &&
            a.mtime.dwLowDateTime < b.mtime.dwLowDateTime);
}

bool cwd(const std::string& dir) {
    return SetCurrentDirectory(dir.c_str()) == TRUE;
}

std::string cwd() {
    char dir[4096];

    if (GetCurrentDirectoryA(sizeof dir, dir) == 0) { // FIXME: could error on small buffer
        WARN("GetCurrentDirectory() failed");
        return "";
    }

    return absolutePath(dir);
}

std::string dirname(const std::string& path) {
    return def::dirname(path);
}

std::string basename(const std::string& path) {
    return def::basename(path);
}

std::string extension(const std::string& path) {
    return def::extension(path);
}

std::string dropExtension(const std::string& path) {
    return def::dropExtension(path);
}

std::string dropTrailingSeparators(const std::string& path) {
    return def::dropTrailingSeparators(path);
}

bool isAbsolute(const std::string& path) {
    return PathIsRelativeA(path.c_str()) != TRUE;
}

bool modificationTime(const std::string& path, sys::fs::ModificationTime *mtime) {
    return def::modificationTime(path, mtime);
}

bool stat(const std::string& path, sys::fs::Stat *stat) {
    ASSERT(stat);
    
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attrs) == 0)
        return false;

    stat->mtime = attrs.ftLastWriteTime;
    stat->type = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? Directory : File;
    stat->absolute = absolutePath(path);

    if (stat->absolute.empty())
        return false;

    return true;
}

std::string absolutePath(const std::string& path) {
    char abs[4096];

    DWORD ret = GetFullPathNameA(path.c_str(), sizeof abs, abs, NULL);
    if (ret == 0)
        return "";

    if (ret > sizeof abs) {
        // FIXME: error on small buffer
        WARN("buffer too small");
        return "";
    }

    return std::string(abs);
}

std::string lookup(const std::vector<std::string>& dirs, const std::string& path) {
    return def::lookup(dirs, path);
}

bool exists(const std::string& path, ObjectType *type) {
    return def::exists(path, type);
}

}

}
