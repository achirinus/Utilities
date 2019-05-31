#pragma once

#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <map>
#include <ctype.h>
#include <cstdio>
#include <thread>
#include <mutex>
#include "StringHelpers.h"


#define SUPPORTED_OPTIONS 20 //Just make sure this is way bigger than number of options

extern const char* HelpText;

extern char StartingWorkingDir[MAX_PATH];

struct FileData
{
	char* FileName;
	char* AbsPath;
	unsigned Size;
};

extern std::vector<FileData> Files;

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
	char SearchDirectory[MAX_PATH];
	char SearchTerm[MAX_PATH];
	StringBuffer FilesToInclude;
	StringBuffer FilesToExclude;
	int NumberOfThreads;
	int OutputLineLength;
	bool ShowTimes;
	bool ShowStats;
	bool ShowInfo;
	bool LongFilename;
};

extern ProgramSettings Settings;

struct FolderNode
{
	char* Data;
	FolderNode* Next;
};

struct FolderStack
{
	FolderNode* Head;
	int Size;
};

struct FilesIndexRange
{
	int Begin;
	int End;
};

FolderNode* CreateFolderNode(char* name);
void DeleteFolderNode(FolderNode* node);
void PushFolder(FolderStack* queue, char* name);
char* PopFolder(FolderStack* queue);

//Defined in main

void ReadProgramProperties(char* argv[], int argc);

//-----
void BeginCounter();
int EndCounter();
void TerminateError(char* mes, ...);
void ClearOptionBuffer(OptionBuffer* buf);
char* GetRelativePath(const char* cwd, const char* absPath);
char* GetLastDirPath(char* src);
char* GetExePath();
int GetIntValue(char* source, char* var);
bool GetBoolValueWithOptions(char* source, char* var, char* trueOpt, char* falseOpt);
bool GetBoolValue(char* source, char* var);

template<typename T>
T ClampMin(T val, T lowerBound)
{
	T Result = val;
	if (Result < lowerBound) Result = lowerBound;
	return Result;
}

