#ifndef _PATCH_UTILS_
#define _PATCH_UTILS_

class Timer final
{
	LARGE_INTEGER StartTime;
	LARGE_INTEGER Frequency;

public:
	void Start()
	{
		QueryPerformanceCounter(&StartTime);
	};

	LONGLONG GetPrecision()
	{
		return Frequency.QuadPart;
	};

	float GetElapsedTimeSeconds()
	{
		LARGE_INTEGER CurrentTime;
		QueryPerformanceCounter(&CurrentTime);

		return static_cast<float>(CurrentTime.QuadPart - StartTime.QuadPart) / Frequency.QuadPart;
	}

	Timer()
		: StartTime(), Frequency()
	{
		QueryPerformanceFrequency(&Frequency);
	}
	;

	~Timer()
	{};
};

extern void RedirectIOToConsole();

extern bool GetModuleFileNameString(HMODULE hModule, TString& filename);

extern bool GetGothicWindowSize(POINT& size);

extern bool IsVdfs();
extern bool IsSpacer();

#define EXE_CRC_GOTHIC2_NOTR_REPORT_V2 0x2BCD7E30
#define EXE_CRC_GOTHIC2_FIX 0xA2EE682C
#define EXE_CRC_GOTHIC_MOD 0x225BA11E
#define EXE_CRC_GOTHIC_MOD_PATCHED 0x1F8D4811
#define EXE_CRC_GOTHIC 0x2C75C07A

extern uLong GetExeCrc32();

#define CODE_CRC_GOTHIC 0xF6DD31C4

extern uLong GetSectionCrc32(const char* name);
extern const uChar* GetSectionAddress(const uChar* codeBase, const char* name, size_t& size);

extern bool Patch(uChar* data, size_t size, uChar* org, uChar* patch);

// Helper template: patch a single address with a value of any type
template <typename T>
bool PatchAddress(T* address, T value)
{
	DWORD OldProtect;
	if (VirtualProtect(address, sizeof(T), PAGE_READWRITE, &OldProtect))
	{
		*address = value;
		VirtualProtect(address, sizeof(T), OldProtect, &OldProtect);
		return true;
	}
	return false;
}

extern DWORD GetImportSize(const uChar* codeBase);
extern PIMAGE_IMPORT_DESCRIPTOR GetImportDescriptor(const uChar* codeBase, const char* name);
extern FARPROC* GetImportFunctionAddress(const uChar* codeBase,
                                         PIMAGE_IMPORT_DESCRIPTOR importDesc,
                                         bool ordinal,
                                         const char* name);

template <typename type>
bool PatchImportFunctionAddress(const uChar* codeBase,
                                const PIMAGE_IMPORT_DESCRIPTOR importDesc,
                                const bool ordinal,
                                const char* name,
                                type value)
{
	FARPROC* Func = GetImportFunctionAddress(codeBase, importDesc, ordinal, name);
	if (Func)
		return PatchAddress<FARPROC>(Func, value);
	return false;
}

#endif
