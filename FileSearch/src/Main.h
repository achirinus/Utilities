#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <ctype.h>
#include <cstdio>


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

struct ProgramSettings
{
	char* SearchTerm;
	StringBuffer FilesToInclude;
	StringBuffer FilesToExclude;
	int NumberOfThreads;
	int OutputLineLength;
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

int StringSize(const char* str)
{
	if (!str) return 0;
	int result = 0;
	while (*str++) result++;
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
	int sourceLen = StringSize(source);
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

char* StringCopy(const char* from)
{
	char* result = 0;
	if (!from) return result;
	int size = StringSize(from);
	if (!size) return result;
	char* dest = new char[size];
	result = dest;
	while (*dest++ = *from++);
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

	char* tempResult = result;
	int copied = StringCopy(first, tempResult);
	tempResult += copied;
	int secondCopied = StringCopy(second, tempResult);
	tempResult += secondCopied;
	*tempResult = 0;
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

	int absPathSize = StringSize(absPath);
	int cwdPathSize = StringSize(cwd);

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

char* CopyString(char* str)
{
	if (!str) return 0;
	int strSize = StringSize(str);
	if (!strSize) return 0;
	char* result = new char[strSize + 1];
	char* tempResult = result;
	while (*tempResult++ = *str++);
	return result;
}


StringBuffer BreakStringByToken(char* str, char token)
{
	StringBuffer result = {};
	if (!str) return result;
	char* tempStr = str;
	int lastTokenIndex = -1;
	int index = 0;
	bool tokenExists = false;
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
			else
			{
				result.Strings[result.Size++] = Substring(tempStr, 0, index);
			}
			lastTokenIndex = index;
			tokenExists = true;
		}
		index++;
	}
	if (tokenExists && (index > lastTokenIndex))
	{
		result.Strings[result.Size++] = Substring(tempStr, lastTokenIndex + 1, index - lastTokenIndex - 1);
	}
	else
	{
		result.Strings[result.Size++] = CopyString(str);
	}
	return result;
}


char* ReadStringLine(char** str)
{
	if (!str) return 0;
	if (!*str)return 0;
	char* tempStr = *str;
	int finalIndex = 0;
	for (int index = 0;; index++)
	{
		char At = tempStr[index];
		if (At == '\r' || At == '\n' || At == 0)
		{
			finalIndex = index;
			char* nextLine = tempStr + index;
			while (*nextLine && ((*nextLine == '\r') || (*nextLine == '\n')))
			{
				nextLine++;
			}
			*str = nextLine;
			break;
		}
	}
	char* result = 0;
	if(finalIndex) result = Substring(tempStr, 0, finalIndex);
	return result;
}

bool StartsWith(char* baseStr, char* str)
{
	if (!baseStr) return false;
	if (!str) return false;
	while (*str)
	{
		if (*str++ != *baseStr++) return false;
	}
	return true;
}

char* SkipString(char* baseStr, char* str)
{
	if (!baseStr) return 0;
	if (!str) return 0;
	while (*str)
	{
		if (*str++ != *baseStr++) return 0;
	}
	return baseStr;
}

bool IsDigit(char c)
{
	return (c > 47) && (c < 58);
}

int ToDigit(char c)
{
	return c - 48;
}

int ToPower(int base, int pow)
{
	if (pow == 0) return 1;
	while (pow > 1)
	{
		base *= base;
		pow--;
	}
	return base;
}

int StringToInt(char* str)
{
	int result = 0;
	if (!str) return result;
	char digits[10];
	int index = 0;
	while (*str && IsDigit(*str))
	{
		digits[index++] = *str++;
		if (index == 9) break;
	}
	for (int i = 0; i < index; i++)
	{
		int pow = index - i - 1;
		result += ToPower(10, pow) * ToDigit(digits[i]);
	}
	return result;
}

bool StringCompare(char* first, char* second)
{
	if (!first) return false;
	if (!second) return false;
	while (*first || *second)
	{
		if (*first++ != *second++) return false;
	}
	return true;
}

char* GetExePath()
{
	char* result = new char[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	return result;
}

void GetAllFilesInDir();
void SearchFiles();
void ReadProgramProperties(char* argv[], int argc);
