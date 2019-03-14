
#include "Helpers.h"

long StartTime;

const char* HelpText = 
"Usage: FileSearch -s\"[search_string]\" <options> \n"
"Use [arg,arg,..] only to specify multiple arguments to an option\n"
"Options: \n"
"-t[NUM]	NUM=number of threads to use(default = max possible)\n"
"-e[FILE_NAME] FILE_NAME=name of the file/folder to exclude\n"
"-i[FILE_NAME] FILE_NAME=name of the file/folder to include(will exclude everything else)\n";

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

char* GetLastDirPath(char* src)
{
	int LastSlashIndex = FindLastString(src, "\\");
	return Substring(src, 0, LastSlashIndex);
}

char* GetExePath()
{
	char* result = new char[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	return result;
}

int GetIntValue(char* source, char* var)
{
	char* valueStr = SkipString(source, var);
	valueStr++;
	return StringToInt(valueStr);
}

bool GetBoolValueWithOptions(char* source, char* var, char* trueOpt, char* falseOpt)
{
	char* valueStr = SkipString(source, var);
	valueStr++;
	bool Found1 = FindString(valueStr, trueOpt) >= 0;
	if (Found1)
	{
		return true;
	}

	bool Found2 = FindString(valueStr, falseOpt) >= 0;
	if (Found2)
	{
		return false;
	}
	return false;
}

bool GetBoolValue(char* source, char* var)
{
	return GetBoolValueWithOptions(source, var, "true", "false");
}

