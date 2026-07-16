#ifndef _OGG_FILTER_
#define _OGG_FILTER_

#pragma pack(push, 1)
// WaveFileHeader: RIFF/WAV file header (12 bytes)
struct WaveFileHeader
{
	uChar RIFF_SIG[4];
	uLong RiffSize;
	uChar WAVE_SIG[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
// RiffChunk: RIFF chunk header with 4-byte name and 4-byte size
struct RiffChunk
{
	uChar CHUNK_NAME[4];
	uLong ChunkSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
// WaveFormatEx: WAV format chunk describing audio sample format
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
// WaveFile: complete WAV file header structure (fmt + data chunks + RIFF header)
struct WaveFile
{
	WaveFileHeader Header;
	RiffChunk fmtChunk;
	WaveFormatEx Format;
	RiffChunk dataChunk;
};
#pragma pack(pop)


// OggFilter: transforms OGG Vorbis streams into virtual WAV streams on-the-fly
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
