#include "Utils.h"

bool isLetter(char c)
{
	return ((c > 64) && (c < 91)) || ((c > 96) && (c < 123));
}

int absolute(int a)
{
	if (a < 0)
	{
		return a = -1 * a;
	}
	return a;
}

bool cmpCharsNoCase(char c1, char c2)
{
	if (isLetter(c1) && isLetter(c2))
	{
		int dif = absolute(c1 - c2);
		if ((dif == 32) || (dif == 0)) return true;

		return false;
	}
	else if (c1 == c2) return true;
	return false;;
}

bool strCompare(const char* str1, const char* str2)
{
	if (strLength(str1) != strLength(str2)) return false;
	const char* tempStr1 = str1;
	const char* tempStr2 = str2;

	while (*tempStr1)
	{
		if (*tempStr1 != *tempStr2) return false;
		tempStr1++;
		tempStr2++;
	}
	return true;
}

bool strCompareNoCase(const char* str1, const char* str2)
{

	if (strLength(str1) != strLength(str2)) return false;
	const char* tempStr1 = str1;
	const char* tempStr2 = str2;

	while (*tempStr1)
	{

		if (!cmpCharsNoCase(*tempStr1, *tempStr2)) return false;
		tempStr1++;
		tempStr2++;
	}

	return true;
}

uint32 strLength(const char *str)
{
	uint32 result = 0;
	const char* temp = str;
	while (*temp++) result++;

	return result;
}


//If bufferLength <=0 or buffer null, return the size if the str
int getLine(char* buffer, int bufferLength, char* str)
{
	int result = 0;
	if (bufferLength > 0 && buffer)
	{
		for (int i = 0; i < bufferLength; i++)
		{
			if (!*str) break;
			if ((*str == '\r') && (*(str + 1) == '\n'))
			{
				result += 2;
				break;
			}
			if (*str == '\n')
			{
				result++;
				break;
			}
			*buffer++ = *str++;
			result++;
		}
		*buffer = '\0';
	}
	else
	{
		for (;;)
		{
			if (!*str) break;
			if ((*str == '\r') && (*(str + 1) == '\n'))
			{
				result += 2;
				break;
			}
			if (*str == '\n')
			{
				result++;
				break;
			}
			str++;
			result++;
		}
	}
	return result;
}

bool endsWith(char* str, char* end)
{
	uint32 sizeStr = strLength(str);
	uint32 sizeEnd = strLength(end);
	if (sizeEnd > sizeStr) return false;
	bool result = strCompare((str + (sizeStr - sizeEnd)), end);

	return result;
}

void changeConsoleColor(ConsoleColor color)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	WORD currentColor = consoleInfo.wAttributes;
	switch (color)
	{
	case ConsoleColor::BLUE:
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case ConsoleColor::RED:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case ConsoleColor::GREEN:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case ConsoleColor::CYAN:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN |
										FOREGROUND_INTENSITY |
										FOREGROUND_BLUE);
		break;
	case ConsoleColor::YELLOW:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN |
										FOREGROUND_INTENSITY |
										FOREGROUND_RED);
		break;
	case ConsoleColor::PURPLE:
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE |
										FOREGROUND_INTENSITY |
										FOREGROUND_RED);
		break;
	case ConsoleColor::DEFAULT:
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE |
										FOREGROUND_RED |
										FOREGROUND_GREEN);
		break;
	case ConsoleColor::WHITE:
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE |
										FOREGROUND_INTENSITY |
										FOREGROUND_RED |
										FOREGROUND_GREEN);
		break;
	case ConsoleColor::BLACK:
		SetConsoleTextAttribute(hConsole, 0);
		break;
	}
}

void consoleColoredOutput(ConsoleColor color, const char* str, ...)
{
	va_list args;
	va_start(args, str);
	changeConsoleColor(color);
	vprintf(str, args);
	changeConsoleColor(DEFAULT);
	va_end(args);
}


bool lineEmpty(char* line)
{
	while (*line)
	{
		if ((*line != ' ') &&
			(*line != '\t') &&
			(*line > 0) &&
			(*line < 128)) return false;
		line++;
	}
	return true;
}

int lineWithComments(char* line)
{

	bool isCode = false;
	bool multiLine = false;
	int result = 0;
	while (*line)
	{
		if (!multiLine)
		{
			if ((*line != ' ') && (*line != '\t'))
			{
				if ((*line == '/') && ((*(line + 1) == '/')))
				{
					result = 1;
					break;
				}
				else if ((*line == '/') && ((*(line + 1) == '*')))
				{
					multiLine = true;
					result = 2;
				}
				else
				{
					isCode = true;
					break;
				}
			}
			if ((*line == '/') && ((*(line + 1) == '/')))
			{
				result = 1;
				break;
			}
			else if ((*line == '/') && ((*(line + 1) == '*')))
			{
				multiLine = true;
				result = 2;
			}
		}
		else
		{
			if ((*line == '*') && ((*(line + 1) == '/')))
			{
				result = 1;
				multiLine = false;
			}
		}

		line++;
	}

	return result;
}

int getNumOfMatches(char* str, char match)
{
	int result = 0;
	while (*str)
	{
		if (*str == match) result++;
		str++;
	}
	return result;
}

void boxedColorPrintf(const char* str, ...)
{
	changeConsoleColor(RED);
	printf("|");
	changeConsoleColor(DEFAULT);
	va_list args;
	va_start(args, str);
	size_t sizeOfStr = strlen(str);
	char temp[256];
	char* goodTemp = temp;
	int numOfChars = vsprintf(temp, str, args);
	int numOfColors = getNumOfMatches(temp, '$');
	int paddsNeeded = FORMATED_LINE_WIDTH - (numOfChars - (numOfColors * 2) + 1);
	char finalLine[51];
	sprintf(finalLine, "%*s\n", paddsNeeded, "|");

	char* firstColor = strstr(temp, "$");
	if (firstColor != temp)
	{
		size_t numOfChars = firstColor - temp;
		char* firstStr = new char[numOfChars + 1];
		char* tempFirst = firstStr;
		firstStr = strncpy(firstStr, temp, numOfChars);
		*(tempFirst += numOfChars) = '\0';
		printf(firstStr);
		delete[] firstStr;
	}

	if (numOfColors > 0) goodTemp = strstr(goodTemp, "$");

	do
	{
		goodTemp++;
		switch (*goodTemp)
		{
		case 'r':
		case 'R':
			changeConsoleColor(RED);
			break;
		case 'g':
		case 'G':
			changeConsoleColor(GREEN);
			break;
		case 'y':
		case 'Y':
			changeConsoleColor(YELLOW);
			break;
		case 'c':
		case 'C':
			changeConsoleColor(CYAN);
			break;
		case 'd':
		case 'D':
			changeConsoleColor(DEFAULT);
			break;
		case 'p':
		case 'P':
			changeConsoleColor(PURPLE);
			break;
		}
		goodTemp++;
		char* nextColor = strstr(goodTemp, "$");
		char forPrint[50];
		if (nextColor)
		{
			int charsToCopy = nextColor - goodTemp;
			char* tempForPrint = strncpy(forPrint, goodTemp, charsToCopy);
			*(tempForPrint += charsToCopy) = '\0';
		}
		else
		{
			char* tempForPrint = strcpy(forPrint, goodTemp);
		}
		printf(forPrint);
		goodTemp = nextColor;
		changeConsoleColor(DEFAULT);
	} while (goodTemp);

	changeConsoleColor(RED);
	printf(finalLine);
	changeConsoleColor(DEFAULT);
	va_end(args);

}

void boxedColorPrintf(int lineWidth, const char* str, ...)
{
	changeConsoleColor(RED);
	printf("|");
	changeConsoleColor(DEFAULT);
	va_list args;
	va_start(args, str);
	size_t sizeOfStr = strlen(str);
	char temp[256];
	char* goodTemp = temp;
	int numOfChars = vsprintf(temp, str, args);
	int numOfColors = getNumOfMatches(temp, '$');
	int paddsNeeded = lineWidth - (numOfChars - (numOfColors * 2) + 1);
	char finalLine[51];
	sprintf(finalLine, "%*s\n", paddsNeeded, "|");

	char* firstColor = strstr(temp, "$");
	if (firstColor != temp)
	{
		size_t numOfChars = firstColor - temp;
		char* firstStr = new char[numOfChars + 1];
		char* tempFirst = firstStr;
		firstStr = strncpy(firstStr, temp, numOfChars);
		*(tempFirst += numOfChars) = '\0';
		printf(firstStr);
		delete[] firstStr;
	}

	if (numOfColors > 0) goodTemp = strstr(goodTemp, "$");

	do
	{
		goodTemp++;
		switch (*goodTemp)
		{
		case 'r':
		case 'R':
			changeConsoleColor(RED);
			break;
		case 'g':
		case 'G':
			changeConsoleColor(GREEN);
			break;
		case 'y':
		case 'Y':
			changeConsoleColor(YELLOW);
			break;
		case 'c':
		case 'C':
			changeConsoleColor(CYAN);
			break;
		case 'd':
		case 'D':
			changeConsoleColor(DEFAULT);
			break;
		case 'p':
		case 'P':
			changeConsoleColor(PURPLE);
			break;
		}
		goodTemp++;
		char* nextColor = strstr(goodTemp, "$");
		char forPrint[50];
		if (nextColor)
		{
			int charsToCopy = nextColor - goodTemp;
			char* tempForPrint = strncpy(forPrint, goodTemp, charsToCopy);
			*(tempForPrint += charsToCopy) = '\0';
		}
		else
		{
			char* tempForPrint = strcpy(forPrint, goodTemp);
		}
		printf(forPrint);
		goodTemp = nextColor;
		changeConsoleColor(DEFAULT);
	} while (goodTemp);

	changeConsoleColor(RED);
	printf(finalLine);
	changeConsoleColor(DEFAULT);
	va_end(args);

}

