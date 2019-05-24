#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <ctype.h>
#include <cstdio>
#include "StringHelpers.h"

#define MAX_LINE_BUFFER_LENGTH 512
#define MAX_FILE_NAME_LENGTH 128
#define SUPPORTED_OPTIONS 20 //Just make sure this is way bigger than number of options

extern const char* HelpText;

struct FileData
{
	char* FileName;
	char* AbsPath;
	unsigned Size;
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
	bool ShowTimes;
	bool ShowStats;
	bool ShowInfo;
	bool LongFilename;
};

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
void SearchFiles();
void SearchFilesRange(FilesIndexRange range);
void ReadProgramProperties(char* argv[], int argc);
void FindAllFiles();
void ProcessFile(char* fileName, char* filePath, unsigned fileSize);
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


