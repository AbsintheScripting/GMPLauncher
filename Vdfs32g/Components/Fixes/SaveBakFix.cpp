#include "PreCompiled.h"
#include <string>
#include <filesystem>

// Hooked DeleteFileA: move save files to a bak_ subdirectory instead of deleting them
BOOL WINAPI MyDeleteFileA(
	__in const LPCSTR lpFileName
)
{
	auto savegame = "savegame";
	// Intercept savegame file deletions
	if (!strncmp(lpFileName, savegame, sizeof(savegame)))
	{
		std::string savegameBak("bak_");
		savegameBak += lpFileName;

		// Create backup directory structure
		std::string savegameBakDir(savegameBak);
		savegameBakDir.erase(savegameBakDir.find("\\"));
		std::filesystem::create_directories(savegameBakDir);

		// Remove any existing backup and move save file to backup location
		DeleteFileA(savegameBak.c_str());
		return MoveFileA(lpFileName, savegameBak.c_str());
	}
	return DeleteFileA(lpFileName);
}

// Install save game backup fix by patching KERNEL32.dll DeleteFileA import
bool InstallSaveBakFix()
{
	char FixSaveBak[256];
	if (!GothicReadIniString("DEBUG", "FixSaveBak", "1", FixSaveBak, 256, "SystemPack.ini"))
		GothicWriteIniString("DEBUG", "FixSaveBak", "1", "SystemPack.ini");

	if (atoi(FixSaveBak) != 1)
	{
		return true;
	}

	const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
	const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "KERNEL32.dll");
	if (importDesc)
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "DeleteFileA", (FARPROC)MyDeleteFileA);
	return true;
}

// No cleanup needed for the save backup fix
void RemoveSaveBakFix()
{}
