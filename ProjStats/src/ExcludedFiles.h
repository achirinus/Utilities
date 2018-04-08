#pragma once

#define MAX_EXCLUDED_FILES 3


struct ExcludedFiles
{
	char* excludeFiles[MAX_EXCLUDED_FILES];
	int numOfFilesToExclude;
	int excludedFilesIndices[MAX_EXCLUDED_FILES];
	int numOfFilesExcluded;

	ExcludedFiles()
	{
		numOfFilesExcluded = 0;
		numOfFilesToExclude = 0;
		for (int i = 0; i < MAX_EXCLUDED_FILES; i++)
		{
			excludedFilesIndices[i] = -1;
		}
	}
	int getIgnoredFiles() { return numOfFilesExcluded - numOfFilesToExclude; }
	void printIgnoredFiles()
	{
		for (int i = 0; i < numOfFilesToExclude; i++)
		{
			bool found = false;
			for (int j = 0; j < MAX_EXCLUDED_FILES; j++)
			{
				if (i == excludedFilesIndices[j]) found = true;
			}
			if (!found) consoleColoredOutput(RED,"\"%s\" was not excluded!\n", excludeFiles[i]);
		}
	}
	void addExcludeFile(char* fileName)
	{
		excludeFiles[numOfFilesToExclude++] = fileName;
	}
};
