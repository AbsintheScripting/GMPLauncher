#include "PreCompiled.h"

// Open a file from the VDFS archive or physical filesystem
long _cdecl vdf_fopen(const char* filename, const long flags)
{
	IfsBase* res = VdfsBase.OpenFile(filename, flags);
	if (res)
		return (int)res;
	VdfsBase.SetLastError(AString(filename) + " file not found");
	return -1;
}

// Close a previously opened file handle
long _cdecl vdf_fclose(const long fp)
{
	VdfsBase.CloseFile((IfsBase*)fp);
	return 0;
}

// Read data from an open file handle into the provided buffer
long _cdecl vdf_fread(const long fp, char* buffer, const long size)
{
	if (fp)
		return ((IfsBase*)fp)->GetData(buffer, size);
	return 0;
}

// Seek to an absolute offset within an open file
long _cdecl vdf_fseek(const long fp, const long offset)
{
	return (((IfsBase*)fp)->SetOffset(offset) ? offset : -1);
}

// Get the current file position offset
long _cdecl vdf_ftell(const long fp)
{
	return ((IfsBase*)fp)->GetOffset();
}

// Check if a file exists in the archive or filesystem
long _cdecl vdf_fexists(const char* filename, const long flags)
{
	return VdfsBase.FileExist(filename, flags);
}

// Search for a file by name and return its full virtual path
long _cdecl vdf_searchfile(const char* filename, char* fullname)
{
	*fullname = '\0';
	return VdfsBase.SearchFile(filename, fullname) ? 1 : 0;
};

// Retrieve the last error message string
long _cdecl vdf_getlasterror(char* text)
{
	VdfsBase.GetLastError(text);
	return 0;
}

// Initialize VDFS by mounting archive files; returns 0 on success
long _cdecl vdf_initall(long numdisks, const char* cdid, long* cddrives, long* disksfound)
{
	if (!IsVdfs())
	{
		//if(!IsSpacer())
		//	InstallSteamOverlayFix();

		if (VdfsBase.Init())
			return 0; // Ok
		return -1; // No files
	}
	return 0;
}

// Terminate VDFS and unmount all archives
long _cdecl vdf_exitall()
{
	VdfsBase.Clear();
	return 0;
}

// Get the size of a file opened via vdf_fopen
long _cdecl vdf_ffilesize(const long fp)
{
	return ((IfsBase*)fp)->GetFileSize();
}

// Not implemented - displays error message box
long _cdecl vdf_getdir(char* dirname)
{
	MessageBox(nullptr, _T("Unsupported function vdf_getdir"), _T("VDFS"), MB_ICONERROR);
	return -1;
}

// Not implemented - displays error message box
long _cdecl vdf_findopen(const char* path, long flags)
{
	MessageBox(nullptr, _T("Unsupported function vdf_findopen"), _T("VDFS"), MB_ICONERROR);
	return NULL;
}

// Not implemented - displays error message box
long _cdecl vdf_findnext(long find, char* filename)
{
	MessageBox(nullptr, _T("Unsupported function vdf_findnext"), _T("VDFS"), MB_ICONERROR);
	return -1;
}

// Not implemented - displays error message box
long _cdecl vdf_findclose(long find)
{
	MessageBox(nullptr, _T("Unsupported function vdf_findclose"), _T("VDFS"), MB_ICONERROR);
	return -1;
}

// Internal: initialize virtual file system without physical flow
long _cdecl vdf_initall_internal()
{
	if (VdfsBase.InitVirtual())
		return 0; // Ok
	return -1; // No files
}

// Not used - stub placeholder
int _cdecl vdf_fseekrel()
{
	return 0;
};

int _cdecl vdf_fdirexist()
{
	return 0;
};

int _cdecl vdf_changedir()
{
	return 0;
};

int _cdecl GetFileInfo()
{
	return 0;
};

int _cdecl vdf_setOption()
{
	return 0;
};

int _cdecl vdf_GetOption()
{
	return 0;
};

// DLL entry point: install fixes on process attach, remove them on detach
BOOL APIENTRY DllMain(HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			if (!IsVdfs())
			{
				AttachFixesInstaller();
			}
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		RemoveFixes();
		break;
	}
	return TRUE;
}
