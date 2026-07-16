#include "PreCompiled.h"

// FileExist: check if a file exists in virtual or physical storage, respecting VDF_PHYSICALFIRST flag
long Vdfs::FileExist(const char* filename, const long flags)
{
	if (flags & VDF_PACKED)
		MessageBox(nullptr, _T("vdf_fexists unsupported flag VDF_PACKED"), _T("VDFS"), MB_ICONERROR);
	if (flags & VDF_NOBUFFER)
		MessageBox(nullptr, _T("vdf_fexists unsupported flag VDF_NOBUFFER"), _T("VDFS"), MB_ICONERROR);

	EnterCriticalSection(&CS);
	if ((flags & VDF_PHYSICALFIRST) && (flags & VDF_PHYSICAL))
	{
		if (PhysicalFlow->FileExists(filename))
		{
			LeaveCriticalSection(&CS);
			return VDF_PHYSICAL;
		}
	}
	if (flags & VDF_VIRTUAL)
	{
		VdfsIndex::FileInfo* FileInfo = Index->GetFileInfo(filename);
		if (FileInfo)
		{
			long Result = (FileInfo->Flow == PhysicalFlow) ? VDF_PHYSICAL : VDF_VIRTUAL;
			LeaveCriticalSection(&CS);
			return Result;
		}
	}
	if ((!(flags & VDF_PHYSICALFIRST)) && (flags & VDF_PHYSICAL))
	{
		if (PhysicalFlow->FileExists(filename))
		{
			LeaveCriticalSection(&CS);
			return VDF_PHYSICAL;
		}
	}
	LeaveCriticalSection(&CS);
	return 0;
}

// SearchFile: case-insensitive file search in the VDFS index, returns full canonical path
bool Vdfs::SearchFile(const char* filename, char* fullname)
{
	EnterCriticalSection(&CS);
	String UpperName(filename);
	UpperName.ToUpperCase();
	bool Result = Index->SearchFile(UpperName, fullname);
	//printf("%s -> %s\n", filename, fullname);
	LeaveCriticalSection(&CS);
	return Result;
}

// ApplyFilter: chain all registered filters (e.g. OggFilter) over a source IFS stream
IfsBase* Vdfs::ApplyFilter(IFS* src)
{
	for (uInt f = 0; src && (f < Filters.Size()); f++)
	{
		IfsBase* Res = Filters[f]->Open(src);
		if (Res)
			return Res;
	}
	return src;
}

// OpenFile: open a file from virtual (VDF/MOD) or physical storage with filter chain
IfsBase* Vdfs::OpenFile(const char* filename, const long flags)
{
	if (flags & VDF_PACKED)
		MessageBox(nullptr, _T("vdf_fopen unsupported flag VDF_PACKED"), _T("VDFS"), MB_ICONERROR);
	if (flags & VDF_NOBUFFER)
		MessageBox(nullptr, _T("vdf_fopen unsupported flag VDF_NOBUFFER"), _T("VDFS"), MB_ICONERROR);

	EnterCriticalSection(&CS);

	if ((flags & VDF_PHYSICALFIRST) && (flags & VDF_PHYSICAL))
	{
		VdfsIndex::FileInfoPtr FileInfo = PhysicalFlow->GetFileInfo(filename);
		if (FileInfo)
		{
			IFS* Result = FileInfo->Flow->Open(FileInfo);
			LeaveCriticalSection(&CS);
			return ApplyFilter(Result);
		}
	}
	if (flags & VDF_VIRTUAL)
	{
		VdfsIndex::FileInfoPtr FileInfo = Index->GetFileInfo(filename);
		if (FileInfo)
		{
			FileInfo.SetAuto(false); // In index file infos are created not by new
			IFS* Result = FileInfo->Flow->Open(FileInfo);
			LeaveCriticalSection(&CS);
			return ApplyFilter(Result);
		}
	}
	if ((!(flags & VDF_PHYSICALFIRST)) && (flags & VDF_PHYSICAL))
	{
		VdfsIndex::FileInfoPtr FileInfo = PhysicalFlow->GetFileInfo(filename);
		if (FileInfo)
		{
			IFS* Result = FileInfo->Flow->Open(FileInfo);
			LeaveCriticalSection(&CS);
			return ApplyFilter(Result);
		}
	}
	LeaveCriticalSection(&CS);
	return nullptr;
}

// CloseFile: close a virtual file stream
void Vdfs::CloseFile(IfsBase* fp)
{
	EnterCriticalSection(&CS);
	if (fp)
		fp->Close();
	LeaveCriticalSection(&CS);
}

// VdfCompTimeStamp: comparator for sorting VDF flows by modification timestamp (newer first)
int VdfCompTimeStamp(const IfsPtr& Obj1, const IfsPtr& Obj2, Object* arg)
{
	if ((Obj1->GetType() == IFS_TYPE_VDF) && (Obj2->GetType() == IFS_TYPE_VDF))
	{
		VdfTime Time1 = dcast<const VdfFlow*>(static_cast<const IFS*>(Obj1))->GetTimeStamp();
		VdfTime Time2 = dcast<const VdfFlow*>(static_cast<const IFS*>(Obj2))->GetTimeStamp();
		if (*(uInt*)&Time1 == *(uInt*)&Time2)
			return 0;
		if (*(uInt*)&Time1 < *(uInt*)&Time2)
			return 1;
		return -1;
	}
	return 0;
}

// EnumDirs: recursively enumerate subdirectories under a path
void Vdfs::EnumDirs(const TCHAR* dir, TStringArray& dirs, const bool recurse)
{
	TString SearchString(dir);
	SearchString += _T("*");

	WIN32_FIND_DATA findfiledata;
	findfiledata.cFileName[0] = _T('?');
	HANDLE hf = FindFirstFile(SearchString, &findfiledata);
	for (BOOL cont = true; (hf != INVALID_HANDLE_VALUE) && (cont != false); cont = FindNextFile(hf, &findfiledata))
	{
		if (findfiledata.cFileName[0] == _T('?'))
			break;
		if (findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_tcscmp(findfiledata.cFileName, _T(".")) && _tcscmp(findfiledata.cFileName, _T("..")))
			{
				TString& DirName = dirs.Add();
				DirName = dir;
				DirName += findfiledata.cFileName;
				DirName += _T("\\");
				if (recurse)
					EnumDirs(DirName, dirs, recurse);
			}
		}
	}
	if (hf != INVALID_HANDLE_VALUE)
		FindClose(hf);
}

// MountDir: scan a directory for .vdf/.mod files and add them as virtual flows
void Vdfs::MountDir(const TCHAR* dir, const TCHAR* ext)
{
	TStringArray Dirs;
	Dirs.Add(dir);
	//EnumDirs(dir, Dirs, false);

	for (uInt i = 0; i < Dirs.Size(); i++)
	{
		TString SearchString(Dirs[i]);
		SearchString += ext;

		WIN32_FIND_DATA findfiledata;
		findfiledata.cFileName[0] = _T('?');
		HANDLE hf = FindFirstFile(SearchString, &findfiledata);
		for (BOOL cont = true; (hf != INVALID_HANDLE_VALUE) && (cont != false); cont = FindNextFile(hf, &findfiledata))
		{
			if (findfiledata.cFileName[0] == _T('?'))
				break;
			if (!(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TString ArcName(Dirs[i]);
				ArcName += findfiledata.cFileName;
				VdfFlowPtr Flow = new VdfFlow;
				if (Flow->Init(ArcName))
					VirtualFlows.Add(Flow);
			}
		}
		if (hf != INVALID_HANDLE_VALUE)
			FindClose(hf);
	}
}

// UpdateStdFileIndex: update the physical flow's index for a specific file
bool Vdfs::UpdateStdFileIndex(const AString& file, const uInt size)
{
	if (PhysicalFlow && Index)
		return dcast<StdFlow*>(PhysicalFlow)->UpdateFileIndex(file, size, false, Index);
	return true;
}

// InitVirtual: mount all .vdf and .mod files from Data/, sort by timestamp, create physical StdFlow and index
bool Vdfs::InitVirtual()
{
	EnterCriticalSection(&CS);

	if (!VirtualFlows.Size())
	{
		MountDir(_T("Data\\"), _T("*.vdf"));
		MountDir(_T("Data\\"), _T("*.mod"));

		if (VirtualFlows.Size() > 2)
		{
			VirtualFlows.SetComparator(VdfCompTimeStamp);
			VirtualFlows.SortSubSet(0, VirtualFlows.Size() - 1);
		}
		PhysicalFlow = new StdFlow();

		Index = new VdfsIndex();
		for (uInt i = 0; i < VirtualFlows.Size(); i++)
			VirtualFlows[i]->UpdateIndex(Index);
	}

	LeaveCriticalSection(&CS);
	return true;
}

// Init: register all filters (e.g. OggFilter), initialize virtual storage, and build the complete file index
bool Vdfs::Init()
{
	EnterCriticalSection(&CS);

	Filters.Add(new OggFilter);

	if (!Index)
		InitVirtual();
	PhysicalFlow->UpdateIndex(Index);

	LeaveCriticalSection(&CS);
	return true;
}

// Clear: release all virtual flows and the physical flow
void Vdfs::Clear()
{
	EnterCriticalSection(&CS);
	PhysicalFlow = nullptr;
	VirtualFlows.Clear();
	LeaveCriticalSection(&CS);
}

// Vdfs constructor: initialize the critical section and set PhysicalFlow to null
Vdfs::Vdfs()
{
	InitializeCriticalSection(&CS);
	PhysicalFlow = nullptr;
}

// Vdfs destructor: clear all resources and delete the critical section
Vdfs::~Vdfs()
{
	Clear();
	DeleteCriticalSection(&CS);
}
