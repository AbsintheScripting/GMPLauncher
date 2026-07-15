#include "PreCompiled.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

// Tracks whether SystemPack has been attached
bool SystemPackAttached = false;

// Attach all SystemPack fixes if not already done
bool AttachSystemPack()
{
	if (SystemPackAttached)
		return true;

	bool Ok = true;
	if (!IsSpacer())
	{
		// Change working directory to parent if not already in System folder
		bool ChangeWorkDir = false;
		TString WorkPath;
		if (PlatformGetWorkPath(WorkPath) && WorkPath.TruncateBeforeLast(_T("\\")) && WorkPath.Compare(
			_T("System"),
			true))
			ChangeWorkDir = (SetCurrentDirectory(_T("..\\")) == TRUE);

		// Load additional libraries listed in System\pre.load
		TStringArray Libraries;
		if (PlatformReadTextFile(_T("System\\pre.load"), Libraries))
		{
			for (uInt l = 0; l < Libraries.Size(); l++)
			{
				if (!LoadLibrary(TString(_T("System\\")) + Libraries[l]))
				{
					RedirectIOToConsole();
					_tprintf(_T("%s not loaded\n"), Libraries[l].GetData());
				}
			}
		}

		// Restore working directory to System folder after loading
		if (ChangeWorkDir)
			SetCurrentDirectory(_T("System\\"));
	}
	Ok = Ok && InstallFsHook(VdfsBase);
	if (!Ok)
	{
		RedirectIOToConsole();
		printf("InstallFsHook failed\n");
	}
	Ok = Ok && InstallSendMsgFix();
	if (!Ok)
	{
		RedirectIOToConsole();
		printf("InstallSendMsgFix failed\n");
	}
	if (!IsSpacer())
	{
		Ok = Ok && InstallKillerFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallKillerFix failed\n");
		}
		Ok = Ok && InstallD3DFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallD3DFix failed\n");
		}
		Ok = Ok && InstallIniFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallIniFix failed\n");
		}
		Ok = Ok && InstallBinkFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallBinkFix failed\n");
		}
		Ok = Ok && InstallSplashFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallSplashFix failed\n");
		}
		Ok = Ok && InstallSaveBakFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallSaveBakFix failed\n");
		}
		Ok = Ok && InstallMssFix();
		if (!Ok)
		{
			RedirectIOToConsole();
			printf("InstallMssFix failed\n");
		}
	}
	SystemPackAttached = true;
	return Ok;
}

// Hooked GetCommandLineA: loads gmp.dll and attaches SystemPack before returning
LPSTR WINAPI MyGetCommandLineA()
{
	AttachSystemPack();
	return GetCommandLineA();
}

// Install fix for GameUX (AcGenral.dll) and optionally hook GetCommandLineA via IAT patching
bool AttachFixesInstaller()
{
	auto codeBase = (uChar*)GetModuleHandle(nullptr);
	PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "Kernel32.dll");
	if (importDesc)
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "GetCommandLineA", (FARPROC)MyGetCommandLineA);

	bool Ok = InstallGUXFix();
	if (!Ok)
	{
		RedirectIOToConsole();
		printf("InstallGUXFix failed\n");
	}
	//Ok = Ok && InstallSteamOverlayFix();
	//if(!Ok)
	//{
	//	RedirectIOToConsole();
	//	printf("InstallSteamOverlayFix failed\n");
	//}
	return Ok;
}

// Remove all installed fixes when the DLL is detached
void RemoveFixes()
{
	if (!IsVdfs() && !IsSpacer())
	{
		RemoveMssFix();
		RemoveSplashFix();
		RemoveBinkFix();
		RemoveIniFix();
		RemoveD3DFix();
		RemoveKillerFix();
	}
}
