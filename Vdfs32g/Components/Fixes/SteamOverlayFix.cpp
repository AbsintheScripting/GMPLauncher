#include "PreCompiled.h"

// Handle to the loaded ddraw.dll module
HMODULE hDDraw = nullptr;
// Backup of the original DirectDrawCreateEx function bytes for restoration
static uChar Bak[5] = {};

// Function pointer type for DirectDrawEnumerateExA from ddraw.dll
using DirectDrawEnumerateExAPtr = HRESULT(WINAPI*)(LPVOID lpCallback, LPVOID lpContext, DWORD dwFlags);

// MyDirectDrawEnumerateExA: restore original DirectDrawCreateEx bytes before calling the real DirectDrawEnumerateExA
HRESULT WINAPI MyDirectDrawEnumerateExA(const LPVOID lpCallback, const LPVOID lpContext, const DWORD dwFlags)
{
	const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
	const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "ddraw.dll");
	if (importDesc)
	{
		const FARPROC* Res = GetImportFunctionAddress(codeBase, importDesc, false, "DirectDrawCreateEx");
		auto Raw = (uChar*)*Res;
		for (uInt i = 0; i < 5; i++)
			PatchAddress<uChar>(&Raw[i], Bak[i]);
	}

	if (hDDraw || (hDDraw = LoadLibraryA("ddraw.dll")))
	{
		const auto Func = (DirectDrawEnumerateExAPtr)GetProcAddress(hDDraw, "DirectDrawEnumerateExA");
		if (Func)
			return Func(lpCallback, lpContext, dwFlags);
	}

	return S_OK;
}

// InstallSteamOverlayFix: patch ddraw.dll DirectDrawEnumerateExA import to fix Steam overlay crashes
bool InstallSteamOverlayFix()
{
	char SteamOverlayFix[256];
	GothicReadIniString("DEBUG", "SteamOverlayFix", "1", SteamOverlayFix, 256, "SystemPack.ini");

	if (atoi(SteamOverlayFix))
	{
		const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
		const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "ddraw.dll");
		if (importDesc)
		{
			const FARPROC* Res = GetImportFunctionAddress(codeBase, importDesc, false, "DirectDrawCreateEx");
			if (Res && *Res)
			{
				const uChar* Raw = (uChar*)*Res;
				memcpy(Bak, Raw, 5);

				const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
				const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "ddraw.dll");
				if (importDesc)
					PatchImportFunctionAddress<FARPROC>(codeBase,
					                                    importDesc,
					                                    false,
					                                    "DirectDrawEnumerateExA",
					                                    (FARPROC)MyDirectDrawEnumerateExA);
			}
		}
	}
	return true;
}
