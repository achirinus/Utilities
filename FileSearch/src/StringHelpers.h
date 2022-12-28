#pragma once
#include "Allocator.h"

struct StringBuffer
{
	char* Strings[50];
	int Size;
};

struct StringPool
{
	char* Data;
	int Cursor;
	int Size;
};

int PoolFreeSpace(StringPool* Pool);

struct CStr
{
	char* Data;
	int Size;
};

CStr AllocString(int Size);
void FreeString(CStr Str);

char ToLower(char c);
char* ToLower(char* str);
int StringSize(const char* str);
bool BeginsWith(const char* str, const char* with);
char* Substring(char* source, Allocator::Arena* pArena = nullptr, int startPos = 0, int count = 0);
int StringCopy(const char* from, char* dest);
char* StringCopy(const char* from);
char* StringConcat(const char* first, const char* second, char* dest = 0);
int FindString(const char* source, const char* strToFind, int startIndex = 0);
int FindLastString(const char* source, const char* strToFind);

//Can work for any type of character, but the first occurence of the char must be in the string
//The type of character is specified by the open and close args
//The canStack param specifies if another pair of chars can be found inside a pair
char* FindMatchingClosingChar(char* str, char open, char close, bool canStack = false);

char* CopyString(char* str);
StringBuffer BreakStringByToken(char* str, char token);
char* ReadStringLine(char** str, Allocator::Arena* pArena = nullptr);
bool StartsWith(char* baseStr, char* str);
char* SkipString(char* baseStr, char* str);
bool IsDigit(char c);
int ToDigit(char c);
int ToPower(int base, int pow);
int StringToInt(char* str);
bool StringCompare(char* first, char* second);
