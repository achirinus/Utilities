#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>


#define FORMATED_LINE_WIDTH 35

#define MAX_LINE_CHARS 512
enum ConsoleColor
{
	RED,
	GREEN,
	BLUE,
	YELLOW,
	CYAN,
	DEFAULT,
	WHITE,
	BLACK,
	PURPLE
};

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef int32_t int32;
typedef int64_t int64;
typedef int8_t int8;
typedef int16_t int16;

int absolute(int a);
bool strCompareNoCase(const char* str1, const char* str2);
bool strCompare(const char* str1, const char* str2);
uint32 strLength(const char *str);
int getLine(char* buffer, int bufferLength, char* str);
bool endsWith(char* str, char* end);
void consoleColoredOutput(ConsoleColor color, const char* str, ...);
void boxedColorPrintf(const char* str, ...);
void boxedColorPrintf(int lineWidth, const char* str, ...);
void changeConsoleColor(ConsoleColor color);

int lineWithComments(char* line);
bool lineEmpty(char* line);
#endif