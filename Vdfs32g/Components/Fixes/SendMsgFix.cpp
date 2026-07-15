#include "PreCompiled.h"

// Hooked SendMessageA: use SendMessageTimeout for HWND_BROADCAST to prevent hangs
LRESULT WINAPI MySendMessageA(const HWND hWnd, const UINT Msg, const WPARAM wParam, const LPARAM lParam)
{
	// Use timeout-based send for broadcast messages to avoid potential deadlocks
	if (hWnd == HWND_BROADCAST)
		return SendMessageTimeoutA(hWnd, Msg, wParam, lParam, SMTO_BLOCK, 100, nullptr);
	return SendMessageA(hWnd, Msg, wParam, lParam);
}

// Install SendMessage fix by patching USER32.dll SendMessageA import
bool InstallSendMsgFix()
{
	const uChar* codeBase = (uChar*)GetModuleHandle(nullptr);
	const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "USER32.dll");
	if (importDesc)
		PatchImportFunctionAddress<FARPROC>(codeBase, importDesc, false, "SendMessageA", (FARPROC)MySendMessageA);
	return true;
}

// No cleanup needed for the SendMessage fix
void RemoveSendMsgFix()
{}
