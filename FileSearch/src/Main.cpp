
#include "Helpers.h"


char StartingWorkingDir[MAX_PATH];
char InitialWorkingDir[MAX_PATH];

std::vector<FileData> Files;
ProgramSettings Settings;

int main(int argc, char* argv[])
{
	GetCurrentDirectoryA(MAX_PATH, InitialWorkingDir);
	ReadProgramProperties(argv, argc);
#ifdef _DEBUG
	strcpy(StartingWorkingDir, "E:\\workspace\\InstantWar\\AndroidUpdate4\\Source");
	SetCurrentDirectoryA(StartingWorkingDir);
#else
	if (argc < 2) return 1;

#endif

	GetCurrentDirectoryA(MAX_PATH, StartingWorkingDir);

	BeginCounter();
	GetAllFilesInDir();
	int GetFilesDuration = EndCounter();

	if(Settings.ShowTimes) printf("File gather duration: %dms\n", GetFilesDuration);
	
	BeginCounter();
	SearchFiles();
	int SearchFilesDuration = EndCounter();
	if (Settings.ShowTimes) printf("Files search duration: %dms\n", SearchFilesDuration);

	if (Settings.ShowTimes) printf("Total time: %dms\n", SearchFilesDuration + GetFilesDuration);
	return 0;
}

void GetAllFilesInDir()
{
	char CurrentDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, CurrentDir);
	WIN32_FIND_DATA CurrentFileData;
	strcat_s(CurrentDir, "\\*");
	HANDLE FindHandle = FindFirstFile(CurrentDir, &CurrentFileData);
	while (FindNextFile(FindHandle, &CurrentFileData))
	{
		if (CurrentFileData.cFileName[0] == '.') continue;

		if (CurrentFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			SetCurrentDirectory(CurrentFileData.cFileName);
			GetAllFilesInDir();
		}
		else
		{
			bool ShouldProcess = true;
			if (Settings.FilesToInclude.Size) ShouldProcess = false;

			for (int i = 0; i < Settings.FilesToInclude.Size; i++)
			{
				char* FileSugested = Settings.FilesToInclude.Strings[i];
				char TempSugestedName[1024];
				char TempFileName[1024];
				strcpy_s(TempSugestedName, FileSugested);
				strcpy_s(TempFileName, CurrentFileData.cFileName);

				char* LoweredTempSug = ToLower(TempSugestedName);
				char* LoweredTempFile = ToLower(TempFileName);
				//Handle wild card
				int preWildCardIndex = FindString(TempSugestedName, "*.");
				int postWildCardIndex = FindString(TempSugestedName, ".*");
				if (preWildCardIndex == 0)
				{
					if (FindString(LoweredTempFile, TempSugestedName + 1) >= 0)
					{
						ShouldProcess = true;
						break;
					}
				}
				else if (postWildCardIndex > 0)
				{
					//@Leak
					char* beforeWildCard = Substring(TempSugestedName, 0, StringSize(TempSugestedName) - 1);
					if (FindString(LoweredTempFile, beforeWildCard) >= 0)
					{
						ShouldProcess = true;
						break;
					}
				}
				else
				{
					int index = FindString(LoweredTempFile, LoweredTempSug);
					if (StringCompare(LoweredTempFile, LoweredTempSug))
					{
						ShouldProcess = true;
						break;
					}
				}
			}
			for (int i = 0; i < Settings.FilesToExclude.Size; i++)
			{
				char* FileSugested = Settings.FilesToExclude.Strings[i];
				char TempSugestedName[1024];
				char TempFileName[1024];
				strcpy_s(TempSugestedName, FileSugested);
				strcpy_s(TempFileName, CurrentFileData.cFileName);

				char* LoweredTempSug = ToLower(TempSugestedName);
				char* LoweredTempFile = ToLower(TempFileName);

				//Handle wild card
				int preWildCardIndex = FindString(TempSugestedName, "*.");
				int postWildCardIndex = FindString(TempSugestedName, ".*");
				if (preWildCardIndex == 0)
				{
					if (FindString(LoweredTempFile, TempSugestedName + 1) >= 0)
					{
						ShouldProcess = false;
						break;
					}
				}
				else if (postWildCardIndex > 0)
				{
					//@Leak
					char* beforeWildCard = Substring(TempSugestedName, 0, StringSize(TempSugestedName) - 1);
					if (FindString(LoweredTempFile, beforeWildCard) >= 0)
					{
						ShouldProcess = false;
						break;
					}
				}
				else
				{
					if (StringCompare(LoweredTempFile, LoweredTempSug))
					{
						ShouldProcess = false;
						break;
					}
				}
			}
			if (ShouldProcess)
			{
				char CurrentDir[MAX_PATH];
				GetCurrentDirectoryA(MAX_PATH, CurrentDir);
				//@Leak
				char* FileName = StringCopy(CurrentFileData.cFileName);
				char* first = StringConcat(CurrentDir, "\\");
				char* FullFileName = StringConcat(first, FileName);
				Files.push_back({ FileName, FullFileName, CurrentFileData.nFileSizeLow });
			}
		}
	}
	SetCurrentDirectory("..");
	FindClose(FindHandle);
}


void SearchFiles()
{
	for (FileData& names : Files)
	{
		FILE* pFile = 0;
		fopen_s(&pFile, names.AbsPath, "rb");
		if (pFile == nullptr) continue;
		if (names.Size == 0) continue;
		char* Contents = new char[names.Size + 1];
		
		char* TempCont = Contents;
		fread(Contents, 1, names.Size, pFile);
		Contents[names.Size] = 0;
		int LineNumber = 1;
		int SearchTermSize = StringSize(Settings.SearchTerm);
		bool ShouldBreak = false;
		char* Line = ReadStringLine(&TempCont);
		while (Line)
		{
			char temp[MAX_LINE_BUFFER_LENGTH + 1];
			int count = StringSize(Line);

			if (count <= Settings.OutputLineLength)
			{
				strncpy_s(temp, Line, count);
			}
			else
			{
				//TODO this is not safe, those numbers can be <=0
				int AvailableCharCount = Settings.OutputLineLength - SearchTermSize - 7;
				int NumberOfSideChars = AvailableCharCount / 2;
				int FoundIndex = FindString(Line, Settings.SearchTerm);
				
				if (FoundIndex >= 0)
				{
					char* BeginOfSearchString = Line + FoundIndex;
					int NumOfCharsBeforeTerm = BeginOfSearchString - Line;
					int NumOfCharsAfterTerm = (Line + count) - (BeginOfSearchString + SearchTermSize);

					bool PreDotsRequired = false;
					bool PostDotsRequired = false;

					if (NumOfCharsBeforeTerm > NumberOfSideChars)
					{
						PreDotsRequired = true;
						NumOfCharsBeforeTerm = NumberOfSideChars;
					}
					if (NumOfCharsAfterTerm > NumberOfSideChars)
					{
						PostDotsRequired = true;
						NumOfCharsAfterTerm = NumberOfSideChars;
					}
					std::string tempStr;

					if (PreDotsRequired)
					{
						tempStr += "...";
					}
					char* StartOfPre = BeginOfSearchString - NumOfCharsBeforeTerm;
					for (int i = 0; i < NumOfCharsBeforeTerm; i++)
					{
						tempStr += StartOfPre[i];
					}

					tempStr += Settings.SearchTerm;

					char* StartOfAfter = BeginOfSearchString + SearchTermSize;
					for (int i = 0; i < NumOfCharsAfterTerm; i++)
					{
						tempStr += StartOfAfter[i];
					}

					if (PostDotsRequired)
					{
						tempStr += "...";
					}

					strncpy_s(temp, tempStr.c_str(), tempStr.size() + 1);
				}
			}

			if (char* BeginOfSearchString = strstr(temp, Settings.SearchTerm))
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
				GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
				WORD currentColor = consoleInfo.wAttributes;

				char lineToString[MAX_LINE_BUFFER_LENGTH];
				strncpy_s(lineToString, temp, BeginOfSearchString - temp);
				char lineFromString[MAX_LINE_BUFFER_LENGTH];

				int SearchLen = StringSize(Settings.SearchTerm);
				char* EndOfSearch = BeginOfSearchString + SearchLen;

				strcpy_s(lineFromString, EndOfSearch);
				char* filename = names.FileName;
				if (Settings.LongFilename)
				{
					filename = GetRelativePath(StartingWorkingDir, names.AbsPath);
				}
				printf("%s(%d): ", filename, LineNumber);
				printf("%s", lineToString);
				SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

				printf("%s", Settings.SearchTerm);

				SetConsoleTextAttribute(hConsole, currentColor);

				printf("%s\n", lineFromString);
			}
			LineNumber++;
			delete[] Line;
			Line = 0;
			Line = ReadStringLine(&TempCont);
		}

		delete[] Contents;
		fclose(pFile);
	}
}

void ReadProgramProperties(char* argv[], int argc)
{
	//Set default properties that will be overriden if it exists in file
	Settings.NumberOfThreads = 1;
	Settings.OutputLineLength = 256;
	Settings.LongFilename = false;
	Settings.ShowTimes = false;

	FILE* file = 0;
	char* exeName = GetExePath();
	char* exeDir = GetLastDirPath(exeName);
	char* fullFileName = StringConcat(exeDir, "config.fsinfo");
	fopen_s(&file, fullFileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		int fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (!fileSize) return;
		char* fileContents = new char[fileSize + 1];
		fread(fileContents, 1, fileSize, file);
		fileContents[fileSize] = 0;

		char* line = 0;
		char* remainingContents = fileContents;
		
		do
		{
			//@Leak ReadStringLine allocates the string that returns
			line = ReadStringLine(&remainingContents);
			if (StartsWith(line, "exclude"))
			{
				char* valueStr = SkipString(line, "exclude");
				valueStr++;
				Settings.FilesToExclude = BreakStringByToken(valueStr, ',');
			}
			else if (StartsWith(line, "threads"))
			{
				Settings.NumberOfThreads = GetIntValue(line, "threads");
			}
			else if (StartsWith(line, "line"))
			{
				Settings.OutputLineLength = GetIntValue(line, "line");
			}
			else if (StartsWith(line, "filename"))
			{
				Settings.LongFilename = GetBoolValueWithOptions(line, "filename", "long", "short");
			}
			else if (StartsWith(line, "show_times"))
			{
				Settings.ShowTimes = GetBoolValue(line, "show_times");
			}
		}
		while (line);
	}

	Settings.SearchTerm = argv[1];
	for (int i = 2; i < argc; i++)
	{
		StringBuffer& buf = Settings.FilesToInclude;
		buf.Strings[buf.Size++] = argv[i];
	}
}

