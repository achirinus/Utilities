#undef UNICODE
#undef _UNICODE

#include <cstdio>
#include <cstring>
#include <windows.h>
#include <vector>
#include <ctype.h>

#define OUTPUT_LINE_LENGTH 100

struct FileData
{
	std::string ShortName;
	std::string FullName;
	unsigned Size;
};

char* SearchTerm;
std::vector<FileData> Files;
std::vector<std::string> FilesSugested;


char* ToLower(char* str)
{
	char* result = str;
	if (!str) return str;
	while (*str)
	{
		*str = tolower(*str);
		*str++;
	}
	return result;
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
			if (FilesSugested.size()) ShouldProcess = false;

			for (std::string& FileSugested : FilesSugested)
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
		fopen_s(&pFile, names.FullName.c_str(), "r");
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
				
				char temp[OUTPUT_LINE_LENGTH + 1];
				int count = LineEnd - LineStart;

				if (count <= OUTPUT_LINE_LENGTH)
				{
					strncpy_s(temp, LineStart, count);
				}
				else
				{
					//TODO this is not safe, those numbers can be <=0
					int AvailableCharCount = OUTPUT_LINE_LENGTH - SearchTermSize - 7;
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

					char lineToString[OUTPUT_LINE_LENGTH];
					strncpy_s(lineToString, temp, BeginOfSearchString - temp);
					char lineFromString[OUTPUT_LINE_LENGTH];

					int SearchLen = strlen(SearchTerm);
					char* EndOfSearch = BeginOfSearchString + SearchLen;

					strcpy_s(lineFromString, EndOfSearch);

					printf("%s(%d): ", names.ShortName.c_str(), LineNumber);
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

int main(int argc, char* argv[])
{
	
#ifdef _DEBUG
	SearchTerm = "unsigned char*>(&value)), static";
	SetCurrentDirectoryA("D:\\workspace\\InstantWar\\AndroidUpdate4");
#else
	SearchTerm = argv[1];
	if (argc < 2) return 1;

#endif
	
	if (argc > 2)
	{
		//Files or wildcards supplied
		for (int i = 2; i < argc; i++)
		{
			FilesSugested.push_back(argv[i]);
		}
	}


	GetAllFilesInDir();

	SearchFiles();

	return 0;
}