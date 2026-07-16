#include "PreCompiled.h"

// ExtractSplash: extract SPLASH.BMP from the VDF archive to a temp file
bool ExtractSplash(TString& name)
{
	bool Result = false;

	bool ChangeWorkDir = false;
	TString WorkPath;
	if (PlatformGetWorkPath(WorkPath) && WorkPath.TruncateBeforeLast(_T("\\")) && WorkPath.Compare(_T("System"), true))
		ChangeWorkDir = (SetCurrentDirectory(_T("..\\")) == TRUE);

	if (!vdf_initall_internal())
	{
		char SplashName[256];
		SplashName[0] = '\\';
		if (vdf_searchfile("SPLASH.BMP", &SplashName[1]))
		{
			const long splash = vdf_fopen(SplashName, VDF_VIRTUAL);
			if (splash > 0)
			{
				TString TempFile;
				if (vdf_ffilesize(splash) && PlatformGetTempFileName(TempFile))
				{
					FILE* temp = _tfopen(TempFile, _T("wb"));
					if (temp)
					{
						char Buffer[256];
						long readed = 0;
						while (readed = vdf_fread(splash, Buffer, 256))
							fwrite(Buffer, 1, readed, temp);
						name = TempFile;
						Result = true;
						fclose(temp);
					}
				}
				vdf_fclose(splash);
			}
		}
	}

	if (ChangeWorkDir)
		SetCurrentDirectory(_T("System\\"));
	return Result;
}

// MyLoadBitmapA: hooked LoadBitmapA that intercepts bitmap ID 169 and loads custom splash.bmp instead
HBITMAP WINAPI MyLoadBitmapA(const HINSTANCE hInstance, LPCSTR lpBitmapName)
{
	if ((uInt)lpBitmapName == 169)
	{
		TString TempName;
		if (ExtractSplash(TempName))
		{
			const auto hBMP = static_cast<HBITMAP>(LoadImage(NULL, TempName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
			if (hBMP)
				return hBMP;
		}

		const auto hBMP = static_cast<HBITMAP>(LoadImage(NULL, _T("Splash.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		if (hBMP)
			return hBMP;
	}

	return LoadBitmapA(hInstance, lpBitmapName);
}

// InstallSplashFix: patch USER32.dll LoadBitmapA import to redirect bitmap ID 169 to custom splash
bool InstallSplashFix()
{
	const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
	const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "USER32.dll");
	if (importDesc)
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "LoadBitmapA", (FARPROC)MyLoadBitmapA);
	return true;
}

// RemoveSplashFix: no cleanup needed for the splash fix
void RemoveSplashFix()
{}
