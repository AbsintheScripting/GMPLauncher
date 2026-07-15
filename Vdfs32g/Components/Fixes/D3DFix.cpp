#include "PreCompiled.h"
#include <VersionHelpers.h>

// Patch D3DIM700.dll: search for a signature and replace video mode check to allow high resolutions
bool PatchD3D(uChar* codeBase)
{
	constexpr uInt Signature = 0x88760064;
	const unsigned char SubSignature[] = { 0xB8, 0x00, 0x08, 0x00, 0x00, 0x39 };
	const unsigned char SubSignatureNew[] = { 0xB8, 0x00, 0x80, 0x00, 0x00, 0x39 };

	const DWORD codeSize = GetImportSize(codeBase);
	if (codeSize <= sizeof(Signature) + sizeof(SubSignature))
		return true;

	for (size_t i = 0; i < (codeSize - sizeof(Signature) - sizeof(SubSignature)); i++)
	{
		if (memcmp(&codeBase[i], &Signature, sizeof(Signature)))
			continue;

		// Search ±1200 bytes around the signature for the specific sub-signature
		const size_t StartToCheck = MAX(i - 1200, 0);
		const size_t ToCheck = MIN(2400, codeSize - i - sizeof(SubSignature));
		for (size_t n = StartToCheck; n <= StartToCheck + ToCheck - 1; n++)
		{
			if (memcmp(&codeBase[n], SubSignature, sizeof(SubSignature)))
				continue;

			DWORD OldProtect = 0;
			if (!VirtualProtect(&codeBase[n], sizeof(SubSignature), PAGE_READWRITE, &OldProtect))
				return false;

			memcpy(&codeBase[n], SubSignatureNew, sizeof(SubSignature));
			VirtualProtect(&codeBase[n], sizeof(SubSignature), OldProtect, &OldProtect);
			i = n + sizeof(SubSignature);
			break;
		}
	}
	return true;
}

static HMODULE hD3Dim = nullptr;
using SetAppCompatDataFunc = int(__stdcall*)(int a1, int a2);

// Hooked CreateWindowExA: force WS_POPUP style for non-windowed Gothic windows
HWND WINAPI MyCreateWindowExA(__in DWORD dwExStyle,
                              __in_opt const LPCSTR lpClassName,
                              __in_opt const LPCSTR lpWindowName,
                              __in DWORD dwStyle,
                              __in const int X,
                              __in const int Y,
                              __in const int nWidth,
                              __in const int nHeight,
                              __in_opt const HWND hWndParent,
                              __in_opt const HMENU hMenu,
                              __in_opt const HINSTANCE hInstance,
                              __in_opt const LPVOID lpParam)
{
	if (!strcmp(lpClassName, "DDWndClass") && !atoi(zStartupWindowed))
	{
		dwExStyle = 0;
		dwStyle = WS_POPUP;
	}
	return CreateWindowExA(dwExStyle,
	                       lpClassName,
	                       lpWindowName,
	                       dwStyle,
	                       X,
	                       Y,
	                       nWidth,
	                       nHeight,
	                       hWndParent,
	                       hMenu,
	                       hInstance,
	                       lpParam);
}

// Install Direct3D high-resolution and app compatibility fixes
bool InstallD3DFix()
{
	char FixHighRes[256];
	if (!GothicReadIniString("DEBUG", "FixHighRes", "1", FixHighRes, 256, "SystemPack.ini"))
		GothicWriteIniString("DEBUG", "FixHighRes", "1", "SystemPack.ini");

	char FixAppCompat[256];
	if (!GothicReadIniString("DEBUG", "FixAppCompat", "1", FixAppCompat, 256, "SystemPack.ini"))
		GothicWriteIniString("DEBUG", "FixAppCompat", "1", "SystemPack.ini");

	const HMODULE hDDraw = GetModuleHandle(_T("ddraw.dll"));
	if (hDDraw)
	{
		// On XP+: patch D3DIM700.dll to allow high resolutions
		if (IsWindowsXPOrGreater() && (atoi(FixHighRes) == 1))
		{
			DeleteFile(_T("D3DIM700.dll"));
			if (hD3Dim = LoadLibrary(_T("D3DIM700.dll")))
				PatchD3D((uChar*)hD3Dim);
		}

		// On Win8+: apply app compatibility fix via SetAppCompatData or window style patch
		if (IsWindows8OrGreater())
		{
			const int AppCompatFix = atoi(FixAppCompat);
			switch (AppCompatFix)
			{
			case 1:
				{
					// Disable max windowed mode via DirectX app compat data
					const auto SetAppCompatData = (SetAppCompatDataFunc)
						GetProcAddress(hDDraw, "SetAppCompatData"); // DXPrimaryEmulation -DisableMaxWindowedMode
					if (SetAppCompatData)
						SetAppCompatData(12, 0);
				}
				break;
			case 2:
				{
					// Patch CreateWindowExA to force WS_POPUP style for Gothic
					const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
					const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "USER32.dll");
					if (importDesc)
						PatchImportFunctionAddress<FARPROC>(codeBase,
						                                    importDesc,
						                                    false,
						                                    "CreateWindowExA",
						                                    (FARPROC)MyCreateWindowExA);
				}
				break;
			}
		}
	}
	return true;
}

// Remove the D3D fix by freeing the patched D3DIM700.dll
void RemoveD3DFix()
{
	if (hD3Dim)
	{
		FreeLibrary(hD3Dim);
		hD3Dim = nullptr;
	}
}
