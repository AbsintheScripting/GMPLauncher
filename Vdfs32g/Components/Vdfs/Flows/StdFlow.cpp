#include "PreCompiled.h"

bool StdFlow::UpdateFileIndex(const AString& file, const uInt size, const bool failifexists, VdfsIndex* index)
{
	if (!index->FullIndexes[file])
	{
		VdfsIndex::FileInfo* Info = index->Files.Add(new VdfsIndex::FileInfo);
		Info->Flow = this;
		Info->Name = file;
		Info->Size = size;
		index->FullIndexes[Info->Name] = index->Files.Size();

		AString Name(file);
		Name.TruncateBeforeLast("\\");
		Name.ToUpperCase();

		uInt FileIndex = index->FileIndexes[Name];
		if (!FileIndex || (strcmp(Info->Name, index->Files[FileIndex - 1]->Name) < 0))
			index->FileIndexes[Name] = index->Files.Size();

		return true;
	}
	if (!failifexists)
	{
		VdfsIndex::FileInfo* Info = index->Files[static_cast<uInt>(index->FullIndexes[file]) - 1];
		Info->Size = size;
		return true;
	}
	return false;
}

uInt StdFlow::BuildIndex(const TCHAR* dir, VdfsIndex* index)
{
	uInt result = 0;

	TString SearchString(dir);
	SearchString += _T("*");

	WIN32_FIND_DATA findfiledata;
	findfiledata.cFileName[0] = _T('?');
	HANDLE hf = FindFirstFileEx(SearchString,
	                            FindExInfoBasic,
	                            &findfiledata,
	                            FindExSearchNameMatch,
	                            nullptr,
	                            FIND_FIRST_EX_LARGE_FETCH);
	for (BOOL cont = true; (hf != INVALID_HANDLE_VALUE) && (cont != false); cont = FindNextFile(hf, &findfiledata))
	{
		if (findfiledata.cFileName[0] == _T('?'))
			break;
		if (!(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			AString FullName("\\");
			FullName += dir;
			FullName += findfiledata.cFileName;
			FullName.ToUpperCase();

			if (UpdateFileIndex(FullName, findfiledata.nFileSizeLow, true, index))
				result++;
		}
		else
		{
			if (_tcscmp(findfiledata.cFileName, _T(".")) && _tcscmp(findfiledata.cFileName, _T("..")))
			{
				TString DirName(dir);
				DirName += findfiledata.cFileName;
				DirName += _T("\\");
				result += BuildIndex(DirName, index);
			}
		}
	}
	if (hf != INVALID_HANDLE_VALUE)
		FindClose(hf);

	return result;
}

VdfsIndex::FileInfo* StdFlow::GetFileInfo(const AString& filename)
{
	HANDLE hFile = CreateFileA(&filename.GetData()[1],
	                           GENERIC_READ,
	                           FILE_SHARE_READ,
	                           nullptr,
	                           OPEN_EXISTING,
	                           FILE_ATTRIBUTE_NORMAL,
	                           nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		auto Result = new VdfsIndex::FileInfo;
		Result->Flow = this;
		Result->Name = filename;
		Result->Size = ::GetFileSize(hFile, nullptr);

		CloseHandle(hFile);
		return Result;
	}
	return nullptr;
}

bool StdFlow::FileExists(const AString& filename)
{
	return (PathFileExistsA(&filename.GetData()[1]) == TRUE);
}

bool StdFlow::UpdateIndex(VdfsIndex* index)
{
	BuildIndex(_T("_work\\"), index);
	Streams.Add(new StdFlow());
	Streams.Add(new StdFlow());
	return true;
}

StdFlow* StdFlow::GetFreeStream()
{
	for (uInt i = 0; i < Streams.Size(); i++)
	{
		if (!Streams[i]->CurrentFileInfo)
			return Streams[i];
	}
	return Streams.Add(new StdFlow());
}

IFS* StdFlow::Open(VdfsIndex::FileInfoPtr& fileinfo)
{
	if (fileinfo && (fileinfo->Flow == this))
	{
		StdFlow* Result = GetFreeStream();
		if (Result->Init(fileinfo))
			return Result;
	}
	return nullptr;
}

uLong StdFlow::Read(const uLong offset, void* buffer, const uLong size)
{
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD readed = 0;
		SetFilePointer(FileHandle, offset, nullptr, FILE_BEGIN);
		if (ReadFile(FileHandle, buffer, size, &readed, nullptr) && (readed > 0))
			return readed;
	}
	return 0;
}

void StdFlow::Close()
{
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);
		FileHandle = INVALID_HANDLE_VALUE;
	}
	IFS::Close();
}

bool StdFlow::Init(VdfsIndex::FileInfoPtr& fileinfo)
{
	if ((FileHandle != INVALID_HANDLE_VALUE) || CurrentFileInfo)
		return false;

	HANDLE hFile = CreateFileA(&fileinfo->Name.GetData()[1],
	                           GENERIC_READ,
	                           FILE_SHARE_READ,
	                           nullptr,
	                           OPEN_EXISTING,
	                           FILE_ATTRIBUTE_NORMAL,
	                           nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		Name = fileinfo->Name;
		CurrentFileInfo = fileinfo;
		FileHandle = hFile;
		return true;
	}

	return false;
}
