
#include "Main.h"


char StartingWorkingDir[MAX_PATH];

std::vector<FileData> Files;
ProgramSettings Settings;

int main(int argc, char* argv[])
{
	ReadProgramProperties(argv, argc);
#ifdef _DEBUG
	strcpy(StartingWorkingDir, "E:\\workspace\\InstantWar\\AndroidUpdate4");
	SetCurrentDirectoryA(StartingWorkingDir);
#else
	if (argc < 2) return 1;

#endif

	GetCurrentDirectoryA(MAX_PATH, StartingWorkingDir);

	BeginCounter();

	GetAllFilesInDir();

	int GetFilesTime = EndCounter();

	printf("GetAllFilesInDir time: %dms\n", GetFilesTime);

	SearchFiles();

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
		char* LineStart = Contents;
		char* LineEnd = 0;
		bool ShouldBreak = false;
		do
		{
			if ((*TempCont == '\n') || (*TempCont == 0))
			{
				if (*TempCont == 0) ShouldBreak = true;
				LineEnd = TempCont;

				char temp[MAX_LINE_BUFFER_LENGTH + 1];
				int count = LineEnd - LineStart;

				if (count <= Settings.OutputLineLength)
				{
					strncpy_s(temp, LineStart, count);
				}
				else
				{
					//TODO this is not safe, those numbers can be <=0
					int AvailableCharCount = Settings.OutputLineLength - SearchTermSize - 7;
					int NumberOfSideChars = AvailableCharCount / 2;
					char* BeginOfSearchString = strstr(LineStart, Settings.SearchTerm);
					if (BeginOfSearchString && (BeginOfSearchString < LineEnd))
					{
						int NumOfCharsBeforeTerm = BeginOfSearchString - LineStart;
						int NumOfCharsAfterTerm = LineEnd - BeginOfSearchString;

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

					char* relativeFilePath = GetRelativePath(StartingWorkingDir, names.AbsPath);

					printf("%s(%d): ", relativeFilePath, LineNumber);
					printf("%s", lineToString);
					SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

					printf("%s", Settings.SearchTerm);

					SetConsoleTextAttribute(hConsole, currentColor);

					printf("%s\n", lineFromString);
				}
				LineNumber++;
				LineStart = TempCont + 1;
				if (ShouldBreak) break;
			}
			TempCont++;
		}
		while (true);
		

		delete[] Contents;
		fclose(pFile);
	}
}

void ReadProgramProperties(char* argv[], int argc)
{
	//Set default properties that will be overriden if it exists in file
	Settings.NumberOfThreads = 1;

	FILE* file = 0;
	char* fileName = GetExePath();
	char* fullFileName = StringConcat(fileName, "config.fsinfo");
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
				char* valueStr = SkipString(line, "threads");
				valueStr++;
				Settings.NumberOfThreads = StringToInt(valueStr);
			}
			else if (StartsWith(line, "line"))
			{
				char* valueStr = SkipString(line, "line");
				valueStr++;
				Settings.OutputLineLength = StringToInt(valueStr);
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

