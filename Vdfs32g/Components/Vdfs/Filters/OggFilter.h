#ifndef _OGG_FILTER_
#define _OGG_FILTER_

#pragma pack(push, 1)
struct WaveFileHeader
{
	uChar RIFF_SIG[4];
	uLong RiffSize;
	uChar WAVE_SIG[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct RiffChunk
{
	uChar CHUNK_NAME[4];
	uLong ChunkSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WaveFormatEx
{
	uShort FormatTag;
	uShort Channels;
	uInt SamplesPerSec;
	uInt AvgBytesPerSec;
	uShort BlockAlign;
	uShort BitsPerSample;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WaveFile
{
	WaveFileHeader Header;
	RiffChunk fmtChunk;
	WaveFormatEx Format;
	RiffChunk dataChunk;
};
#pragma pack(pop)


class OggFilter final : public IfsFilter
{
protected:
	ObjectArray<AutoPtr<OggFilter>> Streams;

	OggFilter* GetFreeStream();

public:
	IfsFilter* Open(IFS* src) override;

protected:
	IFS* Source;
	OggVorbis_File Vorbis;
	ov_callbacks Callbacks;
	WaveFile Wave;
	uLong Offset;

	bool Attach(IFS* src);

public:
	uLong GetFileSize() const override;
	uLong GetOffset() const override;
	bool SetOffset(uLong offset) override;
	uLong GetData(void* buffer, uLong size) override;
	void Close() override;

	OggFilter();

	~OggFilter() override
	{
		Close();
	}
};

#endif
