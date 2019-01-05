
#include "Main.h"

const char* SearchTerm;
int OutputLineLength = 100;
char StartingWorkingDir[MAX_PATH];

std::vector<FileData> Files;
std::vector<std::string> FilesIncluded;
std::vector<std::string> FilesExcluded;
char* CommandLine;

int main(int argc, char* argv[])
{
	
#ifdef _DEBUG
	SearchTerm = "unsigned char*>(&value)), static";
	strcpy(StartingWorkingDir, "D:\\workspace\\InstantWar\\AndroidUpdate4");
	SetCurrentDirectoryA(StartingWorkingDir);
#else
	if (argc < 2) return 1;

#endif
	GetCurrentDirectoryA(MAX_PATH, StartingWorkingDir);

	CommandLine = GetCommandLine();
	
	OptionBuffer optionBuffer = ParseCommandLine(CommandLine);

	ParseOptions(optionBuffer);
	
	
	BeginCounter();

	GetAllFilesInDir();

	int GetFilesTime = EndCounter();

	printf("GetAllFilesInDir time: %dms", GetFilesTime);

	SearchFiles();

	return 0;
}

void GetAllFilesInDir()
{
	char CurrentDir[MAX_PATH];
	char FullFileName[MAX_PATH];
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
			if (FilesIncluded.size()) ShouldProcess = false;

			for (std::string& FileSugested : FilesIncluded)
			{
				char TempSugestedName[1024];
				char TempFileName[1024];
				strcpy_s(TempSugestedName, FileSugested.c_str());
				strcpy_s(TempFileName, CurrentFileData.cFileName);

				char* LoweredTempSug = ToLower(TempSugestedName);
				char* LoweredTempFile = ToLower(TempFileName);
				//Handle wild card
				if (strstr(TempSugestedName, "*.") == TempSugestedName)
				{
					if (strstr(LoweredTempFile, LoweredTempSug + 1))
					{
						ShouldProcess = true;
						break;
					}
				}
				else
				{
					if (strstr(LoweredTempFile, LoweredTempSug) && (strlen(LoweredTempFile) == strlen(LoweredTempSug)))
					{
						ShouldProcess = true;
						break;
					}
				}
			}
			if (ShouldProcess)
			{
				GetCurrentDirectoryA(MAX_PATH, FullFileName);
				strcat_s(FullFileName, "\\");
				strcat_s(FullFileName, CurrentFileData.cFileName);

				Files.push_back({ CurrentFileData.cFileName, FullFileName, CurrentFileData.nFileSizeLow });
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
		fopen_s(&pFile, names.AbsPath, "r");
		if (pFile == nullptr) continue;
		if (names.Size == 0) continue;
		char* Contents = new char[names.Size];
		char* TempCont = Contents;
		fread(Contents, 1, names.Size, pFile);

		int LineNumber = 1;
		int SearchTermSize = strlen(SearchTerm);

		char* LineStart = Contents;
		char* LineEnd = 0;

		while (*TempCont)
		{
			if (*TempCont == '\n')
			{
				LineEnd = TempCont;

				char temp[MAX_LINE_BUFFER_LENGTH + 1];
				int count = LineEnd - LineStart;

				if (count <= OutputLineLength)
				{
					strncpy_s(temp, LineStart, count);
				}
				else
				{
					//TODO this is not safe, those numbers can be <=0
					int AvailableCharCount = OutputLineLength - SearchTermSize - 7;
					int NumberOfSideChars = AvailableCharCount / 2;
					char* BeginOfSearchString = strstr(LineStart, SearchTerm);
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

						tempStr += SearchTerm;

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

				if (char* BeginOfSearchString = strstr(temp, SearchTerm))
				{
					HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
					CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
					GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
					WORD currentColor = consoleInfo.wAttributes;

					char lineToString[MAX_LINE_BUFFER_LENGTH];
					strncpy_s(lineToString, temp, BeginOfSearchString - temp);
					char lineFromString[MAX_LINE_BUFFER_LENGTH];

					int SearchLen = strlen(SearchTerm);
					char* EndOfSearch = BeginOfSearchString + SearchLen;

					strcpy_s(lineFromString, EndOfSearch);

					char* relativeFilePath = GetRelativePath(StartingWorkingDir, names.AbsPath);

					printf("%s(%d): ", relativeFilePath, LineNumber);
					printf("%s", lineToString);
					SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

					printf("%s", SearchTerm);

					SetConsoleTextAttribute(hConsole, currentColor);

					printf("%s\n", lineFromString);
				}
				LineNumber++;
				LineStart = TempCont + 1;
			}
			TempCont++;
		}

		delete[] Contents;
		fclose(pFile);
	}
}


void ParseOptions(OptionBuffer optBuffer)
{
	for (int i = 0; i < optBuffer.Size; i++)
	{
		ProgramOption option = optBuffer.Buffer[i];
		switch (option.Name)
		{
			case 's':
			{
				//TODO this needs to be parsed now
				SearchTerm = option.Args; 
			}break;

			case 'i':
			{
				//TODO this needs to be parsed now
				FilesIncluded.push_back(option.Args);
			}break;

			default:
			{
				printf("Ignoring unknown argument %c.", option.Name);
			}break;
		}
	}
}

OptionBuffer ParseCommandLine(char* line)
{
	OptionBuffer result = { };
	result.Buffer = new ProgramOption[SUPPORTED_OPTIONS];

	int optPos = FindString(line, " -");
	while (optPos != -1)
	{
		ProgramOption opt = {};
		opt.Name = line[optPos + 2];

		//See if there is an open bracket or quote before searching the next arg
		char* somethingToClose = 0;
		int argPos = optPos + 3;

		if (line[argPos] == '[') somethingToClose = "]";
		if (line[argPos] == '\"') somethingToClose = "\"";

		int somethingToClosePos = -1;

		if (somethingToClose)
		{
			somethingToClosePos = FindString(line, somethingToClose, argPos + 1);
			if (somethingToClosePos == -1)
			{
				TerminateError("No matching %s found for option [%c]", somethingToClose, opt.Name);
			}
			optPos = FindString(line, " -", somethingToClosePos);
			opt.Args = Substring(line, argPos);
		}
		else
		{
			optPos = FindString(line, " -");
			opt.Args = Substring(line, argPos, optPos - argPos);
		}
		
		
		result.Buffer[result.Size++] = opt;
	}
	return result;
}

