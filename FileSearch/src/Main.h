#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <ctype.h>


#define MAX_LINE_BUFFER_LENGTH 512

const char* HelpText =
"Usage: FileSearch -s\"[search_string]\" <options> \n"
"Use [] only to specify multiple arguments to an option\n"
"Options: \n"
"-t[NUM]	NUM=number of threads to use(default = max possible)\n"
"-e[FILE_NAME] FILE_NAME=name of the file/folder to exclude\n"
"-i[FILE_NAME] FILE_NAME=name of the file/folder to include(will exclude everything else)\n";

struct FileData
{
	std::string ShortName;
	std::string FullName;
	unsigned Size;
};


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

long StartTime;

void BeginCounter()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	LARGE_INTEGER startCount;
	QueryPerformanceCounter(&startCount);

	StartTime = (long)((startCount.QuadPart * 1000) / freq.QuadPart);
}

int EndCounter()
{
	int result = 0;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	LARGE_INTEGER stopCount;
	QueryPerformanceCounter(&stopCount);
	long EndTime = (long)((stopCount.QuadPart * 1000) / freq.QuadPart);
	result = (int)(EndTime - StartTime);
	return result;
}

bool BeginsWith(const char* str, const char* with)
{
	return strstr(str, with) == str;
}

void GetAllFilesInDir();
void SearchFiles();
void ParseOptions(int num, char* args[]);