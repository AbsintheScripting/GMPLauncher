#ifndef _VDF_FLOW_
#define _VDF_FLOW_

#pragma pack(push,1)

// VdfTime: packed date/time stored in VDF archive entries (bitfield format)
struct VdfTime
{
	unsigned Sec   : 5; /* *2 */
	unsigned Min   : 6;
	unsigned Hour  : 5;
	unsigned Day   : 5;
	unsigned Month : 4;
	unsigned Year  : 7; /* 1980+ */
};

// VdfHeader: 320-byte header at the start of every VDF archive file
struct VdfHeader
{
	char Comment[256];
	char Signature[16];
	uInt NumEntries;
	uInt NumFiles;
	VdfTime Timestamp;
	uInt DataSize;
	uInt RootCatOffset;
	uInt Version; // ~Always = 0x50, may be a version
};

// VDF signature constants for Gothic 1 and Gothic 2 archive formats
#define VDF_SIGNATURE_G1 "PSVDSC_V2.00\r\n\r\n"
#define VDF_SIGNATURE_G2 "PSVDSC_V2.00\n\r\n\r"

// VdfEntryInfo type flags: VDF_ENTRY_DIR indicates a directory, VDF_ENTRY_LAST marks the last entry in a branch
#define VDF_ENTRY_DIR 0x80000000
#define VDF_ENTRY_LAST 0x40000000

// VdfEntryInfo: directory/file entry in a VDF root catalog (64-byte fixed size)
struct VdfEntryInfo
{
	char Name[64];
	uInt JumpTo; // Dirs = child entry's number, Files = data offset
	uInt Size;
	uInt Type; // = 0x00000000 for files or VDFS_ENTRY_DIR, may be bitmasked by VDFS_ENTRY_LAST
	uInt Attrib; // 20 = A;
};

#pragma pack(pop)

// VdfFlow: virtual file stream for reading from VDF/MOD archive files
class VdfFlow final : public IFS
{
	// Archive info
	HANDLE Archive;
	VdfHeader Header;
	int Version;

	ObjectArray<AutoPtr<VdfFlow>> Streams;

protected:
	uInt BuildVdfsIndex(VdfsIndex* index, VdfEntryInfo* Entries, uInt BeginNum = 0, const char* Directory = nullptr);
	VdfFlow* GetFreeStream();

public:
	const VdfTime& GetTimeStamp() const;

protected:
	uLong Read(uLong offset, void* buffer, uLong size) override;

public:
	int GetType() const override;

	bool UpdateIndex(VdfsIndex* index) override;

	IFS* Open(VdfsIndex::FileInfoPtr& fileinfo) override;
	uLong GetFileSize() const override;

	uInt GetStreamsSize() override
	{
		return Streams.Size();
	};

	bool Init(const TCHAR* arcname);
	bool Init(const TCHAR* arcname, VdfsIndex::FileInfoPtr& fileinfo);
	VdfFlow();
	~VdfFlow() override;
};

inline const VdfTime& VdfFlow::GetTimeStamp() const
{
	return Header.Timestamp;
}

inline int VdfFlow::GetType() const
{
	return IFS_TYPE_VDF;
}

inline uLong VdfFlow::GetFileSize() const
{
	return (CurrentFileInfo) ? CurrentFileInfo->Size : 0;
}

using VdfFlowPtr = AutoPtr<VdfFlow>;

#endif
