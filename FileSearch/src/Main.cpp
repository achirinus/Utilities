#undef UNICODE
#undef _UNICODE

#include <cstdio>
#include <cstring>
#include <windows.h>
#include <vector>
#include <ctype.h>

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
				char TempSugestedName[128];
				char TempFileName[128];
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

		char* LineStart = Contents;
		char* LineEnd = 0;

		while (*TempCont)
		{
			if (*TempCont == '\n')
			{
				LineEnd = TempCont;
				
				char temp[512];
				strncpy_s(temp, LineStart, LineEnd - LineStart);
				if (char* BeginOfSearchString = strstr(temp, SearchTerm))
				{
					HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
					CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
					GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
					WORD currentColor = consoleInfo.wAttributes;

					char lineToString[256];
					strncpy_s(lineToString, temp, BeginOfSearchString - temp);
					char lineFromString[256];

					int SearchLen = strlen(SearchTerm);
					char* EndOfSearch = BeginOfSearchString + SearchLen;

					strcpy_s(lineFromString, EndOfSearch);

					printf("%s(%d): ", names.ShortName.c_str(), LineNumber);
					printf("%s", lineToString);
					SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);

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
	SearchTerm = "ShadyApp";
	SetCurrentDirectoryA("C:\\Users\\ALIN");
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