#ifndef _STD_FLOW_
#define _STD_FLOW_

class StdFlow final : public IFS
{
protected:
	HANDLE FileHandle = INVALID_HANDLE_VALUE;

	ObjectArray<AutoPtr<StdFlow>> Streams;

	uInt BuildIndex(const TCHAR* dir, VdfsIndex* index);
	StdFlow* GetFreeStream();

	uLong Read(uLong offset, void* buffer, uLong size) override;

public:
	int GetType() const override;

	VdfsIndex::FileInfo* GetFileInfo(const AString& filename) override;
	bool FileExists(const AString& filename) override;

	virtual bool UpdateFileIndex(const AString& file, uInt size, bool failifexists, VdfsIndex* index);
	bool UpdateIndex(VdfsIndex* index) override;

	IFS* Open(VdfsIndex::FileInfoPtr& fileinfo) override;
	uLong GetFileSize() const override;
	void Close() override;

	uInt GetStreamsSize() override
	{
		return Streams.Size();
	};

	bool Init(VdfsIndex::FileInfoPtr& fileinfo);

	StdFlow();
	~StdFlow() override;
};

inline int StdFlow::GetType() const
{
	return IFS_TYPE_STDIO;
}

inline uLong StdFlow::GetFileSize() const
{
	return (CurrentFileInfo ? CurrentFileInfo->Size : 0);
}

inline StdFlow::StdFlow()
{
	Name = "stdio";
	FileHandle = INVALID_HANDLE_VALUE;
	CurrentFileInfo = nullptr;
}

inline StdFlow::~StdFlow()
{
	Close();
}

#endif
