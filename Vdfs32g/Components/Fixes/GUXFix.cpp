#include "PreCompiled.h"
#include <VersionHelpers.h>

// Patch AcGenral.dll: replace GameUX version check with a return to bypass GameUX
bool PatchGUX(uChar* codeBase)
{
	const unsigned char Signature[] = { 0x68, 0x28, 0x09, 0x00, 0x00 };
	const unsigned char SignatureNew[] = { 0xC2, 0x04, 0x00, 0x90, 0x90 };

	const DWORD codeSize = GetImportSize(codeBase);
	if (codeSize > sizeof(Signature))
	{
		for (size_t i = 0; i < (codeSize - sizeof(Signature)); i++)
		{
			if (!memcmp(&codeBase[i], &Signature, sizeof(Signature)))
			{
				DWORD OldProtect = 0;
				if (!VirtualProtect(&codeBase[i], sizeof(Signature), PAGE_READWRITE, &OldProtect))
					return false;
				memcpy(&codeBase[i], SignatureNew, sizeof(Signature));
				VirtualProtect(&codeBase[i], sizeof(Signature), OldProtect, &OldProtect);
				i += sizeof(Signature);
				return true;
			}
		}
	}
	return false;
}

// Install GameUX fix for Vista/7: patch AcGenral.dll to bypass GameUX compatibility checks
bool InstallGUXFix()
{
	char FixGameUX[256];
	if (!GothicReadIniString("DEBUG", "FixGameUX", "1", FixGameUX, 256, "SystemPack.ini"))
		GothicWriteIniString("DEBUG", "FixGameUX", "1", "SystemPack.ini");

	// Only needed on Vista and 7, not on 8+
	if (IsWindowsVistaOrGreater() && !IsWindows8OrGreater() && (atoi(FixGameUX) == 1))
	{
		const HMODULE hGUX = GetModuleHandle(_T("AcGenral.dll"));
		if (hGUX)
			PatchGUX((uChar*)hGUX);
	}

	return true;
}

// No cleanup needed for the GameUX patch
void RemoveGUXFix()
{}
