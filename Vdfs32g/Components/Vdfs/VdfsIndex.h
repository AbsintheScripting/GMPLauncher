#ifndef _VDFS_INDEX_
#define _VDFS_INDEX_

class IFS;

// VdfsIndex: stores file metadata (name, size, offset, flow pointer) in hash tables for fast lookup
class VdfsIndex final : public Object
{
public:
	// FileInfo: stores metadata for a single file entry (name, size, flow pointer, offset)
	class FileInfo final : public Object
	{
	public:
		AString Name;
		uInt Size;
		IFS* Flow;
		uInt Offset;

		FileInfo()
		{
			Size = 0;
			Flow = nullptr;
			Offset = 0;
		};

		~FileInfo() override
		{};
	};

	using FileInfoPtr = TunablePtr<FileInfo>;

	ObjectArray<FileInfoPtr> Files;
	HashTable<uInt> FullIndexes;
	HashTable<uInt> FileIndexes;

	FileInfo* GetFileInfo(const AString& filename)
	{
		uInt Index = FullIndexes[filename];
		if (Index)
			return Files[Index - 1];
		return nullptr;
	}

	bool SearchFile(const AString& filename, char* fullname)
	{
		uInt Index = FileIndexes[filename];
		if (Index)
		{
			strncpy(fullname, &Files[Index - 1]->Name.GetData()[1], Files[Index - 1]->Name.Length() - 1);
			fullname[Files[Index - 1]->Name.Length() - 1] = '\0';
			return true;
		}
		return false;
	}

	void Clear()
	{
		Files.Clear();
		FileIndexes.Clear();
		FullIndexes.Clear();
	}

	VdfsIndex()
	{};

	~VdfsIndex() override
	{
		Clear();
	};
};

using VdfsIndexPtr = AutoPtr<VdfsIndex>;

#endif
