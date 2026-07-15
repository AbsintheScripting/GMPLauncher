#include "PreCompiled.h"

// Look up a function by name in the loaded Bink DLL and return its address
void* DLL::GetFuncPtr(const char* funcname)
{
	if (hDLL)
		return (void*)GetProcAddress(hDLL, funcname);
	return nullptr;
}

// Constructor: load the specified DLL by name
DLL::DLL(const TCHAR* name)
{
	hDLL = LoadLibrary(name);
}

// Destructor: free the loaded DLL if still open
DLL::~DLL()
{
	if (hDLL)
	{
		FreeLibrary(hDLL);
		hDLL = nullptr;
	}
}

// cBinkDll constructor: initialize all function pointers to NULL
cBinkDll::cBinkDll(const TCHAR* name)
	: DLL(name)
{
	BinkOpen = nullptr;
	BinkGoto = nullptr;
	BinkSetSoundOnOff = nullptr;
	BinkSetVolumeG1 = nullptr;
	BinkSetVolumeG2 = nullptr;
	BinkClose = nullptr;
	BinkWait = nullptr;
	BinkDoFrame = nullptr;
	BinkNextFrame = nullptr;
	BinkPause = nullptr;
	BinkCopyToBuffer = nullptr;
}

// Resolve all Bink function pointers from the loaded DLL
bool cBinkDll::Load()
{
	if (!GetFuncPtr(BINK_GET_ERROR_FUNC_NAME))
		return false;
	if (!(BinkOpen = static_cast<BinkOpenFuncPtr>(GetFuncPtr(BINK_OPEN_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkGoto = static_cast<BinkGotoFuncPtr>(GetFuncPtr(BINK_GOTO_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkSetSoundOnOff = static_cast<BinkSetSoundOnOffFuncPtr>(GetFuncPtr(BINK_SET_SOUND_ON_OFF_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkSetVolumeG1 = static_cast<BinkSetVolumeFuncG1Ptr>(GetFuncPtr(BINK_SET_VOLUME_G1_FUNC_NAME)))
		&& !(BinkSetVolumeG2 = static_cast<BinkSetVolumeFuncG2Ptr>(GetFuncPtr(BINK_SET_VOLUME_G2_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkClose = static_cast<BinkCloseFuncPtr>(GetFuncPtr(BINK_CLOSE_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkWait = static_cast<BinkWaitFuncPtr>(GetFuncPtr(BINK_WAIT_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkDoFrame = static_cast<BinkDoFrameFuncPtr>(GetFuncPtr(BINK_DO_FRAME_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkNextFrame = static_cast<BinkNextFrameFuncPtr>(GetFuncPtr(BINK_NEXT_FRAME_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkPause = static_cast<BinkPauseFuncPtr>(GetFuncPtr(BINK_PAUSE_FUNC_NAME))))
	{
		return false;
	}
	if (!(BinkCopyToBuffer = static_cast<BinkCopyToBufferFuncPtr>(GetFuncPtr(BINK_COPY_TO_BUFFER_FUNC_NAME))))
	{
		return false;
	}
	return true;
}
