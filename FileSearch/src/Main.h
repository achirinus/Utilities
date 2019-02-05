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
"Use [arg,arg,..] only to specify multiple arguments to an option\n"
"Options: \n"
"-t[NUM]	NUM=number of threads to use(default = max possible)\n"
"-e[FILE_NAME] FILE_NAME=name of the file/folder to exclude\n"
"-i[FILE_NAME] FILE_NAME=name of the file/folder to include(will exclude everything else)\n";

struct FileData
{
	char* FileName;
	char* AbsPath;
	unsigned Size;
};

struct StringBuffer
{
	char* Strings[50];
	int Size;
};

struct ProgramOption
{
	char Name;
	StringBuffer Args;
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
	//if (count == 0) count = sourceLen - startPos;
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

int StringSize(const char* str)
{
	if (!str) return 0;
	int result = 0;
	while (*str++) result++;
	return result;
}

int StringCopy(const char* from, char* dest)
{
	int result = 0;
	if (!dest || !from) return result;
	char c = *from;
	while (*from)
	{
		*dest++ = *from++;
		result++;
	}
	return result;
}

char* StringConcat(const char* first, const char* second, char* dest = 0)
{
	char* result = 0;
	int firstSize = StringSize(first);
	int secondSize = StringSize(second);

	if (!first && !second) return 0;
	if (!dest)
	{

		result = new char[firstSize + secondSize + 1];
	}
	else
	{
		result = dest;
	}

	

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

char* GetRelativePath(const char* cwd, const char* absPath)
{
	if (!cwd || !absPath) return 0;

	char slash = 0;

	if (FindString(cwd, "/") != -1)
	{
		slash = '/';
	}
	else
	{
		slash = '\\';
	}

	int absPathSize = strlen(absPath);
	int cwdPathSize = strlen(cwd);

	char* result = new char[absPathSize + 1];
	int startingIndex = 0;
	for (; startingIndex < absPathSize; startingIndex++)
	{
		if (absPath[startingIndex] != cwd[startingIndex]) break;
	}

	if (startingIndex != cwdPathSize)
	{
		return 0;
	}

	if (absPath[startingIndex] != slash)
	{
		result[0] = slash;
		strcpy(result + 1, absPath + startingIndex);
	}
	else
	{
		strcpy(result, absPath + startingIndex);
	}
	return result;
}


//Can work for any type of character, but the first occurence of the char must be in the string
//The type of character is specified by the open and close args
//The canStack param specifies if another pair of chars can be found inside a pair
char* FindMatchingClosingChar(char* str, char open, char close, bool canStack = false)
{
	if (!str) return 0;
	int openNum = 0;
	int closeNum = 0;

	while (*str)
	{
		if (canStack)
		{
			if (*str == open)
			{
				openNum++;
			}
			if (*str == close)
			{
				closeNum++;
				if (closeNum == openNum)
				{
					return str;
				}
				else if (closeNum > openNum)
				{
					//TODO(Alin): Maybe error here
					return 0;
				}
			}
		}
		else
		{
			if ((*str == close) && (openNum))
			{
				return str;
			}
			if (*str == open)
			{
				openNum = 1;
			}
		}
		str++;
	}
	return 0;
}

StringBuffer BreakStringByToken(char* str, char token)
{
	StringBuffer result = {};
	if (!str) return result;
	char* tempStr = str;
	int lastTokenIndex = -1;
	int index = 0;
	while (tempStr[index])
	{
		char At = tempStr[index];
		if (At == token)
		{
			if (lastTokenIndex != -1)
			{
				int count = index - lastTokenIndex - 1;

				result.Strings[result.Size++] = Substring(tempStr, lastTokenIndex + 1, count);
			}
			lastTokenIndex = index;
		}
		index++;
	}
	return result;
}

void GetAllFilesInDir();
void SearchFiles();
OptionBuffer ParseCommandLine(char* line);
void ParseOptions(OptionBuffer optionbuffer);

//    C://d/abc
//	  C://d