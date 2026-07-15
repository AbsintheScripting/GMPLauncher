#ifndef _DLL_
#define _DLL_

class DLL
{
protected:
	HMODULE hDLL;

public:
	void* GetFuncPtr(const char* name);
	explicit DLL(const TCHAR* name);
	virtual ~DLL();
};

class cBinkDll final : public DLL
{
public:
	BinkOpenFuncPtr BinkOpen;
	BinkGotoFuncPtr BinkGoto;
	BinkSetSoundOnOffFuncPtr BinkSetSoundOnOff;
	BinkSetVolumeFuncG1Ptr BinkSetVolumeG1;
	BinkSetVolumeFuncG2Ptr BinkSetVolumeG2;
	BinkCloseFuncPtr BinkClose;
	BinkWaitFuncPtr BinkWait;
	BinkDoFrameFuncPtr BinkDoFrame;
	BinkNextFrameFuncPtr BinkNextFrame;
	BinkPauseFuncPtr BinkPause;
	BinkCopyToBufferFuncPtr BinkCopyToBuffer;

	cBinkDll() = delete;
	explicit cBinkDll(const TCHAR* name);
	bool Load();

	~cBinkDll() override
	{};
};

#endif
