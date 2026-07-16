#include "PreCompiled.h"

// GetFreeStream: find an unused filter stream or allocate a new one
OggFilter* OggFilter::GetFreeStream()
{
	for (uInt i = 0; i < Streams.Size(); i++)
	{
		if (!Streams[i]->Source)
			return Streams[i];
	}
	return Streams.Add(new OggFilter());
}

// Open: check if the source stream is OGG Vorbis and attach a filter if so
IfsFilter* OggFilter::Open(IFS* src)
{
	TString Ext(src->GetName());
	if (!Ext.TruncateBeforeLast(_T(".")) || (!Ext.Compare(_T("WAV")) && !Ext.Compare(_T("OGG"))))
		return nullptr;

	src->SetOffset(0);

	uChar Buffer[4];
	if ((src->GetData(Buffer, 4) == 4) && !memcmp(Buffer, "OggS", 4))
	{
		src->SetOffset(0);
		OggFilter* Res = GetFreeStream();
		if (Res->Attach(src))
			return Res;
	}

	src->SetOffset(0);
	return nullptr;
}

// Attach: initialize vorbis_file for an OGG source and populate the virtual WAV header
bool OggFilter::Attach(IFS* src)
{
	if (!ov_open_callbacks(src, &Vorbis, nullptr, 0, Callbacks) && (ov_streams(&Vorbis) == 1))
	{
		Source = src;

		vorbis_info* info = ov_info(&Vorbis, -1);

		Wave.Format.Channels = info->channels;
		Wave.Format.SamplesPerSec = info->rate;
		Wave.Format.BlockAlign = Wave.Format.Channels * Wave.Format.BitsPerSample / 8;
		Wave.Format.AvgBytesPerSec = Wave.Format.SamplesPerSec * Wave.Format.BlockAlign;
		Wave.dataChunk.ChunkSize = static_cast<uInt>(ov_pcm_total(&Vorbis, -1)) * Wave.Format.BlockAlign;
		Wave.Header.RiffSize = Wave.dataChunk.ChunkSize + sizeof(WaveFile) - 8;

		Offset = 0;
		return true;
	}
	Close();
	return false;
}

// GetFileSize: return the size of the virtual WAV header plus decoded audio data
uLong OggFilter::GetFileSize() const
{
	return Wave.Header.RiffSize + 8;
}

// GetOffset: return the current read offset in the virtual WAV stream
uLong OggFilter::GetOffset() const
{
	return Offset;
}

// SetOffset: set the read offset, rejecting values beyond the virtual WAV size
bool OggFilter::SetOffset(const uLong offset)
{
	if (offset > GetFileSize())
		return false;
	Offset = offset;
	return true;
}

// GetData: return WAV header bytes first, then decode OGG Vorbis audio data on demand
uLong OggFilter::GetData(void* buffer, const uLong size)
{
	auto Buffer = static_cast<char*>(buffer);
	uLong Size = size;
	if (Offset < sizeof(WaveFile))
	{
		size_t ToRead = sizeof(WaveFile) - Offset;
		if (size < ToRead)
			ToRead = size;

		memcpy(buffer, &Wave, ToRead);

		Buffer += ToRead;
		Size -= ToRead;
		Offset += ToRead;
	}

	if (Size)
	{
		uInt Part = (Offset - sizeof(WaveFile)) % Wave.Format.BlockAlign;
		if (Part)
		{
			Buffer = new char[size + Part];
			Offset -= Part;
			Size += Part;
		}

		if (ov_pcm_seek(&Vorbis, (Offset - sizeof(WaveFile)) / Wave.Format.BlockAlign))
		{
			if (Part)
				delete[] Buffer;
			return 0;
		}

		int default_stream = -1;

		uInt Readed = 0;
		while (Readed < Size)
		{
			int res = ov_read(&Vorbis, &Buffer[Readed], Size - Readed, 0, 2, 1, &default_stream);
			if (res <= 0)
			{
				if (Part)
				{
					memcpy(buffer, &Buffer[Part], Readed - Part);
					delete[] Buffer;
				}
				return (size - (Size - Part)) + Readed;
			}
			Readed += res;
			Offset += res;
		}

		if (Part)
		{
			memcpy(&static_cast<uChar*>(buffer)[size - Size], &Buffer[Part], size);
			delete[] Buffer;
		}
	}
	return size;
}

// Close: clear the vorbis file and close the source stream
void OggFilter::Close()
{
	if (Vorbis.datasource)
		ov_clear(&Vorbis);
	memset(&Vorbis, 0, sizeof(OggVorbis_File));

	if (Source)
	{
		Source->Close();
		Source = nullptr;
	}
}

// vorbis_read: ogg_callbacks read_func that delegates to the IFS stream
size_t vorbis_read(void* ptr, const size_t size, size_t nmemb, void* datasource)
{
	auto FS = static_cast<IFS*>(datasource);
	size_t ActualSize = size * nmemb;

	size_t MaxReadSize = static_cast<size_t>(FS->GetFileSize() - FS->GetOffset());
	if (ActualSize > MaxReadSize)
	{
		nmemb = MaxReadSize / size;
		ActualSize = size * nmemb;
	}

	if (FS->GetData(ptr, ActualSize) == ActualSize)
		return nmemb;

	return 0;
}

// vorbis_seek: ogg_callbacks seek_func that translates to IFS SetOffset
int vorbis_seek(void* datasource, const ogg_int64_t offset, const int whence)
{
	auto FS = static_cast<IFS*>(datasource);
	switch (whence)
	{
	case SEEK_SET:
		FS->SetOffset(static_cast<uLong>(offset));
		break;
	case SEEK_CUR:
		FS->SetOffset(static_cast<long>(FS->GetOffset()) + static_cast<long>(offset));
		break;
	case SEEK_END:
		FS->SetOffset(static_cast<long>(FS->GetFileSize()) + static_cast<long>(offset));
		break;
	}
	return static_cast<int>(FS->GetOffset());
}

// vorbis_close: ogg_callbacks close_func (no-op, source lifecycle managed by OggFilter)
int vorbis_close(void* datasource)
{
	return 0;
}

// vorbis_tell: ogg_callbacks tell_func that returns the current IFS offset
long vorbis_tell(void* datasource)
{
	auto FS = static_cast<IFS*>(datasource);
	return static_cast<long>(FS->GetOffset());
}

// OggFilter constructor: initialize vorbis callbacks and populate the default WAV header
OggFilter::OggFilter()
{
	Source = nullptr;
	memset(&Vorbis, 0, sizeof(OggVorbis_File));
	Callbacks.read_func = vorbis_read;
	Callbacks.close_func = vorbis_close;
	Callbacks.tell_func = vorbis_tell;
	Callbacks.seek_func = vorbis_seek;

	memset(&Wave, 0, sizeof(WaveFile));
	memcpy(&Wave.Header.RIFF_SIG, "RIFF", 4);
	memcpy(&Wave.Header.WAVE_SIG, "WAVE", 4);
	memcpy(&Wave.fmtChunk.CHUNK_NAME, "fmt ", 4);
	Wave.fmtChunk.ChunkSize = sizeof(WaveFormatEx);
	Wave.Format.FormatTag = 1;
	Wave.Format.BitsPerSample = 16;
	memcpy(&Wave.dataChunk.CHUNK_NAME, "data", 4);

	Offset = 0;
}
