#pragma once
#include "Utils.h"
#include "ProjectInfo.h"
#include "ExcludedFiles.h"
#include "Option.h"

#include <vector>

void displayHelp();
void setExcludedFiles();
void setLayout();
void setQuietMode();
void setWaitForExit();

enum OutputLayout
{
	PORTRAIT = 1,
	LANDSCAPE
};

struct AppState
{
	ProjectInfo proj;
	ExcludedFiles exFiles;
	std::vector<Option> options;
	bool quietMode;
	bool waitBeforeExit;
	OutputLayout layout;

	AppState() : proj{}, exFiles{}, waitBeforeExit(false), quietMode(false), layout(OutputLayout::PORTRAIT)
	{
		options.push_back({ "/E", "Specifies files or folder to exclude", true, setExcludedFiles });
		options.push_back({ "/L", "Specifies the layout of the output: 1-Portrait(default)/2-Landscape.", true, setLayout});
		options.push_back({ "/Q", "Sets silent mode(e.g. won't show files parsed)", false, setQuietMode });
		options.push_back({ "/W", "Waits for input before exiting", false, setWaitForExit});
		options.push_back({ "/?", "Display help", false, displayHelp });

	}

	Option* getOption(const char* str)
	{
		for (auto& option : options)
		{
			if (option == str) return &option;
		}
		return nullptr;
	}

	void activateOptions()
	{
		for (auto& option : options)
		{
			option.run();
		}
	}

	void setOptions(int argc, char* argv[])
	{
		int excludeIndex = 0;
		for (int i = 1; i < argc; i++)
		{
			for (auto& option : options)
			{
				option.check(argv[i]);
			}
		}
	}

	void wait()
	{
		if (waitBeforeExit)
		{
			system("PAUSE");
		}
	}
	static AppState& getInstance()
	{
		if (!sInstance)
		{
			sInstance = new AppState();
		}
		return *sInstance;
	}

private:
	static AppState* sInstance;
};

AppState* AppState::sInstance = nullptr;

void displayHelp()
{
	AppState& app = AppState::getInstance();
	for (auto& option : app.options)
	{
		option.showHelp();
	}
	exit(0);
}

void setQuietMode()
{
	AppState& app = AppState::getInstance();
	app.quietMode = true;
}

void setLayout()
{
	AppState& app = AppState::getInstance();
	Option* option = app.getOption("/L");
	app.layout = (OutputLayout)atoi(option->mArgs[0]);
}

void setExcludedFiles()
{
	AppState& app = AppState::getInstance();
	Option* excludeOption = app.getOption("/E");
	for (int i = 0; i < excludeOption->mNumOfArgs; i++)
	{
		app.exFiles.addExcludeFile(excludeOption->mArgs[i]);
	}
}

void setWaitForExit()
{
	AppState& app = AppState::getInstance();
	app.waitBeforeExit = true;
}

