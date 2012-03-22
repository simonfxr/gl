#ifndef SYS_FS_WINDOWS_HPP
#define SYS_FS_WINDOWS_HPP

namespace sys {

namespace fs {

const char SEPARATOR = '\\';

// avoid windows.h
typedef struct _FILETIME {
  int32 dwLowDateTime;
  int32 dwHighDateTime;
} FILETIME;

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
