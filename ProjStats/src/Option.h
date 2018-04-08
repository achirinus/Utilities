#pragma once
#include "Utils.h"
#include <cstring>
typedef void (*pVoidFunc)();



struct Option
{
	char mName[6];
	char mHelp[128];
	bool mActive;
	bool mNeedsArg;
	bool mHasArg;
	pVoidFunc pFunc;
	char* mArgs[10];
	int mNumOfArgs;

	Option(const char* name, 
		const char* help = "Not specified", 
		bool hasArg = false, 
		pVoidFunc func = nullptr): mNeedsArg(hasArg), 
									mHasArg(false),
									mActive(false), mNumOfArgs(0)
	{
		strcpy_s(mName, 6, name);
		strcpy_s(mHelp, 128, help);
		pFunc = func;
	}

	bool operator==(const char* str)
	{
		return strCompareNoCase(mName, str);
	}

	void run()
	{
		if (mActive && pFunc)
		{
			(*pFunc)();
		}
	}

	void check(char* str)
	{
		
		if (strstr(str, mName) == str)
		{
			int nameOffset = strlen(mName);
			char* argBeginning = str + nameOffset;
			if (mNeedsArg)
			{
				if (argBeginning == '\0')
				{
					consoleColoredOutput(ConsoleColor::YELLOW, "Option %s specified but no valid argument found! Use /? for info\n", mName);
					mHasArg = false;
					mActive = false;
				}
				else
				{
					mHasArg = true;
					mActive = true;
					getArgs(argBeginning);
				}
			}
			else
			{
				mActive = true;
				if (argBeginning != '\0')
				{
					mHasArg = true;
					getArgs(argBeginning);
				}
			}
		}
	}

	void getArgs(char* argList)
	{
		int len = strlen(argList);

		//Has only one arg
		char* commaPos = strchr(argList, ',');
		if (!commaPos)
		{
			char* temp = new char[len + 1];
			strcpy_s(temp, len+1, argList);
			mArgs[mNumOfArgs++] = temp;
		}
		else
		{
			char* tempArgs = argList;
			do {
				int size = commaPos - tempArgs;
				char* temp = new char[size + 1];
				strncpy_s(temp, size + 1, tempArgs, size);
				temp[size + 1] = '\0';
				mArgs[mNumOfArgs++] = temp;
				tempArgs = commaPos + 1;
				commaPos = strchr(tempArgs, ',');

			} while (commaPos);
		}
	}

	void showHelp()
	{
		consoleColoredOutput(ConsoleColor::GREEN, "%s - %s\n", mName, mHelp);
	}
	
	
};
