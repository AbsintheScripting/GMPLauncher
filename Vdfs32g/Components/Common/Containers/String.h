#ifndef _COMMON_STRING_
#define _COMMON_STRING_

namespace COMMON
{
	class AString final : public Object
	{
	protected:
		char* Data;
		uInt Size;
		bool CompareIgnoreCase;

		uInt CalcLength(const char* Data, const uInt MaxCount = 0) const
		{
			if (!Data)
				return 0;

			uInt i = 0;
			for (; !MaxCount || (i < MaxCount); i++)
			{
				if (Data[i] == '\0')
					break;
			}
			return i;
		}

		uInt CalcLength(const wchar_t* Data, const uInt MaxCount = 0) const
		{
			if (!Data)
				return 0;

			uInt i = 0;
			for (; !MaxCount || (i < MaxCount); i++)
			{
				if (Data[i] == L'\0')
					break;
			}
			return i;
		}

		void Realloc(const char* data, const uInt size)
		{
			auto NewData = new char[size + 1];
			memcpy(NewData, data, size * sizeof(char));
			NewData[size] = '\0';

			if (Data)
				delete[] Data;

			Data = NewData;
			Size = size;
		}

		void Realloc(const wchar_t* data, const uInt size)
		{
			auto NewData = new char[size + 1];
			size_t ReturnValue;
			wcstombs_s(&ReturnValue, NewData, size + 1, data, size);
			NewData[size] = '\0';

			if (Data)
				delete[] Data;

			Data = NewData;
			Size = size;
		}

	public:
		// Use this only for data preallocation, carefully
		void SetSizeDirectly(const uInt NewSize)
		{
			Size = NewSize;
			Data[Size] = '\0';
		}

		void Resize(const uInt NewSize)
		{
			if (!NewSize)
			{
				Clear();
				return;
			}

			if (NewSize != Size)
			{
				char* OldData = Data;
				uInt OldSize = Size;

				Data = new char[NewSize + 1];
				Data[NewSize] = '\0';
				Size = NewSize;

				if (OldData)
				{
					memcpy(Data, OldData, ((NewSize < OldSize) ? NewSize : OldSize) * sizeof(char));
					delete[] OldData;
				}
			}
		}

		AString& Assign(const char* data)
		{
			if (data)
			{
				if (Data == data)
					Resize(CalcLength(data));
				else
				{
					uInt NewSize = CalcLength(data);
					Realloc(data, NewSize);
				}
			}
			else
				Clear();

			return *this;
		}

		AString& Assign(const wchar_t* data)
		{
			if (data)
			{
				uInt NewSize = CalcLength(data);
				Realloc(data, NewSize);
			}
			else
				Clear();

			return *this;
		}

		AString& Assign(const char data)
		{
			if (data)
				Realloc(&data, 1);
			else
				Clear();

			return *this;
		}

		AString& Assign(const wchar_t data)
		{
			if (data)
			{
				char Result = '\0';
				int sResult = 0;
				wctomb_s(&sResult, &Result, 1, data);
				Realloc(&Result, 1);
			}
			else
				Clear();

			return *this;
		}

		AString& Assign(const char* data, const uInt length)
		{
			if (data && length)
			{
				if (Data == data)
				{
					uInt OrgLen = CalcLength(data, length);
					Resize((OrgLen < length) ? OrgLen : length);
				}
				else
				{
					uInt OrgLen = CalcLength(data, length);
					Realloc(data, (OrgLen < length) ? OrgLen : length);
				}
			}
			else
				Clear();

			return *this;
		}

		AString& Assign(const wchar_t* data, const uInt length)
		{
			if (data && length)
			{
				uInt OrgLen = CalcLength(data, length);
				Realloc(data, (OrgLen < length) ? OrgLen : length);
			}
			else
				Clear();

			return *this;
		}

		AString& Assign(const AString& data)
		{
			if (data.Size)
				Realloc(data.Data, data.Size);
			else
				Clear();

			CompareIgnoreCase = data.CompareIgnoreCase;

			return *this;
		}

		AString& Assign(const AString& data, const uInt length)
		{
			if (data.Size && length)
			{
				if (Data == data.Data)
					Resize((data.Size < length) ? data.Size : length);
				else
					Realloc(data, (data.Size < length) ? data.Size : length);
			}
			else
				Clear();

			CompareIgnoreCase = data.CompareIgnoreCase;

			return *this;
		}

		AString& Assign(const int value, const int radix = 10)
		{
			char Buffer[16];
			if (!_itoa_s(value, Buffer, 16, radix))
				Assign(Buffer);

			return *this;
		}

		AString& Assign(const uInt value, const int radix = 10)
		{
			char Buffer[16];
			if (!_ui64toa_s(value, Buffer, 16, radix))
				Assign(Buffer);

			return *this;
		}

		AString& Assign(const float value)
		{
			char Buffer[48];
			if (sprintf_s(Buffer, 48, "%f", value) > 0)
				Assign(Buffer);

			return *this;
		}

		AString& va_format(const char* format, void* args)
		{
			int Len = _vscprintf(format, static_cast<va_list>(args));
			if (Len > 0)
			{
				auto Buffer = new char[Len + 1];

				Len = vsprintf_s(Buffer, Len + 1, format, static_cast<va_list>(args));
				if (Len > 0)
					Assign(Buffer, static_cast<uInt>(Len));

				delete[] Buffer;
			}

			return *this;
		}

		AString& Format(const char* format, ...)
		{
			va_list args;
			va_start(args, format);

			return va_format(format, args);
		}

		AString& operator =(const char* data)
		{
			return Assign(data);
		}

		AString& operator =(const wchar_t* data)
		{
			return Assign(data);
		}

		AString& operator =(const char data)
		{
			return Assign(data);
		}

		AString& operator =(const wchar_t data)
		{
			return Assign(data);
		}

		AString& operator =(const AString& data)
		{
			return Assign(data);
		}

		AString& operator =(const int value)
		{
			return Assign(value);
		}

		AString& operator =(const uInt value)
		{
			return Assign(value);
		}

		AString& operator =(const float value)
		{
			return Assign(value);
		}

		// Add data
		AString& Append(const char* data)
		{
			if (data)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					memcpy(&Data[OldSize], data, AddSize * sizeof(char));
					Data[Size] = '\0';
				}
			}
			return *this;
		}

		AString& Append(const char* data, const uInt size)
		{
			if (data && size)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data, size);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					memcpy(&Data[OldSize], data, AddSize * sizeof(char));
					Data[Size] = '\0';
				}
			}
			return *this;
		}

		AString& Append(const wchar_t* data)
		{
			if (data)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					size_t ReturnValue;
					wcstombs_s(&ReturnValue, &Data[OldSize], AddSize + 1, data, AddSize);
					Data[Size] = '\0';
				}
			}
			return *this;
		}

		AString& Append(const wchar_t* data, const uInt size)
		{
			if (data && size)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data, size);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					size_t ReturnValue;
					wcstombs_s(&ReturnValue, &Data[OldSize], AddSize + 1, data, AddSize);
					Data[Size] = '\0';
				}
			}
			return *this;
		}

		AString& Append(const char data)
		{
			if (data)
			{
				Resize(Size + 1);
				Data[Size - 1] = data;
				Data[Size] = '\0';
			}
			return *this;
		}

		AString& Append(const wchar_t data)
		{
			if (data)
			{
				Resize(Size + 1);
				int Result = 0;
				wctomb_s(&Result, &Data[Size - 1], 1, data);
				Data[Size] = '\0';
			}
			return *this;
		}

		AString& Append(const AString& data)
		{
			if (data.Size)
			{
				uInt OldSize = Size;
				Resize(Size + data.Size);
				memcpy(&Data[OldSize], data.Data, data.Size * sizeof(char));
				Data[Size] = '\0';
			}
			return *this;
		}

		AString& Append(const int value, const int radix = 10)
		{
			return Append(AString(value, radix));
		}

		AString& Append(const uInt value, const int radix = 10)
		{
			return Append(AString(value, radix));
		}

		AString& Append(const float value)
		{
			return Append(AString(value));
		}

		AString& operator +=(const char* data)
		{
			return Append(data);
		}

		AString& operator +=(const wchar_t* data)
		{
			return Append(data);
		}

		AString& operator +=(const char data)
		{
			return Append(data);
		}

		AString& operator +=(const wchar_t data)
		{
			return Append(data);
		}

		AString& operator +=(const AString& data)
		{
			return Append(data);
		}

		AString& operator +=(const int value)
		{
			return Append(value);
		}

		AString& operator +=(const uInt value)
		{
			return Append(value);
		}

		AString& operator +=(const float value)
		{
			return Append(value);
		}

		AString operator +(const char* data) const
		{
			AString Result(*this);
			return Result.Append(data);
		}

		AString operator +(const wchar_t* data) const
		{
			AString Result(*this);
			return Result.Append(data);
		}

		AString operator +(const char data) const
		{
			AString Result(*this);
			return Result.Append(data);
		}

		AString operator +(const wchar_t data) const
		{
			AString Result(*this);
			return Result.Append(data);
		}

		AString operator +(const AString& data) const
		{
			AString Result(*this);
			return Result.Append(data);
		}

		AString operator +(const int value) const
		{
			AString Result(*this);
			return Result.Append(value);
		}

		AString operator +(const uInt value) const
		{
			AString Result(*this);
			return Result.Append(value);
		}

		AString operator +(const float value) const
		{
			AString Result(*this);
			return Result.Append(value);
		}

		// Clear data
		void Clear()
		{
			if (Data)
			{
				delete[] Data;
				Data = nullptr;
				Size = 0;
			}
		}

		// Compare data
		bool Compare(const char* data, const bool IgnoreCase = false, uInt Count = 0) const
		{
			if (!data)
				return !Data;

			if (!Count ? (CalcLength(data) == Size) : ((Size >= Count) && (CalcLength(data) >= Count)))
			{
				if (!Count)
					Count = Size;

				if (IgnoreCase)
					return (_strnicmp(Data, data, Count) == 0);
				return (memcmp(Data, data, Count * sizeof(char)) == 0);
			}
			return false;
		}

		bool MatchAsMask(const char* text, const bool IgnoreCase = false) const
		{
			if (!Data)
				return false;

			const char* Start = Data;

			const char* TokenStart = nullptr;
			size_t TokenLength = 0;
			const char* PrevTokenStart = nullptr;
			size_t PrevTokenLength = 0;

			while (*Start && text)
			{
				const char* End = nullptr;
				for (End = Start; *End; End++)
				{
					if (strchr("*?", *End))
						break;
				}
				TokenStart = Start;
				TokenLength = (End ? ((End != Start) ? End - Start : 1) : strlen(Start));

				switch (TokenStart[0])
				{
				case '*':	// AnyRepeat
					break;
				case '?':	// Any
					text++;
					break;
				default:	// Template
					{
						if (PrevTokenStart && (PrevTokenStart[0] == '*'))
						{
							if (IgnoreCase)
							{
								if (text = _strnistr(text, TokenStart, TokenLength))
									text += TokenLength;
							}
							else
							{
								if (text = _strnstr(text, TokenStart, TokenLength))
									text += TokenLength;
							}
						}
						else
						{
							if (IgnoreCase)
							{
								if (!_strnicmp(text, TokenStart, TokenLength))
									text += TokenLength;
								else
									text = nullptr;
							}
							else
							{
								if (!strncmp(text, TokenStart, TokenLength))
									text += TokenLength;
								else
									text = nullptr;
							}
						}
					}
					break;
				}

				Start += TokenLength;

				PrevTokenStart = TokenStart;
				PrevTokenLength = TokenLength;
			}

			return ((text != nullptr) && (!*text));
		}

		bool operator ==(const char* data) const
		{
			return Compare(data, CompareIgnoreCase);
		}

		bool operator !=(const char* data) const
		{
			return (!(*this == data));
		}

		bool Compare(const AString& data, const bool IgnoreCase = false, uInt Count = 0) const
		{
			if (!data.Data)
				return !Data;

			if (!Count ? (data.Size == Size) : ((Size >= Count) && (data.Size >= Count)))
			{
				if (!Count)
					Count = Size;

				if (IgnoreCase)
					return (_strnicmp(Data, data.Data, Count) == 0);
				return (memcmp(Data, data.Data, Count * sizeof(char)) == 0);
			}
			return false;
		}

		bool operator ==(const AString& data) const
		{
			return Compare(data, CompareIgnoreCase || data.CompareIgnoreCase);
		}

		bool operator !=(const AString& data) const
		{
			return (!(data == *this));
		}

		bool operator >(const AString& data) const
		{
			if (Size > data.Size)
				return true;
			if (Size < data.Size)
				return false;
			return (memcmp(Data, data.Data, ((Size < data.Size) ? Size : data.Size) * sizeof(char)) > 0);
		}

		operator bool() const
		{
			return (Size > 0);
		}

		operator const char*() const
		{
			return Data;
		}

		char operator [](const int index) const
		{
			if ((index >= 0) && (index < static_cast<int>(Size)))
				return Data[index];
			return '\0';
		}

		char GetFirst(const int index = 0) const
		{
			if ((index >= 0) && (index < static_cast<int>(Size)))
				return Data[index];
			return '\0';
		}

		char GetLast(const int index = 0) const
		{
			if ((index >= 0) && (index < static_cast<int>(Size)))
				return Data[Size - 1 - index];
			return '\0';
		}

		uInt Length() const
		{
			return Size;
		}

		const char* GetData(const char* sub = nullptr, const uInt index = 0, const uInt offset = 0) const
		{
			if (sub && Data)
			{
				const char* pos = strstr(Data, sub);
				for (uInt i = 0; pos && (i < index); i++)
					pos = strstr(pos + 1, sub);
				return pos ? &pos[offset] : nullptr;
			}
			return Data;
		}

		bool GetCompareIgnoreCase() const
		{
			return CompareIgnoreCase;
		}

		AString& SetCompareIgnoreCase(const bool IgnoreCase)
		{
			CompareIgnoreCase = IgnoreCase;
			return *this;
		}

		const uInt GetSubCount(const char* sub, Array<const char*>* Subs = nullptr) const
		{
			uInt Result = 0;
			if (sub)
			{
				uInt SubLen = CalcLength(sub);
				for (char* Pos = Data; Pos;)
				{
					if (Subs)
						Subs->Add() = Pos;

					if (Pos = strstr(Pos, sub))
					{
						Pos += SubLen;
						Result++;
					}
				}
			}
			return Result;
		}

		const char* FirstCharNotInSet(const char* set) const
		{
			if (Data)
			{
				uInt Pos = strspn(Data, set);
				if (Pos != CalcLength(Data))
					return Data + Pos;
			}
			return nullptr;
		}

		const char* FirstCharInSet(const char* set) const
		{
			if (Data)
			{
				for (uInt i = 0; i < Size; i++)
				{
					if (strchr(set, Data[i]))
						return &Data[i];
				}
			}
			return nullptr;
		}

		AString& ToLowerCase()
		{
			if (Data)
				_strlwr_s(Data, Size + 1);
			return *this;
		}

		AString& ToUpperCase()
		{
			if (Data)
				_strupr_s(Data, Size + 1);
			return *this;
		}

		AString* GetLowerCase() const
		{
			auto Result = new AString(*this);
			Result->ToLowerCase();
			return Result;
		}

		AString* GetUpperCase() const
		{
			auto Result = new AString(*this);
			Result->ToUpperCase();
			return Result;
		}

		AString& ToEscapeChars()
		{
			if (Data && Size && strpbrk(Data, "\a\b\f\n\r\t\v\\\'\"\?"))
			{
				uInt NewSize = 0;
				auto NewData = new char[Size * 2];
				for (uInt c = 0; c < Size; c++)
				{
					switch (Data[c])
					{
					case '\a':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 'a';
						break;
					case '\b':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 'b';
						break;
					case '\f':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 'f';
						break;
					case '\n':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 'n';
						break;
					case '\r':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 'r';
						break;
					case '\t':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 't';
						break;
					case '\v':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = 'v';
						break;
					case '\\':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = '\\';
						break;
					case '\'':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = '\'';
						break;
					case '\"':
						NewData[NewSize++] = '\\';
						NewData[NewSize++] = '\"';
						break;
					default:
						NewData[NewSize++] = Data[c];
						break;
					}
				}
				delete[] Data;
				Data = NewData;
				Size = NewSize;
			}
			return *this;
		}

		AString& ToValuableFloat(const int afterdot = -1)
		{
			if (Data)
			{
				const char* Dot = GetData(".");
				if (Dot)
				{
					if (afterdot >= 0)
					{
						size_t num = Size - (Dot - Data);
						if (num > 1)
						{
							num--;
							if (num > static_cast<size_t>(afterdot))
							{
								Size = Size - (num - afterdot);
								if (!afterdot && Size)
									Size--;

								if (Size)
									Data[Size] = '\0';
								else
								{
									Assign("0");
									return *this;
								}
							}
						}
					}

					uInt i = Size;
					for (; i > 0; i--)
					{
						if (Data[i - 1] != '0')
							break;
					}
					if (i && Data[i - 1] == '.')
						i--;

					if (i)
					{
						Data[i] = '\0';
						Size = i;
					}
					else
						Assign("0");
				}
			}
			return *this;
		}

		AString* GetValuableFloat(const int afterdot = -1) const
		{
			auto Result = new AString(*this);
			Result->ToValuableFloat(afterdot);
			return Result;
		}

		AString* GetToken(const uInt index, const char* delim, AString* result = nullptr) const
		{
			AString* Result = nullptr;
			if (delim && Data && Size)
			{
				uInt DelimLen = CalcLength(delim);

				char* Pos = Data;
				for (uInt i = 0; Pos; i++)
				{
					char* OldPos = Pos;
					if (Pos = strstr(OldPos, delim))
						Pos += DelimLen;

					if (i == index)
					{
						if (Pos)
						{
							if (!result)
								Result = new AString(OldPos, static_cast<uInt>(Pos - OldPos) - DelimLen);
							else
								(Result = result)->Assign(OldPos, static_cast<uInt>(Pos - OldPos) - DelimLen);
						}
						else
						{
							if (!result)
								Result = new AString(OldPos);
							else
								(Result = result)->Assign(OldPos);
						}
						break;
					}
				}
			}
			return Result;
		}

		ObjectArray<AString>* GetAllTokens(const char* delim, const bool onlyifcontain = false) const
		{
			ObjectArray<AString>* Result = nullptr;
			if (delim && Data && Size && (!onlyifcontain || GetData(delim)))
			{
				uInt DelimLen = CalcLength(delim);

				Array<const char*> Subs;
				Subs.SetGranularity(256);
				GetSubCount(delim, &Subs);

				Result = new ObjectArray<AString>(Subs.Size());
				for (uInt i = 0; i < Subs.Size(); i++)
				{
					if (i == Subs.Size() - 1)
					{
						if (*Subs[i] != '\0')
							Result->Add() = AString(Subs[i]);
					}
					else
						Result->Add() = AString(Subs[i], Subs[i + 1] - Subs[i] - DelimLen);
				}
			}
			return Result;
		}

		AString* GetLastToken(const char* delim, AString* result = nullptr) const
		{
			AString* Result = nullptr;
			if (delim && Data && Size)
			{
				if (result)
					Result = result;
				else
					Result = new AString;

				uInt DelimLen = CalcLength(delim);

				auto Pos = (const char*)_memrstr((uChar*)Data,
				                                 Size * sizeof(char),
				                                 (uChar*)delim,
				                                 DelimLen * sizeof(char));
				if (Pos)
					Result->Assign(&Pos[DelimLen], static_cast<uInt>(&Data[Size] - &Pos[DelimLen]));
				else
					Result->Assign(Data, Size);
			}
			return Result;
		}

		uInt TruncateAfterFirst(const char* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				char* Pos = strstr(Data, sub);
				for (uInt i = 0; Pos && (i < index) && (Pos = strstr(Pos + 1, sub)); i++);

				if (Pos)
				{
					uInt NewSize = static_cast<uInt>(Pos - Data);
					Delta = Size - NewSize;
					Assign(Data, NewSize);
				}
			}
			return Delta;
		}

		uInt TruncateBeforeFirst(const char* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				uInt SubLen = CalcLength(sub);

				char* Pos = strstr(Data, sub);
				for (uInt i = 0; Pos && (i < index) && (Pos = strstr(Pos + 1, sub)); i++);

				if (Pos)
				{
					Delta = static_cast<uInt>(Pos + SubLen - Data);
					Assign(Pos + SubLen);
				}
			}
			return Delta;
		}

		uInt TruncateAfterLast(const char* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				uInt SubLen = CalcLength(sub);

				const char* Pos = _strrnstr(Data, sub, Size, SubLen);
				for (uInt i = 0; i < index; i++)
				{
					if ((Pos <= Data) || !(Pos = _strrnstr(Data, sub, Pos - Data, SubLen)))
						return 0;
				}

				if (Pos)
				{
					Delta = static_cast<uInt>(Data + Size - Pos);
					Assign(Data, static_cast<uInt>(Pos - Data));
				}
			}
			return Delta;
		}

		uInt TruncateBeforeLast(const char* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				uInt SubLen = CalcLength(sub);

				const char* Pos = _strrnstr(Data, sub, Size, SubLen);
				for (uInt i = 0; i < index; i++)
				{
					if ((Pos <= Data) || !(Pos = _strrnstr(Data, sub, Pos - Data, SubLen)))
						return 0;
				}

				if (Pos)
				{
					Delta = static_cast<uInt>(Pos - Data) + SubLen;
					Assign(Pos + SubLen);
				}
			}
			return Delta;
		}

		uInt TruncateByLength(const uInt NewLength)
		{
			if (!NewLength)
			{
				uInt OldSize = Size;
				Clear();
				return OldSize;
			}

			if (NewLength >= Size)
				return 0;

			uInt Delta = Size - NewLength;
			Resize(NewLength);

			return Delta;
		}

		uInt Replace(const char* From, const char* To)
		{
			int n = 0;
			if (Data)
			{
				uInt FromLen = CalcLength(From);
				uInt ToLen = CalcLength(To);

				AString Temp;
				for (char* Pos = Data; Pos = strstr(Pos, From);)
				{
					n++;

					Temp.Assign(Pos[FromLen]);

					*Pos = '\0';
					uInt LastSize = (Size = static_cast<uInt>(Pos - Data));

					Append(To, ToLen);
					Append(Temp);

					Pos = &Data[LastSize];
				}
			}
			return n;
		}

		AString& GetReplaced(const char* From, const char* To)
		{
			Replace(From, To);
			return *this;
		}

		uInt Replace(const char Character, const char Replace)
		{
			int n = 0;
			if (Data)
			{
				for (uInt i = 0; i < Size; i++)
				{
					if (Data[i] == Character)
					{
						Data[i] = Replace;
						n++;
					}
				}
			}
			return n;
		}

		AString& GetReplaced(const char Character, const char cReplace)
		{
			Replace(Character, cReplace);
			return *this;
		}

		uInt CleanUp(const char Character)
		{
			uInt Delta = 0;
			if (Data)
			{
				uInt n = 0;
				for (uInt i = 0; i < Size; i++)
				{
					if (Data[i] != Character)
						Data[n++] = Data[i];
				}
				Delta = Size - n;
				Resize(n);
			}
			return Delta;
		}

		int ToInt() const
		{
			if (Data)
				return atoi(Data);
			return 0;
		}

		bool IsInt() const
		{
			return Size && (FirstCharNotInSet("-0123456789") == nullptr);
		}

		uInt ToUInt() const
		{
			uInt Result = 0;
			if (Data)
				sscanf_s(Data, GetData("x") ? "%x" : "%u", &Result);
			return Result;
		}

		bool IsUInt() const
		{
			return Size && (FirstCharNotInSet("x0123456789ABCDEFabcdef") == nullptr);
		}

		float ToFloat() const
		{
			if (Data)
				return static_cast<float>(atof(Data));
			return 0.0f;
		}

		bool IsFloat() const
		{
			return Size && (FirstCharNotInSet("-0123456789.") == nullptr);
		}

		bool ToBool() const
		{
			if (Data)
				return Compare("true", true);
			return false;
		}

		bool IsBool() const
		{
			return Size && (Compare("true", true) || Compare("false", true));
		}

		AString()
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
		}

		AString(const char* data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		AString(const wchar_t* data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		AString(const char data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		AString(const wchar_t data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		AString(const char* data, const uInt length)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data, length);
		}

		AString(const wchar_t* data, const uInt length)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data, length);
		}

		AString(const AString& data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		AString(const int value, const int radix = 10)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(value, radix);
		}

		AString(const uInt value, const int radix = 10)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(value, radix);
		}

		AString(const float value)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(value);
		}

		~AString() override
		{
			Clear();
		}
	};

	class WString final : public Object
	{
	protected:
		wchar_t* Data;
		uInt Size;
		bool CompareIgnoreCase;

		uInt CalcLength(const char* Data, const uInt MaxCount = 0) const
		{
			if (!Data)
				return 0;

			uInt i = 0;
			for (; !MaxCount || (i < MaxCount); i++)
			{
				if (Data[i] == '\0')
					break;
			}
			return i;
		}

		uInt CalcLength(const wchar_t* Data, const uInt MaxCount = 0) const
		{
			if (!Data)
				return 0;

			uInt i = 0;
			for (; !MaxCount || (i < MaxCount); i++)
			{
				if (Data[i] == L'\0')
					break;
			}
			return i;
		}

		void Realloc(const char* data, const uInt size)
		{
			auto NewData = new wchar_t[size + 1];
			size_t ReturnValue;
			mbstowcs_s(&ReturnValue, NewData, size + 1, data, size);
			NewData[size] = L'\0';

			if (Data)
				delete[] Data;

			Data = NewData;
			Size = size;
		}

		void Realloc(const wchar_t* data, const uInt size)
		{
			auto NewData = new wchar_t[size + 1];
			memcpy(NewData, data, size * sizeof(wchar_t));
			NewData[size] = L'\0';

			if (Data)
				delete[] Data;

			Data = NewData;
			Size = size;
		}

	public:
		// Use this only for data preallocation, carefully
		void SetSizeDirectly(const uInt NewSize)
		{
			Size = NewSize;
			Data[Size] = L'\0';
		}

		void Resize(const uInt NewSize)
		{
			if (!NewSize)
			{
				Clear();
				return;
			}

			if (NewSize != Size)
			{
				wchar_t* OldData = Data;
				uInt OldSize = Size;

				Data = new wchar_t[NewSize + 1];
				Data[NewSize] = L'\0';
				Size = NewSize;

				if (OldData)
				{
					memcpy(Data, OldData, ((NewSize < OldSize) ? NewSize : OldSize) * sizeof(wchar_t));
					delete[] OldData;
				}
			}
		}

		WString& Assign(const wchar_t* data)
		{
			if (data)
			{
				if (Data == data)
					Resize(CalcLength(data));
				else
				{
					uInt NewSize = CalcLength(data);
					Realloc(data, NewSize);
				}
			}
			else
				Clear();

			return *this;
		}

		WString& Assign(const char* data)
		{
			if (data)
			{
				uInt NewSize = CalcLength(data);
				Realloc(data, NewSize);
			}
			else
				Clear();

			return *this;
		}

		WString& Assign(const char data)
		{
			if (data)
			{
				wchar_t Result = L'\0';
				mbtowc(&Result, &data, 1);
				Realloc(&Result, 1);
			}
			else
				Clear();

			return *this;
		}

		WString& Assign(const wchar_t data)
		{
			if (data)
				Realloc(&data, 1);
			else
				Clear();

			return *this;
		}

		WString& Assign(const wchar_t* data, const uInt length)
		{
			if (data && length)
			{
				if (Data == data)
				{
					uInt OrgLen = CalcLength(data, length);
					Resize((OrgLen < length) ? OrgLen : length);
				}
				else
				{
					uInt OrgLen = CalcLength(data, length);
					Realloc(data, (OrgLen < length) ? OrgLen : length);
				}
			}
			else
				Clear();

			return *this;
		}

		WString& Assign(const char* data, const uInt length)
		{
			if (data && length)
			{
				uInt OrgLen = CalcLength(data, length);
				Realloc(data, (OrgLen < length) ? OrgLen : length);
			}
			else
				Clear();

			return *this;
		}

		WString& Assign(const WString& data)
		{
			if (data.Size)
				Realloc(data.Data, data.Size);
			else
				Clear();

			CompareIgnoreCase = data.CompareIgnoreCase;

			return *this;
		}

		WString& Assign(const WString& data, const uInt length)
		{
			if (data.Size && length)
			{
				if (Data == data.Data)
					Resize((data.Size < length) ? data.Size : length);
				else
					Realloc(data, (data.Size < length) ? data.Size : length);
			}
			else
				Clear();

			CompareIgnoreCase = data.CompareIgnoreCase;

			return *this;
		}

		WString& Assign(const int value, const int radix = 10)
		{
			wchar_t Buffer[16];
			if (!_itow_s(value, Buffer, 16, radix))
				Assign(Buffer);

			return *this;
		}

		WString& Assign(const uInt value, const int radix = 10)
		{
			wchar_t Buffer[16];
			if (!_ui64tow_s(value, Buffer, 16, radix))
				Assign(Buffer);

			return *this;
		}

		WString& Assign(const float value)
		{
			wchar_t Buffer[48];
			if (swprintf_s(Buffer, 48, L"%f", value) > 0)
				Assign(Buffer);

			return *this;
		}

		WString& va_format(const wchar_t* format, void* args)
		{
			int Len = _vscwprintf(format, static_cast<va_list>(args));
			if (Len > 0)
			{
				auto Buffer = new wchar_t[Len + 1];

				Len = vswprintf_s(Buffer, Len + 1, format, static_cast<va_list>(args));
				if (Len > 0)
					Assign(Buffer, static_cast<uInt>(Len));

				delete[] Buffer;
			}

			return *this;
		}

		WString& Format(const wchar_t* format, ...)
		{
			va_list args;
			va_start(args, format);

			return va_format(format, args);
		}

		WString& operator =(const wchar_t* data)
		{
			return Assign(data);
		}

		WString& operator =(const char* data)
		{
			return Assign(data);
		}

		WString& operator =(const wchar_t data)
		{
			return Assign(data);
		}

		WString& operator =(const char data)
		{
			return Assign(data);
		}

		WString& operator =(const WString& data)
		{
			return Assign(data);
		}

		WString& operator =(const int value)
		{
			return Assign(value);
		}

		WString& operator =(const uInt value)
		{
			return Assign(value);
		}

		WString& operator =(const float value)
		{
			return Assign(value);
		}

		// Add data
		WString& Append(const wchar_t* data)
		{
			if (data)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					memcpy(&Data[OldSize], data, AddSize * sizeof(wchar_t));
					Data[Size] = L'\0';
				}
			}
			return *this;
		}

		WString& Append(const wchar_t* data, const uInt size)
		{
			if (data && size)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data, size);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					memcpy(&Data[OldSize], data, AddSize * sizeof(wchar_t));
					Data[Size] = L'\0';
				}
			}
			return *this;
		}

		WString& Append(const char* data)
		{
			if (data)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					size_t ReturnValue;
					mbstowcs_s(&ReturnValue, &Data[OldSize], AddSize + 1, data, AddSize);
					Data[Size] = L'\0';
				}
			}
			return *this;
		}

		WString& Append(const char* data, const uInt size)
		{
			if (data && size)
			{
				uInt OldSize = Size;
				uInt AddSize = CalcLength(data, size);
				if (AddSize)
				{
					Resize(OldSize + AddSize);
					size_t ReturnValue;
					mbstowcs_s(&ReturnValue, &Data[OldSize], AddSize + 1, data, AddSize);
					Data[Size] = L'\0';
				}
			}
			return *this;
		}

		WString& Append(const char data)
		{
			if (data)
			{
				Resize(Size + 1);
				mbtowc(&Data[Size - 1], &data, sizeof(char));
				Data[Size] = L'\0';
			}
			return *this;
		}

		WString& Append(const wchar_t data)
		{
			if (data)
			{
				Resize(Size + 1);
				Data[Size - 1] = data;
				Data[Size] = L'\0';
			}
			return *this;
		}

		WString& Append(const WString& data)
		{
			if (data.Size)
			{
				uInt OldSize = Size;
				Resize(Size + data.Size);
				memcpy(&Data[OldSize], data.Data, data.Size * sizeof(wchar_t));
				Data[Size] = L'\0';
			}
			return *this;
		}

		WString& Append(const int value, const int radix = 10)
		{
			return Append(WString(value, radix));
		}

		WString& Append(const uInt value, const int radix = 10)
		{
			return Append(WString(value, radix));
		}

		WString& Append(const float value)
		{
			return Append(WString(value));
		}

		WString& operator +=(const wchar_t* data)
		{
			return Append(data);
		}

		WString& operator +=(const char* data)
		{
			return Append(data);
		}

		WString& operator +=(const wchar_t data)
		{
			return Append(data);
		}

		WString& operator +=(const char data)
		{
			return Append(data);
		}

		WString& operator +=(const WString& data)
		{
			return Append(data);
		}

		WString& operator +=(const int value)
		{
			return Append(value);
		}

		WString& operator +=(const uInt value)
		{
			return Append(value);
		}

		WString& operator +=(const float value)
		{
			return Append(value);
		}

		WString operator +(const wchar_t* data) const
		{
			WString Result(*this);
			return Result.Append(data);
		}

		WString operator +(const char* data) const
		{
			WString Result(*this);
			return Result.Append(data);
		}

		WString operator +(const wchar_t data) const
		{
			WString Result(*this);
			return Result.Append(data);
		}

		WString operator +(const char data) const
		{
			WString Result(*this);
			return Result.Append(data);
		}

		WString operator +(const WString& data) const
		{
			WString Result(*this);
			return Result.Append(data);
		}

		WString operator +(const int value) const
		{
			WString Result(*this);
			return Result.Append(value);
		}

		WString operator +(const uInt value) const
		{
			WString Result(*this);
			return Result.Append(value);
		}

		WString operator +(const float value) const
		{
			WString Result(*this);
			return Result.Append(value);
		}

		// Clear data
		void Clear()
		{
			if (Data)
			{
				delete[] Data;
				Data = nullptr;
				Size = 0;
			}
		}

		// Compare data
		bool Compare(const wchar_t* data, const bool IgnoreCase = false, uInt Count = 0) const
		{
			if (!data)
				return !Data;

			if (!Count ? (CalcLength(data) == Size) : ((Size >= Count) && (CalcLength(data) >= Count)))
			{
				if (!Count)
					Count = Size;

				if (IgnoreCase)
					return (_wcsnicmp(Data, data, Count) == 0);
				return (memcmp(Data, data, Count * sizeof(wchar_t)) == 0);
			}
			return false;
		}

		bool MatchAsMask(const wchar_t* text, const bool IgnoreCase = false) const
		{
			if (!Data)
				return false;

			const wchar_t* Start = Data;

			const wchar_t* TokenStart = nullptr;
			size_t TokenLength = 0;
			const wchar_t* PrevTokenStart = nullptr;
			size_t PrevTokenLength = 0;

			while (*Start && text)
			{
				const wchar_t* End = nullptr;
				for (End = Start; *End; End++)
				{
					if (wcschr(L"*?", *End))
						break;
				}
				TokenStart = Start;
				TokenLength = (End ? ((End != Start) ? End - Start : 1) : wcslen(Start));

				switch (TokenStart[0])
				{
				case L'*':	// AnyRepeat
					break;
				case L'?':	// Any
					text++;
					break;
				default:	// Template
					{
						if (PrevTokenStart && (PrevTokenStart[0] == L'*'))
						{
							if (IgnoreCase)
							{
								if (text = _wcsnistr(text, TokenStart, TokenLength))
									text += TokenLength;
							}
							else
							{
								if (text = _wcsnstr(text, TokenStart, TokenLength))
									text += TokenLength;
							}
						}
						else
						{
							if (IgnoreCase)
							{
								if (!_wcsnicmp(text, TokenStart, TokenLength))
									text += TokenLength;
								else
									text = nullptr;
							}
							else
							{
								if (!wcsncmp(text, TokenStart, TokenLength))
									text += TokenLength;
								else
									text = nullptr;
							}
						}
					}
					break;
				}

				Start += TokenLength;

				PrevTokenStart = TokenStart;
				PrevTokenLength = TokenLength;
			}

			return ((text != nullptr) && (!*text));
		}

		bool operator ==(const wchar_t* data) const
		{
			return Compare(data, CompareIgnoreCase);
		}

		bool operator !=(const wchar_t* data) const
		{
			return (!(*this == data));
		}

		bool Compare(const WString& data, const bool IgnoreCase = false, uInt Count = 0) const
		{
			if (!data.Data)
				return !Data;

			if (!Count ? (data.Size == Size) : ((Size >= Count) && (data.Size >= Count)))
			{
				if (!Count)
					Count = Size;

				if (IgnoreCase)
					return (_wcsnicmp(Data, data.Data, Count) == 0);
				return (memcmp(Data, data.Data, Count * sizeof(wchar_t)) == 0);
			}
			return false;
		}

		bool operator ==(const WString& data) const
		{
			return Compare(data, CompareIgnoreCase || data.CompareIgnoreCase);
		}

		bool operator !=(const WString& data) const
		{
			return (!(data == *this));
		}

		bool operator >(const WString& data) const
		{
			if (Size > data.Size)
				return true;
			if (Size < data.Size)
				return false;
			return (memcmp(Data, data.Data, ((Size < data.Size) ? Size : data.Size) * sizeof(wchar_t)) > 0);
		}

		operator bool() const
		{
			return (Size > 0);
		}

		operator const wchar_t*() const
		{
			return Data;
		}

		wchar_t operator [](const int index) const
		{
			if ((index >= 0) && (index < static_cast<int>(Size)))
				return Data[index];
			return L'\0';
		}

		wchar_t GetFirst(const int index = 0) const
		{
			if ((index >= 0) && (index < static_cast<int>(Size)))
				return Data[index];
			return L'\0';
		}

		wchar_t GetLast(const int index = 0) const
		{
			if ((index >= 0) && (index < static_cast<int>(Size)))
				return Data[Size - 1 - index];
			return L'\0';
		}

		uInt Length() const
		{
			return Size;
		}

		const wchar_t* GetData(const wchar_t* sub = nullptr, const uInt index = 0, const uInt offset = 0) const
		{
			if (sub && Data)
			{
				const wchar_t* pos = wcsstr(Data, sub);
				for (uInt i = 0; pos && (i < index); i++)
					pos = wcsstr(pos + 1, sub);
				return pos ? &pos[offset] : nullptr;
			}
			return Data;
		}

		bool GetCompareIgnoreCase() const
		{
			return CompareIgnoreCase;
		}

		WString& SetCompareIgnoreCase(const bool IgnoreCase)
		{
			CompareIgnoreCase = IgnoreCase;
			return *this;
		}

		const uInt GetSubCount(const wchar_t* sub, Array<const wchar_t*>* Subs = nullptr) const
		{
			uInt Result = 0;
			if (sub)
			{
				uInt SubLen = CalcLength(sub);
				for (wchar_t* Pos = Data; Pos;)
				{
					if (Subs)
						Subs->Add() = Pos;
					if (Pos = wcsstr(Pos, sub))
					{
						Pos += SubLen;
						Result++;
					}
				}
			}
			return Result;
		}

		const wchar_t* FirstCharNotInSet(const wchar_t* set) const
		{
			if (Data)
			{
				uInt Pos = wcsspn(Data, set);
				if (Pos != CalcLength(Data))
					return Data + Pos;
			}
			return nullptr;
		}

		const wchar_t* FirstCharInSet(const wchar_t* set) const
		{
			if (Data)
			{
				for (uInt i = 0; i < Size; i++)
				{
					if (wcschr(set, Data[i]))
						return &Data[i];
				}
			}
			return nullptr;
		}

		WString& ToLowerCase()
		{
			if (Data)
				_wcslwr_s(Data, Size + 1);
			return *this;
		}

		WString& ToUpperCase()
		{
			if (Data)
				_wcsupr_s(Data, Size + 1);
			return *this;
		}

		WString* GetLowerCase() const
		{
			auto Result = new WString(*this);
			Result->ToLowerCase();
			return Result;
		}

		WString* GetUpperCase() const
		{
			auto Result = new WString(*this);
			Result->ToUpperCase();
			return Result;
		}

		WString& ToEscapeChars()
		{
			if (Data && Size && wcspbrk(Data, L"\a\b\f\n\r\t\v\\\'\"\?"))
			{
				uInt NewSize = 0;
				auto NewData = new wchar_t[Size * 2];
				for (uInt c = 0; c < Size; c++)
				{
					switch (Data[c])
					{
					case L'\a':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'a';
						break;
					case L'\b':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'b';
						break;
					case L'\f':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'f';
						break;
					case L'\n':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'n';
						break;
					case L'\r':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'r';
						break;
					case L'\t':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L't';
						break;
					case L'\v':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'v';
						break;
					case L'\\':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'\\';
						break;
					case L'\'':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'\'';
						break;
					case L'\"':
						NewData[NewSize++] = L'\\';
						NewData[NewSize++] = L'\"';
						break;
					default:
						NewData[NewSize++] = Data[c];
						break;
					}
				}
				delete[] Data;
				Data = NewData;
				Size = NewSize;
			}
			return *this;
		}

		WString& ToValuableFloat(const int afterdot = -1)
		{
			if (Data)
			{
				const wchar_t* Dot = GetData(L".");
				if (Dot)
				{
					if (afterdot >= 0)
					{
						size_t num = Size - (Dot - Data);
						if (num > 1)
						{
							num--;
							if (num > static_cast<size_t>(afterdot))
							{
								Size = Size - (num - afterdot);
								if (!afterdot && Size)
									Size--;

								if (Size)
									Data[Size] = L'\0';
								else
								{
									Assign(L"0");
									return *this;
								}
							}
						}
					}

					uInt i = Size;
					for (; i > 0; i--)
					{
						if (Data[i - 1] != L'0')
							break;
					}
					if (i && Data[i - 1] == L'.')
						i--;

					if (i)
					{
						Data[i] = L'\0';
						Size = i;
					}
					else
						Assign(L"0");
				}
			}
			return *this;
		}

		WString* GetValuableFloat(const int afterdot = -1) const
		{
			auto Result = new WString(*this);
			Result->ToValuableFloat(afterdot);
			return Result;
		}

		WString* GetToken(const uInt index, const wchar_t* delim, WString* result = nullptr) const
		{
			WString* Result = nullptr;
			if (delim && Data && Size)
			{
				uInt DelimLen = CalcLength(delim);

				wchar_t* Pos = Data;
				for (uInt i = 0; Pos; i++)
				{
					wchar_t* OldPos = Pos;
					if (Pos = wcsstr(OldPos, delim))
						Pos += DelimLen;

					if (i == index)
					{
						if (Pos)
						{
							if (!result)
								Result = new WString(OldPos, static_cast<uInt>(Pos - OldPos) - DelimLen);
							else
								(Result = result)->Assign(OldPos, static_cast<uInt>(Pos - OldPos) - DelimLen);
						}
						else
						{
							if (!result)
								Result = new WString(OldPos);
							else
								(Result = result)->Assign(OldPos);
						}
						break;
					}
				}
			}
			return Result;
		}

		ObjectArray<WString>* GetAllTokens(const wchar_t* delim, const bool onlyifcontain = false) const
		{
			ObjectArray<WString>* Result = nullptr;
			if (delim && Data && Size && (!onlyifcontain || (GetData(delim) != nullptr)))
			{
				uInt DelimLen = CalcLength(delim);
				Array<const wchar_t*> Subs;
				Subs.SetGranularity(256);
				GetSubCount(delim, &Subs);

				Result = new ObjectArray<WString>(Subs.Size());
				for (uInt i = 0; i < Subs.Size(); i++)
				{
					if (i == Subs.Size() - 1)
					{
						if (*Subs[i] != L'\0')
							Result->Add() = WString(Subs[i]);
					}
					else
						Result->Add() = WString(Subs[i], Subs[i + 1] - Subs[i] - DelimLen);
				}
			}
			return Result;
		}

		WString* GetLastToken(const wchar_t* delim, WString* result = nullptr) const
		{
			WString* Result = nullptr;
			if (delim && Data && Size)
			{
				if (result)
					Result = result;
				else
					Result = new WString;

				uInt DelimLen = CalcLength(delim);

				auto Pos = (const wchar_t*)_memrstr((uChar*)Data,
				                                    Size * sizeof(wchar_t),
				                                    (uChar*)delim,
				                                    DelimLen * sizeof(wchar_t));
				if (Pos)
					Result->Assign(&Pos[DelimLen], static_cast<uInt>(&Data[Size] - &Pos[DelimLen]));
				else
					Result->Assign(Data, Size);
			}
			return Result;
		}

		uInt TruncateAfterFirst(const wchar_t* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				wchar_t* Pos = wcsstr(Data, sub);
				for (uInt i = 0; Pos && (i < index) && (Pos = wcsstr(Pos + 1, sub)); i++);

				if (Pos)
				{
					uInt NewSize = static_cast<uInt>(Pos - Data);
					Delta = Size - NewSize;
					Assign(Data, NewSize);
				}
			}
			return Delta;
		}

		uInt TruncateBeforeFirst(const wchar_t* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				uInt SubLen = CalcLength(sub);

				wchar_t* Pos = wcsstr(Data, sub);
				for (uInt i = 0; Pos && (i < index) && (Pos = wcsstr(Pos + 1, sub)); i++);

				if (Pos)
				{
					Delta = static_cast<uInt>(Pos + SubLen - Data);
					Assign(Pos + SubLen);
				}
			}
			return Delta;
		}

		uInt TruncateAfterLast(const wchar_t* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				uInt SubLen = CalcLength(sub);

				const wchar_t* Pos = _wcsrnstr(Data, sub, Size, SubLen);
				for (uInt i = 0; i < index; i++)
				{
					if ((Pos <= Data) || !(Pos = _wcsrnstr(Data, sub, Pos - Data, SubLen)))
						return 0;
				}

				if (Pos)
				{
					Delta = static_cast<uInt>(Data + Size - Pos);
					Assign(Data, static_cast<uInt>(Pos - Data));
				}
			}
			return Delta;
		}

		uInt TruncateBeforeLast(const wchar_t* sub, const uInt index = 0)
		{
			uInt Delta = 0;
			if (Data && sub)
			{
				uInt SubLen = CalcLength(sub);

				const wchar_t* Pos = _wcsrnstr(Data, sub, Size, SubLen);
				for (uInt i = 0; i < index; i++)
				{
					if ((Pos <= Data) || !(Pos = _wcsrnstr(Data, sub, Pos - Data, SubLen)))
						return 0;
				}

				if (Pos)
				{
					Delta = static_cast<uInt>(Pos - Data) + SubLen;
					Assign(Pos + SubLen);
				}
			}
			return Delta;
		}

		uInt TruncateByLength(const uInt NewLength)
		{
			if (!NewLength)
			{
				uInt OldSize = Size;
				Clear();
				return OldSize;
			}

			if (NewLength >= Size)
				return 0;

			uInt Delta = Size - NewLength;
			Resize(NewLength);

			return Delta;
		}

		uInt Replace(const wchar_t* From, const wchar_t* To)
		{
			int n = 0;
			if (Data)
			{
				uInt FromLen = CalcLength(From);
				uInt ToLen = CalcLength(To);

				AString Temp;
				for (wchar_t* Pos = Data; Pos = wcsstr(Pos, From);)
				{
					n++;

					Temp.Assign(Pos[FromLen]);

					*Pos = L'\0';
					uInt LastSize = (Size = static_cast<uInt>(Pos - Data));

					Append(To, ToLen);
					Append(Temp);

					Pos = &Data[LastSize];
				}
			}
			return n;
		}

		WString& GetReplaced(const wchar_t* From, const wchar_t* To)
		{
			Replace(From, To);
			return *this;
		}

		uInt Replace(const wchar_t Character, const wchar_t Replace)
		{
			int n = 0;
			if (Data)
			{
				for (uInt i = 0; i < Size; i++)
				{
					if (Data[i] == Character)
					{
						Data[i] = Replace;
						n++;
					}
				}
			}
			return n;
		}

		WString& GetReplaced(const wchar_t Character, const wchar_t cReplace)
		{
			Replace(Character, cReplace);
			return *this;
		}

		uInt CleanUp(const wchar_t Character)
		{
			uInt Delta = 0;
			if (Data)
			{
				uInt n = 0;
				for (uInt i = 0; i < Size; i++)
				{
					if (Data[i] != Character)
						Data[n++] = Data[i];
				}
				Delta = Size - n;
				Resize(n);
			}
			return Delta;
		}

		int ToInt() const
		{
			if (Data)
				return _wtoi(Data);
			return 0;
		}

		bool IsInt() const
		{
			return Size && (FirstCharNotInSet(L"-0123456789") == nullptr);
		}

		uInt ToUInt() const
		{
			uInt Result = 0;
			if (Data)
				swscanf_s(Data, GetData(L"x") ? L"%x" : L"%u", &Result);
			return Result;
		}

		bool IsUInt() const
		{
			return Size && (FirstCharNotInSet(L"x0123456789ABCDEFabcdef") == nullptr);
		}

		float ToFloat() const
		{
			if (Data)
				return static_cast<float>(_wtof(Data));
			return 0.0f;
		}

		bool IsFloat() const
		{
			return Size && (FirstCharNotInSet(L"-0123456789.") == nullptr);
		}

		bool ToBool() const
		{
			if (Data)
				return Compare(L"true", true);
			return false;
		}

		bool IsBool() const
		{
			return Size && (Compare(L"true", true) || Compare(L"false", true));
		}

		WString()
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
		}

		WString(const wchar_t* data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		WString(const char* data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		WString(const wchar_t data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		WString(const char data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		WString(const wchar_t* data, const uInt length)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data, length);
		}

		WString(const char* data, const uInt length)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data, length);
		}

		WString(const WString& data)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(data);
		}

		WString(const int value, const int radix = 10)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(value, radix);
		}

		WString(const uInt value, const int radix = 10)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(value, radix);
		}

		WString(const float value)
		{
			Data = nullptr;
			Size = 0;
			CompareIgnoreCase = false;
			Assign(value);
		}

		~WString() override
		{
			Clear();
		}
	};

	using String = AString;

#ifdef _UNICODE
	using TString = WString;
#else
	typedef AString TString;
#endif

	using AStringPtr = AutoPtr<AString>;
	using WStringPtr = AutoPtr<WString>;
	using TStringPtr = AutoPtr<TString>;

	using StringArray = ObjectArray<String>;
	using StringArrayPtr = AutoPtr<StringArray>;
	using AStringArray = ObjectArray<AString>;
	using AStringArrayPtr = AutoPtr<AStringArray>;
	using WStringArray = ObjectArray<WString>;
	using WStringArrayPtr = AutoPtr<WStringArray>;
	using TStringArray = ObjectArray<TString>;
	using TStringArrayPtr = AutoPtr<TStringArray>;
}

#endif
