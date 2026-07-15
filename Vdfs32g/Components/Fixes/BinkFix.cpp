#include "PreCompiled.h"
#include "Common/ComPtr.h"
#include <wincodec.h>

// Initialize COM with a fallback chain: OleInitialize → CoInitialize → CoInitializeEx
HRESULT InitCom()
{
	// Due too lack of functionality in other modes (~ no DragDrop support)
	// try OleInitialize first

	HRESULT hRes = S_OK;
	if (SUCCEEDED(hRes = OleInitialize(NULL)))
		return hRes;

	if (hRes == RPC_E_CHANGED_MODE && SUCCEEDED(hRes = CoInitialize(NULL)))
		return hRes;

	if (hRes == RPC_E_CHANGED_MODE && SUCCEEDED(hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return hRes;

	return hRes;
}

// Per-instance Bink fix data: tracks source/dst dimensions and WIC scaler resources
struct BinkFix
{
	BINK* Bink;
	uInt SrcWidth;
	uInt SrcHeight;

	uInt DstWidth;
	uInt DstHeight;

	ComPtr<IWICBitmap> Image;
	uInt SrcX;
	uInt SrcY;

	ComPtr<IWICBitmapScaler> pIScaler;
};

// Global state: WIC factory and array of active Bink fix instances
HRESULT hRes;
ComPtr<IWICImagingFactory> pImgFac;
Array<BinkFix> Binks;

// Find the index of a Bink instance in the global array (returns 0 if not found)
uInt GetBinkIndex(BINK* bnk)
{
	for (uInt i = 0; i < Binks.Size(); i++)
	{
		if (Binks[i].Bink == bnk)
			return (i + 1);
	}
	return 0;
}

// Global flag controlling whether video scaling is enabled
BOOL scaleVideos = TRUE;

// Validate and compute aspect-ratio-corrected source crop and destination scaler
void ValidateAspect(BinkFix& Fix, uInt destx, uInt desty)
{
	uInt ImgWidth = Fix.SrcWidth;
	uInt ImgHeight = Fix.SrcHeight;

	Fix.SrcX = 0;
	Fix.SrcY = 0;

	// Compute letterbox/pillarbox crop to match destination aspect ratio
	const float SrcAspect = static_cast<float>(Fix.SrcWidth) / static_cast<float>(Fix.SrcHeight);
	const float DstAspect = static_cast<float>(Fix.DstWidth) / static_cast<float>(Fix.DstHeight);
	if (DstAspect < SrcAspect)
	{
		ImgHeight = static_cast<uInt>(static_cast<float>(Fix.SrcWidth) / DstAspect);
		ImgHeight -= ImgHeight % 2;
		Fix.SrcY += (ImgHeight - Fix.SrcHeight) / 2;
	}
	else if (DstAspect > SrcAspect)
	{
		ImgWidth = static_cast<int>(static_cast<float>(Fix.SrcHeight) * DstAspect);
		ImgWidth -= ImgWidth % 2;
		Fix.SrcX += (ImgWidth - Fix.SrcWidth) / 2;
	}

	// Recreate WIC bitmap/scaler if dimensions changed
	if (Fix.Image)
	{
		UINT width, height;
		hRes = Fix.Image->GetSize(&width, &height);
		if ((width != ImgWidth) || (height != ImgHeight))
		{
			Fix.Image = nullptr;
		}
	}
	if (!Fix.Image)
	{
		// Create WIC bitmap and scaler for upscaling/letterboxing
		if (FAILED(
			hRes = pImgFac->CreateBitmap(ImgWidth, ImgHeight, GUID_WICPixelFormat32bppBGR, WICBitmapCacheOnDemand, Fix.
				Image.GetAddressOf())))
		{
			Fix.Image = nullptr;
		}
		if (FAILED(hRes = pImgFac->CreateBitmapScaler(Fix.pIScaler.GetAddressOf())))
		{
			Fix.pIScaler = nullptr;
		}
		if (FAILED(
			hRes = Fix.pIScaler->Initialize(Fix.Image, Fix.DstWidth, Fix.DstHeight, WICBitmapInterpolationModeFant)))
		{
			Fix.pIScaler = nullptr;
		}
	}
}

// Create a new Bink fix instance: allocate WIC resources and set scaled dimensions
void CreateBinkFix(BINK* bnk)
{
	char Buffer[256];
	GothicReadIniString("GAME", "scaleVideos", "1", Buffer, 256, "Gothic.ini");
	scaleVideos = atoi(Buffer);

	if (!bnk || !scaleVideos)
		return;

	POINT GothicWindowSize = { 0, 0 };
	if (!GetGothicWindowSize(GothicWindowSize))
		return;

	const uInt Index = GetBinkIndex(bnk);
	if (!Index)
	{
		BinkFix& Fix = Binks.Add();

		Fix.Bink = bnk;
		Fix.SrcWidth = bnk->Width;
		Fix.SrcHeight = bnk->Height;
		Fix.DstWidth = GothicWindowSize.x;
		Fix.DstHeight = GothicWindowSize.y;

		ValidateAspect(Fix, 0, 0);

		// Override Bink dimensions so the game renders to the window size
		bnk->Width = Fix.DstWidth;
		bnk->Height = Fix.DstHeight;
	}
}

// Apply previously computed fix dimensions to a Bink instance
bool ApplyBinkFix(BINK* bnk)
{
	if (!bnk || !scaleVideos)
		return false;

	const uInt Index = GetBinkIndex(bnk);
	if (Index)
	{
		const BinkFix& Fix = Binks[Index - 1];
		bnk->Width = Fix.DstWidth;
		bnk->Height = Fix.DstHeight;
		return true;
	}
	return false;
}

// Hooked BinkCopyToBuffer: scale the video frame using WIC before copying to destination
int BinkFixBinkCopyToBuffer(cBinkDll* dll,
                            BINK* bnk,
                            void* dest,
                            const int destpitch,
                            const uInt destheight,
                            const uInt destx,
                            const uInt desty,
                            const uInt flags)
{
	// Only scale 32-bit videos when scaling is enabled
	if (!bnk || !(flags & BINK_SURFACE_32) || !scaleVideos)
		return dll->BinkCopyToBuffer(bnk, dest, destpitch, destheight, destx, desty, flags);

	const uInt Index = GetBinkIndex(bnk);
	if (Index)
	{
		int res = 0;
		const BinkFix& Fix = Binks[Index - 1];
		if (Fix.Image)
		{
			// Should be in {} for destroying all 'srcLock' related objects before using 'Image'
			{
				UINT width, height;
				hRes = Fix.Image->GetSize(&width, &height);
				const WICRect rect{ 0, 0, static_cast<INT>(width), static_cast<INT>(height) };

				ComPtr<IWICBitmapLock> srcLock;
				hRes = Fix.Image->Lock(&rect, WICBitmapLockWrite, srcLock.GetAddressOf());
				UINT size, stride;
				BYTE* data;
				srcLock->GetDataPointer(&size, &data);
				srcLock->GetStride(&stride);

				// Render original frame into WIC bitmap, then scale to destination
				bnk->Width = Fix.SrcWidth;
				bnk->Height = Fix.SrcHeight;
				res = dll->BinkCopyToBuffer(bnk, data, stride, height, Fix.SrcX, Fix.SrcY, flags);
				bnk->Width = Fix.DstWidth;
				bnk->Height = Fix.DstHeight;
			}
			if (Fix.pIScaler)
			{
				const UINT dstBufferSize = destpitch * destheight;
				hRes = Fix.pIScaler->CopyPixels(nullptr, destpitch, dstBufferSize, static_cast<BYTE*>(dest));
				return res;
			}
		}
	}

	return dll->BinkCopyToBuffer(bnk, dest, destpitch, destheight, destx, desty, flags);
}

// Cancel fix: restore original Bink dimensions
bool CancelBinkFix(BINK* bnk)
{
	if (!bnk || !scaleVideos)
		return false;

	const uInt Index = GetBinkIndex(bnk);
	if (Index)
	{
		const BinkFix& Fix = Binks[Index - 1];
		bnk->Width = Fix.SrcWidth;
		bnk->Height = Fix.SrcHeight;
		return true;
	}
	return false;
}

// Delete fix: restore dimensions and remove instance from array
bool DeleteBinkFix(BINK* bnk)
{
	if (!bnk || !scaleVideos)
		return false;

	const uInt Index = GetBinkIndex(bnk);
	if (Index)
	{
		const BinkFix& Fix = Binks[Index - 1];
		bnk->Width = Fix.SrcWidth;
		bnk->Height = Fix.SrcHeight;
		Binks.EraseIndex(Index - 1);
		return true;
	}
	return false;
}

// Overrides

cBinkDll* OrgDll = nullptr;

// Hooked BinkSetSoundOnOff: cancel and reapply fix around the original call
int __stdcall BinkSetSoundOnOff(BINK* bnk, const int onoff)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		const int res = OrgDll->BinkSetSoundOnOff(bnk, onoff);
		ApplyBinkFix(bnk);
		return res;
	}
	return 0;
}

// Hooked BinkNextFrame: cancel and reapply fix around the original call
void __stdcall BinkNextFrame(BINK* bnk)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		OrgDll->BinkNextFrame(bnk);
		ApplyBinkFix(bnk);
	}
}

// Hooked BinkGoto: cancel and reapply fix around the original call
void __stdcall BinkGoto(BINK* bnk, const uInt frame, const int flags)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		OrgDll->BinkGoto(bnk, frame, flags);
		ApplyBinkFix(bnk);
	}
}

// Hooked BinkWait: cancel and reapply fix around the original call
int __stdcall BinkWait(BINK* bnk)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		const int res = OrgDll->BinkWait(bnk);
		ApplyBinkFix(bnk);
		return res;
	}
	return 0;
}

// Hooked BinkCopyToBuffer: delegate to the scaling wrapper
int __stdcall BinkCopyToBuffer(BINK* bnk,
                               void* dest,
                               const int destpitch,
                               const uInt destheight,
                               const uInt destx,
                               const uInt desty,
                               const uInt flags)
{
	if (OrgDll)
		return BinkFixBinkCopyToBuffer(OrgDll, bnk, dest, destpitch, destheight, destx, desty, flags);
	return 0;
}

// Hooked BinkDoFrame: cancel and reapply fix around the original call
int __stdcall BinkDoFrame(BINK* bnk)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		const int res = OrgDll->BinkDoFrame(bnk);
		ApplyBinkFix(bnk);
		return res;
	}
	return 0;
}

// Hooked BinkOpen: create a scaling fix instance for the opened video
BINK* __stdcall BinkOpen(const char* name, const uInt flags)
{
	if (OrgDll)
	{
		// if resolution is higher than game window size video will be skipped, 
		// so we tell Gothic video have correct resolution and scale video internally
		BINK* res = OrgDll->BinkOpen(name, flags);
		CreateBinkFix(res);
		return res;
	}
	return nullptr;
}

// Hooked BinkClose: delete the scaling fix before closing
void __stdcall BinkClose(BINK* bnk)
{
	if (OrgDll)
	{
		DeleteBinkFix(bnk);
		OrgDll->BinkClose(bnk);
	}
}

// Hooked BinkSetVolumeG2: cancel and reapply fix around the original call
void __stdcall BinkSetVolumeG2(BINK* bnk, const uInt trackid, const int volume)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		if (OrgDll->BinkSetVolumeG2)
			OrgDll->BinkSetVolumeG2(bnk, trackid, volume);
		else
			OrgDll->BinkSetVolumeG1(bnk, volume);
		ApplyBinkFix(bnk);
	}
}

// Hooked BinkSetVolumeG1: cancel and reapply fix around the original call
void __stdcall BinkSetVolumeG1(BINK* bnk, const int volume)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		if (OrgDll->BinkSetVolumeG1)
			OrgDll->BinkSetVolumeG1(bnk, volume);
		else
			OrgDll->BinkSetVolumeG2(bnk, 0, volume);
		ApplyBinkFix(bnk);
	}
}

// Hooked BinkPause: cancel and reapply fix around the original call
int __stdcall BinkPause(BINK* bnk, const int pause)
{
	if (OrgDll)
	{
		CancelBinkFix(bnk);
		const int res = OrgDll->BinkPause(bnk, pause);
		ApplyBinkFix(bnk);
		return res;
	}
	return 0;
}

#include <intrin.h>

// Install the Bink video scaling fix by patching binkw32.dll IAT
bool InstallBinkFix()
{
	char FixBink[256];
	if (!GothicReadIniString("DEBUG", "FixBink", "1", FixBink, 256, "SystemPack.ini"))
	{
		GothicWriteIniString("DEBUG", "FixBink", "1", "SystemPack.ini");
	}

	if (atoi(FixBink) != 1)
		return true;

	// Initialize COM and WIC imaging factory
	if (FAILED(hRes = InitCom()))
	{
		return false;
	}
	if (FAILED(
		hRes = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(pImgFac.
			GetAddressOf()))))
	{
		return false;
	}

	const auto codeBase = (uChar*)GetModuleHandle(nullptr);
	const PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImportDescriptor(codeBase, "binkw32.dll");
	if (importDesc == nullptr)
		return false;

	// Load BinkW32.dll and resolve all function pointers
	OrgDll = new cBinkDll(_T("BinkW32.dll"));
	if (!OrgDll->Load())
	{
		MessageBox(nullptr, _T("Invalid binkw32.dll"), _T("Error"), MB_ICONERROR);
		delete OrgDll;
		OrgDll = nullptr;
		return false;
	}

	// Patch all Bink function imports to our hook wrappers
	bool Ok = true;
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkSetSoundOnOff@8",
	                                               (FARPROC)BinkSetSoundOnOff);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkClose@4",
	                                               (FARPROC)BinkClose);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkNextFrame@4",
	                                               (FARPROC)BinkNextFrame);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkGoto@12",
	                                               (FARPROC)BinkGoto);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkWait@4",
	                                               (FARPROC)BinkWait);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkCopyToBuffer@28",
	                                               (FARPROC)BinkCopyToBuffer);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkDoFrame@4",
	                                               (FARPROC)BinkDoFrame);
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkOpen@8",
	                                               (FARPROC)BinkOpen);
	Ok = Ok && (PatchImportFunctionAddress<FARPROC>(codeBase,
	                                                importDesc,
	                                                false,
	                                                "_BinkSetVolume@12",
	                                                (FARPROC)BinkSetVolumeG2) ||
		PatchImportFunctionAddress<FARPROC>(codeBase,
		                                    importDesc,
		                                    false,
		                                    "_BinkSetVolume@8",
		                                    (FARPROC)BinkSetVolumeG1));
	Ok = Ok && PatchImportFunctionAddress<FARPROC>(codeBase,
	                                               importDesc,
	                                               false,
	                                               "_BinkPause@8",
	                                               (FARPROC)BinkPause);
	return Ok;
}

// Remove the Bink fix by freeing the original DLL handle
void RemoveBinkFix()
{
	if (OrgDll)
	{
		delete OrgDll;
		OrgDll = nullptr;
	}
}
