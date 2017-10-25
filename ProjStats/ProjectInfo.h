#pragma once

#include "Utils.h"

struct AppState;

enum SupportedFileType
{
	CPP,
	C,
	H,
	HPP,
	FRAG,
	VERT,
	JAVAFILE,
	BAT,
	CMD,
	NUM_OF_SUPPORTED_FILE_TYPES,
	INVALID = -1

};

enum SupportedLanguage
{
	C_CPP,
	JAVA,
	SHADER,
	BATCH,
	NUM_OF_SUPPORTED_LANGS
};

struct LanguageStats
{
	SupportedLanguage language;
	uint32 numOfFiles[NUM_OF_SUPPORTED_FILE_TYPES];
	uint32 totalLines;
	uint32 linesOfCode;
	uint32 linesOfComments;
	uint32 blankLines;
	uint32 bracketLines;
};

struct ProjectInfo
{
	uint32 totalLines;
	uint32 linesOfCode;
	uint32 linesOfComments;
	uint32 blankLines;
	uint32 bracketLines;
	uint32 supportedFiles[NUM_OF_SUPPORTED_FILE_TYPES];
	uint32 totalFiles;
	LanguageStats languages[NUM_OF_SUPPORTED_LANGS];

	void incrementTotalLines(SupportedFileType fileType, int value = 1)
	{
		switch (fileType)
		{
		case SupportedFileType::CPP:
		case SupportedFileType::C:
		case SupportedFileType::H:
		case SupportedFileType::HPP:
			languages[SupportedLanguage::C_CPP].totalLines += value;
			totalLines += value;
			break;
		case SupportedFileType::JAVAFILE:
			languages[SupportedLanguage::JAVA].totalLines += value;
			totalLines += value;
			break;
		case SupportedFileType::FRAG:
		case SupportedFileType::VERT:
			languages[SupportedLanguage::SHADER].totalLines += value;
			totalLines += value;
			break;
		case SupportedFileType::BAT:
		case SupportedFileType::CMD:
			languages[SupportedLanguage::BATCH].totalLines += value;
			totalLines += value;
			break;
		
		}
	}
	void incrementLinesOfCode(SupportedFileType fileType, int value = 1)
	{
		switch (fileType)
		{
		case SupportedFileType::CPP:
		case SupportedFileType::C:
		case SupportedFileType::H:
		case SupportedFileType::HPP:
			languages[SupportedLanguage::C_CPP].linesOfCode += value;
			linesOfCode += value;
			break;
		case SupportedFileType::JAVAFILE:
			languages[SupportedLanguage::JAVA].linesOfCode += value;
			linesOfCode++;
			break;
		case SupportedFileType::VERT:
		case SupportedFileType::FRAG:
			languages[SupportedLanguage::SHADER].linesOfCode += value;
			linesOfCode += value;
			break;
		case SupportedFileType::BAT:
		case SupportedFileType::CMD:
			languages[SupportedLanguage::BATCH].linesOfCode += value;
			linesOfCode += value;
			break;
		}
	}
	void incrementLinesOfComments(SupportedFileType fileType, int value = 1)
	{
		switch (fileType)
		{
		case SupportedFileType::CPP:
		case SupportedFileType::C:
		case SupportedFileType::H:
		case SupportedFileType::HPP:
			languages[SupportedLanguage::C_CPP].linesOfComments += value;
			linesOfComments += value;
			break;
		case SupportedFileType::JAVAFILE:
			languages[SupportedLanguage::JAVA].linesOfComments += value;
			linesOfComments++;
			break;
		case SupportedFileType::VERT:
		case SupportedFileType::FRAG:
			languages[SupportedLanguage::SHADER].linesOfComments += value;
			linesOfComments += value;
			break;
		case SupportedFileType::BAT:
		case SupportedFileType::CMD:
			languages[SupportedLanguage::BATCH].linesOfComments += value;
			linesOfComments += value;
			break;
		}
	}
	void incrementBlankLines(SupportedFileType fileType, int value = 1)
	{
		switch (fileType)
		{
		case SupportedFileType::CPP:
		case SupportedFileType::C:
		case SupportedFileType::H:
		case SupportedFileType::HPP:
			languages[SupportedLanguage::C_CPP].blankLines += value;
			blankLines += value;
			break;
		case SupportedFileType::JAVAFILE:
			languages[SupportedLanguage::JAVA].blankLines += value;
			blankLines++;
			break;
		case SupportedFileType::FRAG:
		case SupportedFileType::VERT:
			languages[SupportedLanguage::SHADER].blankLines += value;
			blankLines += value;
			break;
		case SupportedFileType::BAT:
		case SupportedFileType::CMD:
			languages[SupportedLanguage::BATCH].blankLines += value;
			blankLines += value;
			break;
		}
	}
	void incrementBracketLines(SupportedFileType fileType, int value = 1)
	{
		switch (fileType)
		{
		case SupportedFileType::CPP:
		case SupportedFileType::C:
		case SupportedFileType::H:
		case SupportedFileType::HPP:
			languages[SupportedLanguage::C_CPP].bracketLines += value;
			bracketLines += value;
			break;
		case SupportedFileType::JAVAFILE:
			languages[SupportedLanguage::JAVA].bracketLines += value;
			bracketLines++;
			break;
		case SupportedFileType::FRAG:
		case SupportedFileType::VERT:
			languages[SupportedLanguage::SHADER].bracketLines += value;
			bracketLines += value;
			break;
		case SupportedFileType::BAT:
		case SupportedFileType::CMD:
			languages[SupportedLanguage::BATCH].bracketLines += value;
			bracketLines += value;
			break;
		}
	}
	void incrementSupFile(SupportedFileType type)
	{
		switch (type)
		{
		case SupportedFileType::CPP:
			supportedFiles[SupportedFileType::CPP]++;
			languages[SupportedLanguage::C_CPP].numOfFiles[SupportedFileType::CPP]++;
			totalFiles++;
			break;
		case SupportedFileType::C:
			supportedFiles[SupportedFileType::C]++;
			languages[SupportedLanguage::C_CPP].numOfFiles[SupportedFileType::C]++;
			totalFiles++;
			break;
		case SupportedFileType::H:
			languages[SupportedLanguage::C_CPP].numOfFiles[SupportedFileType::H]++;
			supportedFiles[SupportedFileType::H]++;
			totalFiles++;
			break;
		case SupportedFileType::HPP:
			supportedFiles[SupportedFileType::HPP]++;
			languages[SupportedLanguage::C_CPP].numOfFiles[SupportedFileType::HPP]++;
			totalFiles++;
			break;
		case SupportedFileType::FRAG:
			supportedFiles[SupportedFileType::FRAG]++;
			languages[SupportedLanguage::SHADER].numOfFiles[SupportedFileType::FRAG]++;
			totalFiles++;
			break;
		case SupportedFileType::VERT:
			supportedFiles[SupportedFileType::VERT]++;
			languages[SupportedLanguage::SHADER].numOfFiles[SupportedFileType::VERT]++;
			totalFiles++;
			break;
		case SupportedFileType::JAVAFILE:
			supportedFiles[SupportedFileType::JAVAFILE]++;
			languages[SupportedLanguage::JAVA].numOfFiles[SupportedFileType::JAVAFILE]++;
			totalFiles++;
			break;
		case SupportedFileType::BAT:
			supportedFiles[SupportedFileType::BAT]++;
			languages[SupportedLanguage::BATCH].numOfFiles[SupportedFileType::BAT]++;
			totalFiles++;
			break;
		case SupportedFileType::CMD:
			supportedFiles[SupportedFileType::CMD]++;
			languages[SupportedLanguage::BATCH].numOfFiles[SupportedFileType::CMD]++;
			totalFiles++;
			break;
		}
	}
	void printRectLine(int num)
	{
		for (int i = 0; i < num; i++)
		{
			consoleColoredOutput(RED, "-");
		}
		printf("\n");
	}

	void printFormatedInfo()
	{
		
		printFormattedPortrait();
		
	}

	//TODO finish this
	void printFormattedInfoLandscape()
	{
		int numOfLangs = 0;
		for (int i = 0; i < NUM_OF_SUPPORTED_LANGS; i++)
		{
			if (languages[i].totalLines > 0) numOfLangs++;
		}
		int COLUMN_WIDTH = 30;
		int LINE_WIDTH = COLUMN_WIDTH + COLUMN_WIDTH * numOfLangs;
		printRectLine(LINE_WIDTH);
		char* line = new char[LINE_WIDTH + 1];
		char* tempLine = line;
		char* column = new char[COLUMN_WIDTH + 1];
		sprintf(column, "  ");
		
		printRectLine(FORMATED_LINE_WIDTH);
	}

	void printFormattedPortrait()
	{
		printRectLine(FORMATED_LINE_WIDTH);
		boxedColorPrintf("$pFiles parsed: $y%d", totalFiles);
		if (supportedFiles[SupportedFileType::CPP] > 0)
		{
			boxedColorPrintf("    $c.cpp files: $y%d", supportedFiles[SupportedFileType::CPP]);
		}
		if (supportedFiles[SupportedFileType::C] > 0)
		{
			boxedColorPrintf("    $c.c files: $y%d", supportedFiles[SupportedFileType::C]);
		}
		if (supportedFiles[SupportedFileType::H] > 0)
		{
			boxedColorPrintf("    $c.h files: $y%d", supportedFiles[SupportedFileType::H]);
		}
		if (supportedFiles[SupportedFileType::HPP] > 0)
		{
			boxedColorPrintf("    $c.hpp files: $y%d", supportedFiles[SupportedFileType::HPP]);
		}
		if (supportedFiles[SupportedFileType::VERT] > 0)
		{
			boxedColorPrintf("    $c.vert files: $y%d", supportedFiles[SupportedFileType::VERT]);
		}
		if (supportedFiles[SupportedFileType::FRAG] > 0)
		{
			boxedColorPrintf("    $c.frag files: $y%d", supportedFiles[SupportedFileType::FRAG]);
		}
		if (supportedFiles[SupportedFileType::JAVAFILE] > 0)
		{
			boxedColorPrintf("    $c.java files: $y%d", supportedFiles[SupportedFileType::JAVAFILE]);
		}
		if (supportedFiles[SupportedFileType::BAT] > 0)
		{
			boxedColorPrintf("    $c.bat files: $y%d", supportedFiles[SupportedFileType::BAT]);
		}
		if (supportedFiles[SupportedFileType::CMD] > 0)
		{
			boxedColorPrintf("    $c.cmd files: $y%d", supportedFiles[SupportedFileType::CMD]);
		}
		boxedColorPrintf("$pLanguages used:");
		for (int i = 0; i < NUM_OF_SUPPORTED_LANGS; i++)
		{
			if (languages[i].totalLines > 0)
			{
				switch (i)
				{
				case SupportedLanguage::C_CPP:
					boxedColorPrintf("    $cC\\CPP:");
					break;
				case SupportedLanguage::JAVA:
					boxedColorPrintf("    $cJAVA:");
					break;
				case SupportedLanguage::SHADER:
					boxedColorPrintf("    $cSHADER:");
					break;
				case SupportedLanguage::BATCH:
					boxedColorPrintf("    $cBATCH:");
					break;
				}
				boxedColorPrintf("        $gLines of code: $p%d", languages[i].linesOfCode);
				boxedColorPrintf("        $gLines of comments: $y%d", languages[i].linesOfComments);
				boxedColorPrintf("        $gBracket lines: $y%d", languages[i].bracketLines);
				boxedColorPrintf("        $gBlank lines: $y%d", languages[i].blankLines);
				boxedColorPrintf("        $gTotal lines: $y%d", languages[i].totalLines);

			}
		}
		boxedColorPrintf("$pEntire project stats:");
		boxedColorPrintf("    $gLines of code: $p%d", linesOfCode);
		boxedColorPrintf("    $gLines of comments: $y%d", linesOfComments);
		boxedColorPrintf("    $gBracket lines: $y%d", bracketLines);
		boxedColorPrintf("    $gBlank lines: $y%d", blankLines);
		boxedColorPrintf("    $gTotal lines: $y%d", totalLines);

		printRectLine(FORMATED_LINE_WIDTH);
	}
};


