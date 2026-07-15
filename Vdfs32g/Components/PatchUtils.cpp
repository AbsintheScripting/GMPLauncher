#include "PreCompiled.h"

#include <io.h>
#include <fcntl.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

// Allocate a console and redirect stdin/stdout/stderr for debug output
void RedirectIOToConsole()
{
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	// allocate a console for this app
	AllocConsole();

	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 50;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	setvbuf(stdout, nullptr, _IONBF, 0);
	setvbuf(stderr, nullptr, _IONBF, 0);
}

// Get the full module file name, handling buffer reallocation
bool GetModuleFileNameString(const HMODULE hModule, TString& name)
{
	SetLastError(ERROR_INSUFFICIENT_BUFFER);
	for (uInt i = 1; GetLastError() == ERROR_INSUFFICIENT_BUFFER; i++)
	{
		name.Resize(i * MAX_PATH);
		SetLastError(0);
		if (!GetModuleFileName(hModule, (TCHAR*)name.GetData(), i * MAX_PATH))
			return false;
	}
	return (name.Length() != 0);
}

HWND hGothicWindow = nullptr;

// Window enumeration callback: finds the Gothic game window by class name
BOOL CALLBACK EnumWindowsProc(const HWND hWnd, LPARAM lParam)
{
	DWORD ProcID = 0;
	GetWindowThreadProcessId(hWnd, &ProcID);
	if (ProcID == GetCurrentProcessId())
	{
		TCHAR Text[256];
		if (GetClassName(hWnd, Text, 256) && !_tcscmp(Text, _T("DDWndClass")))
		{
			hGothicWindow = hWnd;
			return FALSE;
		}
	}
	return TRUE;
}

// Get the Gothic game window resolution from Gothic.ini VIDEO settings
bool GetGothicWindowSize(POINT& size)
{
	size.x = size.y = 0;

	/*if(!hGothicWindow)
		EnumWindows(EnumWindowsProc, NULL);

	if(hGothicWindow)
	{
		RECT ClientRect;
		if(GetClientRect(hGothicWindow, &ClientRect))
		{
			size.x = ClientRect.right - ClientRect.left;
			size.y = ClientRect.bottom - ClientRect.top;
			printf("%d %d\n", size.x, size.y);
			return true;
		}
	}*/

	char Buffer[256];
	GothicReadIniString("VIDEO", "zVidResFullscreenX", "0", Buffer, 256, "Gothic.ini");
	size.x = atoi(Buffer);
	GothicReadIniString("VIDEO", "zVidResFullscreenY", "0", Buffer, 256, "Gothic.ini");
	size.y = atoi(Buffer);

	return (size.x && size.y);
}

// Check if the current executable is the vdfs32.dll itself (loaded as DLL)
bool IsVdfs()
{
	TString ExeName;
	if (GetModuleFileNameString(GetModuleHandle(nullptr), ExeName))
	{
		ExeName.TruncateBeforeLast(_T("\\"));
		return ExeName.Compare(_T("vdfs32"), true, 6);
	}
	return false;
}

// Check if the current executable is spacer.exe (Gothic 2 Nrt)
bool IsSpacer()
{
	TString ExeName;
	if (GetModuleFileNameString(GetModuleHandle(nullptr), ExeName))
	{
		ExeName.TruncateBeforeLast(_T("\\"));
		return ExeName.Compare(_T("spacer"), true, 6);
	}
	return false;
}

static bool ExeCrcReaded = false;
static uLong ExeCrc = 0;

// Compute and cache CRC32 of the entire executable file
uLong GetExeCrc32()
{
	if (!ExeCrcReaded)
	{
		ExeCrc = 0;

		TString ExeName;
		if (GetModuleFileNameString(nullptr, ExeName))
		{
			FILE* fp = _tfopen(ExeName, _T("rb"));
			if (fp)
			{
				uChar Buffer[1024];
				size_t size = 0;
				while (size = fread(Buffer, 1, 1024, fp))
					ExeCrc = Crc32(Buffer, size, ExeCrc);
				ExeCrcReaded = true;
				fclose(fp);
			}
		}
	}

	return ExeCrc;
}

// Get the address and size of a PE section by name
const uChar* GetSectionAddress(const uChar* codeBase, const char* name, size_t& size)
{
	size_t namelen = strlen(name);
	if (namelen > 8)
		return nullptr;

	auto dos_header = (PIMAGE_DOS_HEADER)codeBase;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	auto headers = (PIMAGE_NT_HEADERS)&((uChar*)dos_header)[dos_header->e_lfanew];
	auto section = IMAGE_FIRST_SECTION(headers);
	for (int i = 0; i < headers->FileHeader.NumberOfSections; i++, section++)
	{
		if (!memcmp(section->Name, name, namelen))
		{
			size = section->SizeOfRawData;
			return codeBase + section->PointerToRawData;
		}
	}
	return nullptr;
}

// Compute CRC32 of a named PE section from the executable file
uLong GetSectionCrc32(const char* name)
{
	uLong Result = 0;

	size_t namelen = strlen(name);
	if (namelen > 8)
		return Result;

	TString ExeName;
	if (GetModuleFileNameString(nullptr, ExeName))
	{
		FILE* fp = _tfopen(ExeName, _T("rb"));
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			long size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			if (!size)
			{
				fclose(fp);
				return Result;
			}

			auto Buffer = new uChar[size];
			size = fread(Buffer, 1, size, fp);
			fclose(fp);

			if (size)
			{
				const uChar* codeBase = Buffer;

				auto dos_header = (PIMAGE_DOS_HEADER)codeBase;
				if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
					return 0;

				auto headers = (PIMAGE_NT_HEADERS)&((uChar*)dos_header)[dos_header->e_lfanew];
				auto section = IMAGE_FIRST_SECTION(headers);
				for (int i = 0; i < headers->FileHeader.NumberOfSections; i++, section++)
				{
					if (!memcmp(section->Name, name, namelen))
					{
						if (section->SizeOfRawData != 0)
							Result = Crc32(codeBase + section->PointerToRawData, section->SizeOfRawData);
						break;
					}
				}
			}
			delete[] Buffer;
		}
	}
	return Result;
}

// Check if any detected GPU adapter belongs to VGA vendor ID
bool HasVgaVendor(const DWORD ven)
{
	bool Result = false;
	IDirect3D9* D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (D3D)
	{
		for (UINT i = 0; i < D3D->GetAdapterCount(); i++)
		{
			D3DADAPTER_IDENTIFIER9 Adapter;
			if (SUCCEEDED(D3D->GetAdapterIdentifier(i, 0, &Adapter)))
			{
				if (Adapter.VendorId == ven)
				{
					Result = true;
					break;
				}
			}
		}
		D3D->Release();
	}
	return Result;
}

// Patch a memory region: verify optional original bytes, then overwrite with new bytes
bool Patch(uChar* data, const size_t size, uChar* org, uChar* patch)
{
	bool Result = false;

	DWORD OldProtect = 0;
	if (VirtualProtect(data, size, PAGE_READWRITE, &OldProtect))
	{
		if (!org || !memcmp(data, org, size))
		{
			memcpy(data, patch, size);
			Result = true;
		}
		else
		{
			RedirectIOToConsole();
			for (size_t i = 0; i < size; i++)
				printf("0x%X(%c), ", data[i], data[i]);
		}
		VirtualProtect(data, 5, OldProtect, &OldProtect);
	}

	return Result;
}

// Get the total image size from the PE optional header
DWORD GetImportSize(const uChar* codeBase)
{
	auto dos_header = (PIMAGE_DOS_HEADER)codeBase;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	auto old_header = (PIMAGE_NT_HEADERS)&codeBase[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	return old_header->OptionalHeader.SizeOfImage;
}

// Get the import descriptor for a specific DLL by name
PIMAGE_IMPORT_DESCRIPTOR GetImportDescriptor(const uChar* codeBase, const char* name)
{
	auto dos_header = (PIMAGE_DOS_HEADER)codeBase;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	auto old_header = (PIMAGE_NT_HEADERS)&codeBase[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;

	PIMAGE_DATA_DIRECTORY directory = &old_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (directory->Size > 0)
	{
		auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(codeBase + directory->VirtualAddress);
		for (; !IsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++)
		{
			auto Name = (LPCSTR)(codeBase + importDesc->Name);
			if (!_stricmp(Name, name))
				return importDesc;
		}
	}

	return nullptr;
}

// Get a pointer to the import function address entry in the IAT
FARPROC* GetImportFunctionAddress(const uChar* codeBase,
                                  const PIMAGE_IMPORT_DESCRIPTOR importDesc,
                                  const bool ordinal,
                                  const char* name)
{
	DWORD* thunkRef = nullptr;
	FARPROC* funcRef = nullptr;

	if (importDesc->OriginalFirstThunk)
	{
		thunkRef = (DWORD*)(codeBase + importDesc->OriginalFirstThunk);
		funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
	}
	else
	{
		// no hint table
		thunkRef = (DWORD*)(codeBase + importDesc->FirstThunk);
		funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
	}

	for (; *thunkRef; thunkRef++, funcRef++)
	{
		if (IMAGE_SNAP_BY_ORDINAL(*thunkRef))
		{
			if (ordinal)
			{
				auto FuncOrdinal = (LPCSTR)IMAGE_ORDINAL(*thunkRef);
				if (FuncOrdinal == name)
					return funcRef;
			}
		}
		else
		{
			if (!ordinal)
			{
				auto thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
				if (!_stricmp((LPCSTR)&thunkData->Name, name))
					return funcRef;
			}
		}
	}

	return nullptr;
}

// Helper template: patch a specific import function address in the IAT
template <typename T>
bool PatchImportFunctionAddress(uChar* codeBase,
                                const PIMAGE_IMPORT_DESCRIPTOR importDesc,
                                const bool ordinal,
                                const char* name,
                                T newFunc)
{
	FARPROC* funcAddr = GetImportFunctionAddress(codeBase, importDesc, ordinal, name);
	if (funcAddr)
	{
		DWORD OldProtect;
		if (VirtualProtect(funcAddr, sizeof(FARPROC), PAGE_READWRITE, &OldProtect))
		{
			*funcAddr = static_cast<FARPROC>(newFunc);
			VirtualProtect(funcAddr, sizeof(FARPROC), OldProtect, &OldProtect);
			return true;
		}
	}
	return false;
}
