#ifndef _IFS_
#define _IFS_

#define VDFS_BUFFER_SIZE 10000

#define IFS_TYPE_STDIO	1
#define IFS_TYPE_VDF	2

class IfsBase : public Object
{
public:
	virtual uLong GetFileSize() const = 0;
	virtual uLong GetOffset() const = 0;
	virtual bool SetOffset(uLong offset) = 0;
	virtual uLong GetData(void* buffer, uLong size) = 0;
	virtual void Close() = 0;
};

class IFS : public IfsBase
{
protected:
	class IfsBuffer
	{
	public:
		uLong Begin;
		Byte Data[VDFS_BUFFER_SIZE];
		uLong Size;

		void Reset()
		{
			Begin = 0;
			Size = 0;
		}

		IfsBuffer()
		{
			Begin = 0;
			Size = 0;
		}

		~IfsBuffer()
		{}
	};

	TString Name;

	VdfsIndex::FileInfoPtr CurrentFileInfo;
	uLong CurrentOffset;
	IfsBuffer Buffer;

	virtual uLong Read(uLong offset, void* buffer, uLong size) = 0;

public:
	virtual const TCHAR* GetName() const;
	virtual int GetType() const = 0;

	virtual VdfsIndex::FileInfo* GetFileInfo(const AString& filename)
	{
		return nullptr;
	};

	virtual bool FileExists(const AString& filename)
	{
		return false;
	};

	virtual bool UpdateIndex(VdfsIndex* index) = 0;

	virtual IFS* Open(VdfsIndex::FileInfoPtr& fileinfo) = 0;
	uLong GetFileSize() const override = 0;
	uLong GetOffset() const override;
	bool SetOffset(uLong offset) override;
	uLong GetData(void* buffer, uLong size) override;
	void Close() override;

	virtual uInt GetStreamsSize()
	{
		return 0;
	};

	IFS()
	{
		CurrentOffset = 0;
	};

	~IFS() override
	{};
};

inline const TCHAR* IFS::GetName() const
{
	return Name;
}

inline uLong IFS::GetOffset() const
{
	return CurrentOffset;
}

inline bool IFS::SetOffset(const uLong offset)
{
	if (offset <= GetFileSize())
	{
		CurrentOffset = offset;
		return true;
	}
	return false;
}

inline void IFS::Close()
{
	CurrentFileInfo = nullptr;
	CurrentOffset = 0;
	Buffer.Reset();
}

inline uLong IFS::GetData(void* buffer, uLong size)
{
	if (!size)
		return 0;

	if (GetFileSize() <= CurrentOffset)
		return 0;

	size = MIN(size, GetFileSize() - CurrentOffset);

	if (size > VDFS_BUFFER_SIZE)
	{
		if (Read(CurrentOffset, buffer, size))
		{
			CurrentOffset += size;
			return size;
		}
		return 0;
	}

	if ((CurrentOffset < Buffer.Begin) || (CurrentOffset + size > Buffer.Begin + Buffer.Size))
	{
		Buffer.Reset();
		uLong SizeToRead = MIN(VDFS_BUFFER_SIZE, GetFileSize() - CurrentOffset);
		if (!Read(CurrentOffset, Buffer.Data, SizeToRead))
			return 0;
		Buffer.Begin = CurrentOffset;
		Buffer.Size = SizeToRead;
	}

	memcpy(buffer, &Buffer.Data[CurrentOffset - Buffer.Begin], size);
	CurrentOffset += size;
	return size;
}

using IfsPtr = AutoPtr<IFS>;

class IfsFilter : public IfsBase
{
public:
	virtual IfsFilter* Open(IFS* src) = 0;
};

using IfsFilterPtr = AutoPtr<IfsFilter>;

#endif
