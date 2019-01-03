#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <ctype.h>


#define MAX_LINE_BUFFER_LENGTH 512
#define SUPPORTED_OPTIONS 20 //Just make sure this is way bigger than number of options

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


struct ProgramOption
{
	char Name;
	char* Args;
};

struct OptionBuffer
{
	ProgramOption* Buffer;
	int Size;
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

void TerminateError(char* mes, ...)
{
	va_list args;
	va_start(args, mes);

	vprintf(mes, args);
	
	va_end(args);
	exit(1);
}

void ClearOptionBuffer(OptionBuffer* buf)
{
	if (buf && buf->Buffer)
	{
		delete[] buf->Buffer;
	}
}

char* Substring(char* source, int startPos = 0, int count = 0)
{
	char* result = 0;
	int sourceLen = strlen(source);
	if ((startPos < 0) || count < 0) return result;
	if (count == 0) count = sourceLen - startPos;
	if (sourceLen < (startPos + count)) count = sourceLen - startPos;
	
	result = new char[count + 1];
	int i = 0;
	for (; i < count; i++)
	{
		result[i] = source[startPos++];
	}
	result[i] = 0;
	return result;
}

int FindString(const char* source, const char* strToFind, int startIndex = 0)
{
	int result = -1;
	if (!source || !strToFind) return result;

	int index = startIndex;
	source += startIndex;
	char temp = *source;
	while (temp)
	{
		if (temp == *strToFind)
		{
			const char* tempFind = strToFind;
			bool found = false;
			result = index;
			char c = *tempFind++;
			
			while (c)
			{
				if (c != *source)
				{
					result = -1;
					found = false;
					break;
				}
				source++;
				c = *tempFind++;
				found = true;
			}
			if (found) break;
		}
		source++;
		temp = *source;
		index++;
	}
	return result;
}

void GetAllFilesInDir();
void SearchFiles();
OptionBuffer ParseCommandLine(char* line);
void ParseOptions(OptionBuffer optionbuffer);