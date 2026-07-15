#include "PreCompiled.h"

#include "mssds3d_m3d.h"
#include "mssds3dh_m3d.h"

// Extract the G2 MSS DLL from embedded resources to a temp file
bool ExtractLibG2(TString& name)
{
	bool Result = false;
	if (PlatformGetTempPath(name))
	{
		name += _T(MSSDS3D_M3D);
		FILE* temp = _tfopen(name, _T("wb"));
		if (temp)
		{
			fwrite(MssDS3D_m3d, 1, sizeof(MssDS3D_m3d), temp);
			Result = true;
			fclose(temp);
		}
	}
	return Result;
}

// Extract the G1 MSS DLL from embedded resources to a temp file
bool ExtractLibG1(TString& name)
{
	bool Result = false;
	if (PlatformGetTempPath(name))
	{
		name += _T(MSSDS3DH_M3D);
		FILE* temp = _tfopen(name, _T("wb"));
		if (temp)
		{
			fwrite(Mssds3dh_m3d, 1, sizeof(Mssds3dh_m3d), temp);
			Result = true;
			fclose(temp);
		}
	}
	return Result;
}

// Hooked LoadLibraryA: intercept mssds3d.m3d/mssds3dh.m3d loads and return the embedded version
HMODULE WINAPI MyLoadLibraryA(const LPCSTR lpLibFileName)
{
	const char* file = strrchr(lpLibFileName, '\\');
	if (file)
	{
		// Intercept G2 MSS DLL load
		if (!_stricmp(file + 1, "mssds3d.m3d"))
		{
			TString LibName;
			if (ExtractLibG2(LibName))
				return LoadLibrary(LibName);
		}
		else
		// Intercept G1 MSS DLL load
			if (!_stricmp(file + 1, "mssds3dh.m3d"))
			{
				TString LibName;
				if (ExtractLibG1(LibName))
					return LoadLibrary(LibName);
			}
	}
	return LoadLibraryA(lpLibFileName);
}

// Install MSS audio fix by patching Mss32.dll's LoadLibraryA import
bool InstallMssFix()
{
	char FixMss[256];
	if (!GothicReadIniString("DEBUG", "FixMss", "1", FixMss, 256, "SystemPack.ini"))
		GothicWriteIniString("DEBUG", "FixMss", "1", "SystemPack.ini");

	// Only needed on XP+
	if (IsWindowsXPOrGreater() && (atoi(FixMss) == 1))
	{
		const uChar* codeBase = (uChar*)GetModuleHandle(_T("Mss32.dll"));
		if (codeBase)
		{
			const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "KERNEL32.dll");
			if (!importDesc)
				return false;
			// Patch LoadLibraryA to redirect MSS DLL loading to embedded version
			return PatchImportFunctionAddress<FARPROC>(codeBase,
			                                           importDesc,
			                                           false,
			                                           "LoadLibraryA",
			                                           (FARPROC)MyLoadLibraryA);
		}
	}
	return true;
}

// No cleanup needed for the MSS fix
void RemoveMssFix()
{}
