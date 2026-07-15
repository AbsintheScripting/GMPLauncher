#ifndef _COMMON_UTILITY_EX_
#define _COMMON_UTILITY_EX_

namespace COMMON
{
	// Platform independence functions
	extern bool PlatformGetComputerName(TString& name);
	extern uInt PlatformGetCurrentProcessId();
	extern uInt PlatformGetCurrentThreadId();

	extern bool PlatformGetWorkPath(TString& path);
	extern bool PlatformGetTempPath(TString& path);
	extern bool PlatformGetTempFileName(TString& name);
	extern bool PlatformGetExePath(TString& name);

	extern bool PlatformReadTextFile(const TString& file, TStringArray& lines);
}

#endif
