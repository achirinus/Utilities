#undef UNICODE
#undef _UNICODE

#include <cstdio>
#include <cstring>
#include <windows.h>
#include <vector>

char* StringToSearch;

struct FileData
{
	std::string ShortName;
	std::string FullName;
	unsigned Size;
};

std::vector<FileData> Files;

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
			GetCurrentDirectoryA(MAX_PATH, FullFileName);
			strcat_s(FullFileName, "\\");
			strcat_s(FullFileName, CurrentFileData.cFileName);
			
			Files.push_back({ CurrentFileData.cFileName, FullFileName, CurrentFileData.nFileSizeLow});
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
				if (char* BeginOfSearchString = strstr(temp, StringToSearch))
				{
					HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
					CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
					GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
					WORD currentColor = consoleInfo.wAttributes;

					char lineToString[256];
					strncpy_s(lineToString, temp, BeginOfSearchString - temp);
					char lineFromString[256];

					int SearchLen = strlen(StringToSearch);
					char* EndOfSearch = BeginOfSearchString + SearchLen;

					strcpy_s(lineFromString, EndOfSearch);

					printf("%s(%d): ", names.ShortName.c_str(), LineNumber);
					printf("%s", lineToString);
					SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);

					printf("%s", StringToSearch);

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
	//if (argc < 2) return 1;
#ifdef _DEBUG
	StringToSearch = "ShadyApp";
	SetCurrentDirectoryA("C:\\Users\\ALIN");
#else
	StringToSearch = argv[1];
#endif
	
	GetAllFilesInDir();

	SearchFiles();

	return 0;
}