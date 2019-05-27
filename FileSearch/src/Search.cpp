#include "Search.h"
#include "Helpers.h"

std::mutex OutputMutex;

#define MAX_LINE_BUFFER_LENGTH 512

FolderStack FStack;

void SearchFilesRange(FilesIndexRange range)
{
	for (int i = range.Begin; i <= range.End; i++)
	{
		FileData& names = Files[i];
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
			delete[] Line;
			Line = 0;
			Line = ReadStringLine(&TempCont);
		}

		delete[] Contents;
		fclose(pFile);
	}
}

void SearchFiles()
{
	SearchFilesRange({ 0, (int)Files.size() - 1 });
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
		char TempSugestedName[1024];
		char TempFileName[1024];
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
			char* beforeWildCard = Substring(TempSugestedName, 0, StringSize(TempSugestedName) - 1);
			int foundIndex = FindString(LoweredTempFile, beforeWildCard);
			delete[] beforeWildCard;
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
		char TempSugestedName[1024];
		char TempFileName[1024];
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
			char* beforeWildCard = Substring(TempSugestedName, 0, StringSize(TempSugestedName) - 1);
			int foundIndex = FindString(LoweredTempFile, beforeWildCard);
			delete[] beforeWildCard;
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


void FindInDirectory(char* Dir)
{
	char* currentDir = StringConcat(Dir, "\\");

	char* findDir = StringConcat(currentDir, "*");
	WIN32_FIND_DATA CurrentFileData;
	HANDLE FindHandle = FindFirstFile(findDir, &CurrentFileData);
	while (FindNextFile(FindHandle, &CurrentFileData))
	{
		if ((CurrentFileData.cFileName[0] == '.') && (CurrentFileData.cFileName[1] == '.'))
		{
			continue;
		}

		char* filePath = StringConcat(currentDir, CurrentFileData.cFileName);

		if (CurrentFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			PushFolder(&FStack, filePath);
		}
		else
		{
			ProcessFile(StringCopy(CurrentFileData.cFileName), StringCopy(filePath), CurrentFileData.nFileSizeLow);
		}
		delete[] filePath;
	}
	delete[] currentDir;
	delete[] findDir;
	FindClose(FindHandle);
}