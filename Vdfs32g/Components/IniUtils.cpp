#include "PreCompiled.h"

// Read a string value from an INI file section/key, returning default if not found
bool ReadIniString(const char* section, const char* key, const char* defval, char* val, const size_t size, const char* file)
{
	AString SectionString("[");
	SectionString += section;
	SectionString += "]";

	bool InSection = false;
	bool Done = false;

	FILE* In = nullptr;
	if (!fopen_s(&In, file, "r"))
	{
		if (In)
		{
			char Line[1024];
			while (fgets(Line, 1024, In))
			{
				// Detect the target section header line
				if (SectionString.Compare(Line, true, SectionString.Length()))
				{
					InSection = true;
					continue;
				}

				// Parse key=value pairs within the target section
				if (InSection && strrchr(Line, '='))
				{
					AString Key(Line);
					Key.TruncateAfterFirst("=");
					Key.CleanUp(' ');
					if (Key.Compare(key, true))
					{
						AString Val(Line);
						Val.TruncateBeforeFirst("=");
						Val.TruncateAfterFirst(";");
						Val.CleanUp('\n');

						if (Val.Length())
						{
							strcpy_s(val, size, Val);
							Done = true;
							break;
						}
					}
				}
				// Exit section on encountering another section header
				else if (strrchr(Line, '['))
					InSection = false;
			}
		}

		fclose(In);
	}
	// Return default value if key was not found
	if (!Done)
		strcpy_s(val, size, defval);
	return Done;
}

// Write a string value to an INI file section/key, preserving existing content with .bak backup
bool WriteIniString(const char* section, const char* key, const char* val, const char* file)
{
	AString BakFile(file);
	BakFile += ".bak";

	AString SectionString("[");
	SectionString += section;
	SectionString += "]";

	AString ValString(key);
	ValString += "=";
	ValString += val;

	FILE* In = nullptr;
	if (PathFileExistsA(file))
	{
		// Back up existing file before modifying
		DeleteFileA(BakFile);
		if (!MoveFileA(file, BakFile))
			return false;
		if (fopen_s(&In, BakFile, "r"))
			return false;
	}

	bool SectionDone = false;
	bool Done = false;

	FILE* Out = nullptr;
	if (!fopen_s(&Out, file, "w"))
	{
		if (In)
		{
			char Line[1024];
			while (fgets(Line, 1024, In))
			{
				bool SkipLine = false;

				if (!SectionDone)
				{
					// Once we find the target section, mark it and prepare to write
					if (SectionString.Compare(Line, true, SectionString.Length()))
						SectionDone = true;
				}
				else
				{
					if (!Done)
					{
						// Replace existing key or insert new value at section boundary
						if (strrchr(Line, '='))
						{
							AString Key(Line);
							Key.TruncateAfterFirst("=");
							Key.CleanUp(' ');
							if (Key.Compare(key, true))
							{
								Done = true;
								fprintf(Out, "%s\n", ValString.GetData());
								SkipLine = true;
							}
						}
						else if (strrchr(Line, '['))
						{
							Done = true;
							fprintf(Out, "%s\n", ValString.GetData());
						}
					}
				}
				if (!SkipLine)
				{
					if (Line[strlen(Line) - 1] == '\n')
						fprintf(Out, "%s", Line);
					else
						fprintf(Out, "%s\n", Line);
				}
			}
		}

		// Create section and write key if it didn't exist
		if (!SectionDone)
		{
			SectionDone = true;
			Done = true;

			fprintf(Out, "%s\n", SectionString.GetData());
			fprintf(Out, "%s\n", ValString.GetData());
		}
		else if (!Done)
		{
			Done = true;
			fprintf(Out, "%s\n", ValString.GetData());
		}
		fclose(Out);
	}

	if (In)
	{
		fclose(In);
		DeleteFileA(BakFile);
	}
	return SectionDone && Done;
}

// Read an INI string resolving the path relative to the Gothic game directory
bool GothicReadIniString(const char* section,
                         const char* key,
                         const char* defval,
                         char* val,
                         const size_t size,
                         const char* file)
{
	TString WorkPath;
	if (PlatformGetWorkPath(WorkPath) && WorkPath.TruncateBeforeLast(_T("\\")))
	{
		AString File(file);
		if (!WorkPath.Compare(_T("System"), true))
			File.Format("System\\%s", file);

		return ReadIniString(section, key, defval, val, size, File);
	}
	return false;
}

// Write an INI string resolving the path relative to the Gothic game directory
bool GothicWriteIniString(const char* section, const char* key, const char* val, const char* file)
{
	TString WorkPath;
	if (PlatformGetWorkPath(WorkPath) && WorkPath.TruncateBeforeLast(_T("\\")))
	{
		AString File(file);
		if (!WorkPath.Compare(_T("System"), true))
			File.Format("System\\%s", file);

		return WriteIniString(section, key, val, File);
	}
	return false;
}
