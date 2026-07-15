#include "PreCompiled.h"

// Compute CRC32 of a file handle, preserving the original file position
uInt CalcFileCrc(const HANDLE hFile)
{
	uInt ResCrc = 0;
	const DWORD Offset = SetFilePointer(hFile, 0, nullptr, FILE_CURRENT);

	SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);

	uChar Buffer[0xFFFF];
	constexpr DWORD BufferSize = 0xFFFF;

	DWORD Readed = 0;
	while (ReadFile(hFile, Buffer, BufferSize, &Readed, nullptr) && (Readed != 0))
		ResCrc = Crc32(Buffer, Readed, ResCrc);

	SetFilePointer(hFile, Offset, nullptr, FILE_BEGIN);
	return ResCrc;
}


// Cached file info: version, last write time, and CRC for fast lookup
#pragma pack(push, 1)
struct FixFileInfo
{
	uInt Version;
	FILETIME LastWriteTime;
	uInt CRC;
};
#pragma pack(pop)

#define FIX_FILE_INFO_VERSION 1

// Get file CRC from NTFS alternate data stream cache, or compute and cache it
uInt GetFileCrc(const TString& file)
{
	const HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// Check NTFS alternate data stream for cached CRC info
		HANDLE hFixCrcInfo = CreateFile(file + _T(":FixCrcInfo"),
		                                GENERIC_READ,
		                                FILE_SHARE_READ,
		                                nullptr,
		                                OPEN_EXISTING,
		                                0,
		                                nullptr);
		if (hFixCrcInfo != INVALID_HANDLE_VALUE)
		{
			DWORD Readed = 0;

			FixFileInfo FileInfo;
			ReadFile(hFixCrcInfo, &FileInfo, sizeof(uInt), &Readed, nullptr);
			if (FileInfo.Version == FIX_FILE_INFO_VERSION)
			{
				if (ReadFile(hFixCrcInfo, ((uInt*)&FileInfo) + 1, sizeof(FixFileInfo) - sizeof(uInt), &Readed, nullptr))
				{
					FILETIME LastWriteTime = {};
					if (GetFileTime(hFile, nullptr, nullptr, &LastWriteTime))
					{
						// Return cached CRC if file hasn't changed
						if (!memcmp(&FileInfo.LastWriteTime, &LastWriteTime, sizeof(FILETIME)))
						{
							CloseHandle(hFixCrcInfo);
							CloseHandle(hFile);
							return FileInfo.CRC;
						}
					}
				}
			}
			CloseHandle(hFixCrcInfo);
		}

		// Compute CRC, cache it in NTFS alternate data stream, and return
		FixFileInfo NewFileInfo;
		memset(&NewFileInfo, 0, sizeof(FixFileInfo));
		NewFileInfo.Version = 1;
		NewFileInfo.CRC = CalcFileCrc(hFile);
		if (GetFileTime(hFile, nullptr, nullptr, &NewFileInfo.LastWriteTime))
		{
			hFixCrcInfo = CreateFile(file + _T(":FixCrcInfo"), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
			if (hFixCrcInfo != INVALID_HANDLE_VALUE)
			{
				DWORD Written = 0;
				WriteFile(hFixCrcInfo, &NewFileInfo, sizeof(FixFileInfo), &Written, nullptr);
				SetFileTime(hFixCrcInfo, nullptr, nullptr, &NewFileInfo.LastWriteTime);
				CloseHandle(hFixCrcInfo);
			}
		}

		CloseHandle(hFile);
		return NewFileInfo.CRC;
	}
	return 0;
}

Vdfs* FS;
TaggedArray<AString, HANDLE> WriteFiles;

// Hooked CreateFileA: track write handles to _WORK\DATA for VDFS index updates
HANDLE WINAPI MyCreateFileA(const LPCSTR lpFileName,
                            const DWORD dwDesiredAccess,
                            const DWORD dwShareMode,
                            const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            const DWORD dwCreationDisposition,
                            const DWORD dwFlagsAndAttributes,
                            const HANDLE hTemplateFile)
{
	const HANDLE hFile = CreateFileA(lpFileName,
	                                 dwDesiredAccess,
	                                 dwShareMode,
	                                 lpSecurityAttributes,
	                                 dwCreationDisposition,
	                                 dwFlagsAndAttributes,
	                                 hTemplateFile);
	if ((hFile != INVALID_HANDLE_VALUE) && ((dwDesiredAccess & GENERIC_ALL) || (dwDesiredAccess & GENERIC_WRITE)))
	{
		AString FileName(lpFileName);
		FileName.ToUpperCase();
		if (FileName.GetData("_WORK\\DATA"))
		{
			TString ExePath;
			if (PlatformGetExePath(ExePath))
			{
				ExePath.ToUpperCase();
				ExePath.TruncateAfterLast(_T("\\SYSTEM"));
				// Normalize path relative to game root for index consistency
				FileName.TruncateBeforeFirst(AString(ExePath));
				WriteFiles.Add(hFile, FileName);
				if (FS)
					FS->UpdateStdFileIndex(FileName, GetFileSize(hFile, nullptr));
			}
		}
	}
	return hFile;
}

// Hooked WriteFile: update VDFS index when a tracked file is modified
BOOL WINAPI MyWriteFile(const HANDLE hFile,
                        const LPCVOID lpBuffer,
                        const DWORD nNumberOfBytesToWrite,
                        const LPDWORD lpNumberOfBytesWritten,
                        const LPOVERLAPPED lpOverlapped)
{
	const BOOL Res = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	if (Res && WriteFiles.TagExist(hFile))
	{
		if (FS)
			FS->UpdateStdFileIndex(WriteFiles.GetElement(hFile), GetFileSize(hFile, nullptr));
	}
	return Res;
}

// Hooked CloseHandle: remove tracked file from the write tracking array
BOOL WINAPI MyCloseHandle(const HANDLE hObject)
{
	WriteFiles.EraseByTag(hObject);
	return CloseHandle(hObject);
}

// Install file system hooks to enable VDFS index updates for _WORK\DATA files
bool InstallFsHook(Vdfs& vdfs)
{
	constexpr auto font = _T(R"(_WORK\DATA\TEXTURES\_COMPILED\FONT_15_WHITE.FNT)");
	FS = &vdfs;

	// Remove known-bad font file that causes rendering issues
	bool ChangeWorkDir = false;
	TString WorkPath;
	if (PlatformGetWorkPath(WorkPath) && WorkPath.TruncateBeforeLast(_T("\\")) && WorkPath.Compare(_T("System"), true))
		ChangeWorkDir = (SetCurrentDirectory(_T("..\\")) == TRUE);

	if (PathFileExists(font))
	{
		if (GetFileCrc(font) == 0xC61A14B6)
			DeleteFile(font);
	}

	if (ChangeWorkDir)
		SetCurrentDirectory(_T("System\\"));

	// Hook KERNEL32.dll imports for file I/O tracking
	const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
	const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "KERNEL32.dll");
	if (importDesc)
	{
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "CreateFileA", (FARPROC)MyCreateFileA);
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "WriteFile", (FARPROC)MyWriteFile);
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "CloseHandle", (FARPROC)MyCloseHandle);
	}

	return true;
}
