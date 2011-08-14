#ifndef SYS_FS_WINDOWS_HPP
#define SYS_FS_WINDOWS_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <Windows.h>

namespace sys {

namespace fs {

const char SEPARATOR = '\\';

struct ModificationTime {
	FILETIME mtime;
	ModificationTime() {}
	ModificationTime(FILETIME _mtime) :
	    mtime(_mtime) {}
	ModificationTime(DWORD lo, DWORD hi) {
		mtime.dwLowDateTime = lo;
		mtime.dwHighDateTime = hi;
	}
};

} // namespace fs

} // namespace sys

#endif
