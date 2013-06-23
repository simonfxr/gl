#include <cstdio>

#include "sys/fs.hpp"
#include "err/err.hpp"

namespace sys {

namespace fs {

namespace def {

namespace {

size_t dropTrailingSeps(const std::string& path) {
    if (path.empty())
        return std::string::npos;
    size_t pos = path.length() - 1;

    while (pos > 0 && path[pos] == '/')
        --pos;
    
    return pos;   
}

}

std::string dirname(const std::string& path) {
    if (path.empty())
        return ".";
    
    size_t pos = path.rfind(SEPARATOR, dropTrailingSeps(path));
    
    if (pos != std::string::npos) {
        while (pos > 1 && path[pos - 1] == SEPARATOR)
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
    
    size_t pos = path.rfind(SEPARATOR, dropTrailingSeps(path));
    size_t end = std::string::npos;

    if (pos != std::string::npos) {
        pos += 1;
        end = path.find(SEPARATOR, pos + 1);
    } else {
        pos = 0;
    }

    if (end == std::string::npos)
        end = path.length();
        
    return path.substr(pos, end - pos);
}

std::string extension(const std::string& path) {
    size_t pos = path.rfind(SEPARATOR);
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return "";
    return path.substr(pos + 1);
}

std::string dropExtension(const std::string& path) {
    size_t pos = path.rfind(SEPARATOR);
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return path;
    return path.substr(0, pos);
}

std::string dropTrailingSeparators(const std::string& path) {
    size_t pos = dropTrailingSeps(path);
    if (pos == std::string::npos)
        return path;
    return path.substr(0, pos);
}

bool exists(const std::string& path, ObjectType *type) {
    ASSERT(type);
    Stat st;
    if (!stat(path, &st))
        return false;
    *type = st.type;
    return true;
}

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

std::string absolutePath(const std::string& path) {
    Stat st;
    if (!stat(path, &st))
        return "";
    return st.absolute;
}

bool modificationTime(const std::string& path, sys::fs::ModificationTime *mtime) {
    ASSERT(mtime);
    Stat st;
    if (!stat(path, &st))
        return false;
    *mtime = st.mtime;
    return true;
}

} // namespace def

} // namespace fs

} // namespace sys
