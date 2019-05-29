#pragma once

#include "Helpers.h"


extern HANDLE Console;
extern std::mutex SettingsMutex;
extern std::mutex FilesMutex;
extern std::mutex OutputMutex;

struct SearchResult
{
	int LineNumber;
	char* FirstPart;
	char* ThirdPart;
};
typedef std::vector<SearchResult> ResultVector;
extern std::map<char*, ResultVector> SearchResults;

void FindInDirectory(char* Dir);
void SearchFiles();
void SearchFilesRange(FilesIndexRange range);
void FindAllFiles();
void ProcessFile(char* fileName, char* filePath, unsigned fileSize);
