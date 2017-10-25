

#include "Utils.h"
#include "AppState.h"
#include "Timer.h"

//Declarations
void parseProject();
void analyzeCstyleFile(char* fileName, int64 fileSize, SupportedFileType fileType);
void analyzeFileByType(char* fileName, int64 fileSize, SupportedFileType fileType);
void analyzeBatchFile(char* fileName, int64 fileSize, SupportedFileType fileType);
//Globals



int main(int argc, char* argv[])
{
	AppState& app = AppState::getInstance();
	app.setOptions(argc, argv);
	app.activateOptions();
#if NDEBUG
	char currentDir[256];
	GetCurrentDirectoryA(256, currentDir);
#else
	char* currentDir = "D:\\workspace\\Shady\\shady-core";
	SetCurrentDirectoryA(currentDir);
#endif
	Timer timer{};
	char tempDir[512];
	strcpy(tempDir, currentDir);
	WIN32_FIND_DATAA findData = {};
	HANDLE findHandle = FindFirstFileA(strcat(tempDir, "\\*"), &findData);
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		timer.start();
		parseProject();
		
		int64 elapsedTime = timer.getElapsedTime();
		ConsoleColor timeColor = GREEN;
		if (elapsedTime < 500)
		{
			timeColor = GREEN;
		}
		else if (elapsedTime < 1000)
		{
			timeColor = YELLOW;
		}
		else
		{
			timeColor = RED;
		}
		consoleColoredOutput(PURPLE, "Time spent parsing project: ");
		consoleColoredOutput(timeColor, "%lld ms\n", elapsedTime);
		FindClose(findHandle);
	}
	app.exFiles.printIgnoredFiles();
	
	app.proj.printFormatedInfo();
	app.wait();
	return 0;
}


void parseProject()
{
	AppState& app = AppState::getInstance();
	char dirName[215];
	GetCurrentDirectoryA(215, dirName);
	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(strcat(dirName, "\\*"), &findData);

	while (FindNextFileA(findHandle, &findData))
	{
		if (findData.cFileName[0] == '.') continue;
		bool shouldExclude = false;
		for (int excludeIndex = 0; excludeIndex < app.exFiles.numOfFilesToExclude; excludeIndex++)
		{
			if (strCompare(findData.cFileName, app.exFiles.excludeFiles[excludeIndex]))
			{
				app.exFiles.excludedFilesIndices[app.exFiles.numOfFilesExcluded++] = excludeIndex;
				
				shouldExclude = true;
			}
		}
		if (shouldExclude) continue;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			SetCurrentDirectoryA(findData.cFileName);
			parseProject();
		}
		else
		{
			// TODO these count files as parsed before entering the parse function
			//wich may fail..

			LARGE_INTEGER size = {};
			size.LowPart = findData.nFileSizeLow;
			size.HighPart = findData.nFileSizeHigh;
			SupportedFileType fileType = SupportedFileType::INVALID;
			if (endsWith(findData.cFileName, ".cpp"))
			{
				fileType = SupportedFileType::CPP;
			}
			else if (endsWith(findData.cFileName, ".h"))
			{
				fileType = SupportedFileType::H;
			}
			else if (endsWith(findData.cFileName, ".hpp"))
			{
				fileType = SupportedFileType::HPP;
			}
			else if (endsWith(findData.cFileName, ".c"))
			{
				fileType = SupportedFileType::C;
			}
			else if (endsWith(findData.cFileName, ".java"))
			{
				fileType = SupportedFileType::JAVAFILE;
			}
			else if (endsWith(findData.cFileName, ".frag"))
			{
				fileType = SupportedFileType::FRAG;
			}
			else if (endsWith(findData.cFileName, ".vert"))
			{
				fileType = SupportedFileType::VERT;
			}
			else if (endsWith(findData.cFileName, ".bat"))
			{
				fileType = SupportedFileType::BAT;
			}
			else if (endsWith(findData.cFileName, ".cmd"))
			{
				fileType = SupportedFileType::CMD;
			}
			if (fileType == SupportedFileType::INVALID)
			{
				//if (!app.quietMode.active) consoleColoredOutput(RED, "File %s not supported\n", findData.cFileName);
				continue;
			}
			//char buffer[MAX_PATH];
			//GetFullPathNameA(findData.cFileName, MAX_PATH, buffer, NULL);
			//consoleColoredOutput(YELLOW, "%s\n", buffer);
			app.proj.incrementSupFile(fileType);
			analyzeFileByType(findData.cFileName, (int64)size.QuadPart, fileType);
		}

	}
	SetCurrentDirectoryA("..");
	FindClose(findHandle);
}

void analyzeFileByType(char* fileName, int64 fileSize, SupportedFileType fileType)
{
	switch (fileType)
	{
	case SupportedFileType::CPP:
	case SupportedFileType::C:
	case SupportedFileType::HPP:
	case SupportedFileType::H:
	case SupportedFileType::JAVAFILE:
	case SupportedFileType::VERT:
	case SupportedFileType::FRAG:
		analyzeCstyleFile(fileName, fileSize, fileType);
		break;
	case SupportedFileType::BAT:
	case SupportedFileType::CMD:
		analyzeBatchFile(fileName, fileSize, fileType);
		break;
	}
}

void analyzeBatchFile(char* fileName, int64 fileSize, SupportedFileType fileType)
{
	AppState& app = AppState::getInstance();
	if (!app.quietMode)
	{
		consoleColoredOutput(WHITE, "Parsing ");
		consoleColoredOutput(YELLOW, fileName);
		consoleColoredOutput(WHITE, " ... ");
	}
	HANDLE fHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fHandle == INVALID_HANDLE_VALUE)
	{
		if (!app.quietMode) consoleColoredOutput(RED, "FAILED!\n");
		return;
	}
	char* contents = new char[(uint32)fileSize];
	char lineContents[MAX_LINE_CHARS];
	if (!ReadFile(fHandle, contents, (DWORD)fileSize, NULL, NULL))
	{
		if (!app.quietMode) consoleColoredOutput(RED, "FAILED!\n");
		return;
	}
	char* tempCont = contents;
	
	while (*tempCont)
	{
		char* line = lineContents;
		bool lineIsComment = false;
		int chars = getLine(line, 512, tempCont);
		if (chars) app.proj.incrementTotalLines(fileType);
		if (lineEmpty(line))
		{
			app.proj.incrementBlankLines(fileType);
			tempCont += chars;
			continue;
		}
		else
		{
			bool blank = false;
			for (int i = 0; i < chars; i++)
			{
				if (line[i] == '\0') break;
				if ((line[i] == ' ') || (line[i] == '\t'))
				{
					blank = true;
				}
				else
				{
					if ((line[i] == ':') && (line[i + 1] == ':') && (blank || (i == 0)))
					{
						app.proj.incrementLinesOfComments(fileType);
						break;
					}
					else if (((line[i] == 'R') || (line[i] == 'r')) &&
						((line[i + 1] == 'E') || (line[i + 1] == 'e')) &&
						((line[i + 2] == 'M') || (line[i + 2] == 'm')) &&
						 (blank || (i == 0)))
					{
						app.proj.incrementLinesOfComments(fileType);
						break;
					}
					else
					{
						app.proj.incrementLinesOfCode(fileType);
						break;
					}
					blank = false;
				}
				
			}
		}
		
		tempCont += chars;
	}
	if (!app.quietMode) consoleColoredOutput(GREEN, "DONE!\n");
	delete[] contents;
	CloseHandle(fHandle);
}


void analyzeCstyleFile(char* fileName, int64 fileSize, SupportedFileType fileType)
{
	AppState& app = AppState::getInstance();
	if (!app.quietMode)
	{
		consoleColoredOutput(WHITE, "Parsing ");
		consoleColoredOutput(YELLOW, fileName);
		consoleColoredOutput(WHITE, " ... ");
	}
	HANDLE fHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fHandle == INVALID_HANDLE_VALUE)
	{
		if (!app.quietMode) consoleColoredOutput(RED, "FAILED!\n");
		return;
	}
	char* contents = new char[(uint32)fileSize];
	char lineContents[MAX_LINE_CHARS];
	if (!ReadFile(fHandle, contents, (DWORD)fileSize, NULL, NULL))
	{
		if (!app.quietMode) consoleColoredOutput(RED, "FAILED!\n");
		return;
	}
	char* tempCont = contents;
	bool isMultiLineComment = false;
	int multiCommentLines = 0;
	while (*tempCont)
	{
		char* line = lineContents;
		bool lineIsComment = false;
		int chars = getLine(line, 512, tempCont);
		if (chars) app.proj.incrementTotalLines(fileType);
		if (isMultiLineComment) multiCommentLines++;
		if ((chars == 1) || (chars == 2))
		{
			app.proj.incrementBlankLines(fileType);
		}
		else
		{
			bool blank = false;
			for (int i = 0; i < chars; i++)
			{
				if (!isMultiLineComment)
				{
					if (line[i] == '\0') break;
					if ((line[i] == ' ') || (line[i] == '\t') || (line[i] == ';'))
					{
						blank = true;
					}
					else
					{
						if ((line[i] == '/') && (line[i + 1] == '/'))
						{
							app.proj.incrementLinesOfComments(fileType);
							lineIsComment = true;
						}
						if ((line[i] == '/') && (line[i + 1] == '*'))
						{
							isMultiLineComment = true;
							multiCommentLines++;
						}
						blank = false;
						bool isJustBracket = false;
						if ((line[i] == '{') || (line[i] == '}'))
						{
							int num = lineWithComments(line + i + 1);
							bool empty = lineEmpty(line + i + 1);
							
							if((num > 0) || empty)
							{
								isJustBracket = true;
								app.proj.incrementBracketLines(fileType);
								if (num == 2)
								{
									isMultiLineComment = true;
								}
								break;
								
							}
						}
						if (!isMultiLineComment && !lineIsComment && !isJustBracket)
						{
							app.proj.incrementLinesOfCode(fileType);
							break;
						}
					}
				}
				else
				{
					if ((line[i] == '*') && (line[i + 1] == '/'))
					{
						isMultiLineComment = false;

						bool codePresent = false;
						for (int j = i + 2; j < chars; j++)
						{
							if (line[j] == '\0') break;
							if ((line[j] != ' ') && (line[j] != '\t'))
							{
								if ((line[j] == '{') || (line[j] == '}'))
								{
									if (lineWithComments(line + i + 1) ||
										lineEmpty(line + i + 1))
									{
										app.proj.incrementBracketLines(fileType);
									}
								}
								else
								{
									codePresent = true;
								}
							}
						}

						if (codePresent)
						{
							multiCommentLines -= 1;
							app.proj.incrementLinesOfCode(fileType);
						}

						app.proj.incrementLinesOfComments(fileType, multiCommentLines);
						multiCommentLines = 0;
						break;
					}
				}
			}
			if (blank) app.proj.incrementBlankLines(fileType);
		} // while

		tempCont += chars;
	}
	if (!app.quietMode) consoleColoredOutput(GREEN, "DONE!\n");
	delete[] contents;
	CloseHandle(fHandle);
}