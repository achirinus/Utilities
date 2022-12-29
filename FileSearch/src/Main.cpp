
#include "Helpers.h"
#include <thread>
#include <mutex>
#include "Allocator.h"

#define FILES_PER_THREAD 20

FolderStack FStack;

char StartingWorkingDir[MAX_PATH];
std::vector<FileData> Files;

std::vector<std::thread> Threads;
std::mutex OutputMutex;
HANDLE Console;
ProgramSettings Settings;

int main(int argc, char* argv[])
{
	ReadProgramProperties(argv, argc);
#ifdef _DEBUG
	strcpy(StartingWorkingDir, "D:\\workspace\\Downloaded");
	SetCurrentDirectoryA(StartingWorkingDir);
#else
	if (argc < 2) return 1;
	GetCurrentDirectoryA(MAX_PATH, StartingWorkingDir);
#endif
	if (Settings.ShowInfo) printf("Searching in: %s\n", StartingWorkingDir);

	Console = GetStdHandle(STD_OUTPUT_HANDLE);


	BeginCounter();
	printf("Gathering files...\n");
	FindAllFiles();
	int GetFilesDuration = EndCounter();

	if(Settings.ShowTimes) printf("File gather duration: %dms\n", GetFilesDuration);
	if (Settings.ShowStats) printf("Found %d files.\n", Files.size());

	BeginCounter();
	int NumOfFiles = Files.size();

	printf("Searching in files...\n");
	if ((NumOfFiles > 20) && Settings.NumberOfThreads)
	{
		int NumOfFilesPerThread = NumOfFiles / Settings.NumberOfThreads;
		int Remaining = NumOfFiles % Settings.NumberOfThreads;
		FilesIndexRange* Ranges = new FilesIndexRange[Settings.NumberOfThreads];

		int StartIndex = 0;
		for (int i = 0; i < Settings.NumberOfThreads; i++)
		{
			int EndIndex = StartIndex + NumOfFilesPerThread - 1;
			Ranges[i] = { StartIndex, EndIndex};
			StartIndex = EndIndex + 1;
		}
		if (Remaining)
		{
			FilesIndexRange& last = Ranges[Settings.NumberOfThreads - 1];
			last.End += Remaining;
		}
		
		for (int i = 0; i < Settings.NumberOfThreads; i++)
		{
			
			Threads.push_back(std::thread{ SearchFilesRange, Ranges[i] });
		}
		for (size_t i = 0; i < Threads.size(); i++)
		{
			Threads[i].join();
		}
	}
	else
	{
		SearchFiles();
	}
	
	int SearchFilesDuration = EndCounter();
	if (Settings.ShowTimes) printf("Files search duration: %dms\n", SearchFilesDuration);

	if (Settings.ShowTimes) printf("Total time: %dms\n", SearchFilesDuration + GetFilesDuration);
	return 0;
}

void SearchFilesRange(FilesIndexRange range)
{
	Allocator::Arena LineArena;
	Allocator::AllocateArena(&LineArena, 1000000);

	Allocator::Arena ContentArena;
	Allocator::AllocateArena(&ContentArena, 5000000);
	for (int i = range.Begin; i<= range.End; i++)
	{
		FileData& names = Files[i];
		FILE* pFile = 0;
		fopen_s(&pFile, names.AbsPath, "rb");
		if (pFile == nullptr) continue;
		if (names.Size == 0) continue;
		char* Contents = Allocator::ReallocateArena(&ContentArena, names.Size + 1);

		char* TempCont = Contents;
		fread(Contents, 1, names.Size, pFile);
		Contents[names.Size] = 0;
		int LineNumber = 1;
		int SearchTermSize = StringSize(Settings.SearchTerm);
		bool ShouldBreak = false;
		
		do
		{
			char* Line = ReadStringLine(&TempCont, &LineArena);
			char temp[MAX_LINE_BUFFER_LENGTH];
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

				OutputMutex.lock();

				CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
				GetConsoleScreenBufferInfo(Console, &consoleInfo);
				WORD currentColor = consoleInfo.wAttributes;

				printf("%s(%d): ", filename, LineNumber);
				printf("%s", lineToString);
				SetConsoleTextAttribute(Console, FOREGROUND_GREEN);

				printf("%s", Settings.SearchTerm);

				SetConsoleTextAttribute(Console, currentColor);

				printf("%s\n", lineFromString);
				OutputMutex.unlock();
			}
			LineNumber++;
		}
		while (TempCont);
		
		Allocator::ResetArena(&LineArena);
		Allocator::ResetArena(&ContentArena);
		fclose(pFile);
	}
	Allocator::ClearArena(&LineArena);
	Allocator::ClearArena(&ContentArena);
}

void SearchFiles()
{
	SearchFilesRange({ 0, (int)Files.size() - 1});
}

void ReadProgramProperties(char* argv[], int argc)
{
	//Set default properties that will be overriden if it exists in file
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
			else if (StartsWith(line, "filename_format"))
			{
				Settings.LongFilename = GetBoolValueWithOptions(line, "filename_format", "long", "short");
			}
			else if (StartsWith(line, "show_times"))
			{
				Settings.ShowTimes = GetBoolValue(line, "show_times");
			}
			else if (StartsWith(line, "show_stats"))
			{
				Settings.ShowStats = GetBoolValue(line, "show_stats");
			}
			else if (StartsWith(line, "show_info"))
			{
				Settings.ShowInfo = GetBoolValue(line, "show_info");
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

void FindInDirectory(char* Dir)
{
	char* currentDir = StringConcat(Dir, "\\");

	char* findDir = StringConcat(currentDir, "*");
	WIN32_FIND_DATA CurrentFileData;
	HANDLE FindHandle = FindFirstFileEx(findDir, FindExInfoBasic, &CurrentFileData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	while (FindNextFile(FindHandle, &CurrentFileData))
	{
		if ((CurrentFileData.cFileName[0] == '.') && (CurrentFileData.cFileName[1] == '.'))
		{
			continue;
		}
		char FilePathStorage[MAX_LINE_BUFFER_LENGTH];
		char* filePath = StringConcat(currentDir, CurrentFileData.cFileName, FilePathStorage);

		if (CurrentFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			PushFolder(&FStack, filePath);
		}
		else
		{
			if (CurrentFileData.nFileSizeLow < 5000000)
			{
				//TODO(Alin) StringCopy should be in a pool for faster times
				ProcessFile(StringCopy(CurrentFileData.cFileName), StringCopy(filePath), CurrentFileData.nFileSizeLow);
			}
		}
	}
	delete[] currentDir;
	delete[] findDir;
	FindClose(FindHandle);
}

void FindAllFiles()
{
	PushFolder(&FStack, StartingWorkingDir);
	
	do
	{
		char* Dir = PopFolder(&FStack);
		FindInDirectory(Dir);
	} while (FStack.Size > 0);
}

void ProcessFile(char* fileName, char* filePath, unsigned fileSize)
{
	bool ShouldProcess = true;
	if (Settings.FilesToInclude.Size) ShouldProcess = false;

	for (int i = 0; i < Settings.FilesToInclude.Size; i++)
	{
		char* FileSugested = Settings.FilesToInclude.Strings[i];
		char TempSugestedName[MAX_LINE_BUFFER_LENGTH];
		char TempFileName[MAX_FILE_NAME_LENGTH];
		strcpy_s(TempSugestedName, FileSugested);
		strcpy_s(TempFileName, fileName);

		char* LoweredTempSug = ToLower(TempSugestedName);
		char* LoweredTempFile = ToLower(TempFileName);
		//Handle wild card
		int preWildCardIndex = FindString(TempSugestedName, "*.");
		int postWildCardIndex = FindString(TempSugestedName, ".*");
		if ((preWildCardIndex == 0) && (postWildCardIndex > 0))
		{
			ShouldProcess = true;
			break;
		}
		else if (preWildCardIndex == 0)
		{
			if (FindString(LoweredTempFile, TempSugestedName + 1) >= 0)
			{
				ShouldProcess = true;
				break;
			}
		}
		else if (postWildCardIndex > 0)
		{
			char* beforeWildCard = Substring(TempSugestedName, nullptr, 0, StringSize(TempSugestedName) - 1);
			int foundIndex = FindString(LoweredTempFile, beforeWildCard);
			if (foundIndex >= 0)
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
		char TempSugestedName[MAX_LINE_BUFFER_LENGTH];
		char TempFileName[MAX_LINE_BUFFER_LENGTH];
		strcpy_s(TempSugestedName, FileSugested);
		strcpy_s(TempFileName, fileName);

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
			char* beforeWildCard = Substring(TempSugestedName, nullptr, 0, StringSize(TempSugestedName) - 1);
			int foundIndex = FindString(LoweredTempFile, beforeWildCard);
			if (foundIndex >= 0)
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
		Files.push_back({ fileName, filePath, fileSize });
	}
}

