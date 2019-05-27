#pragma once

#include "Helpers.h"


extern HANDLE Console;

void FindInDirectory(char* Dir);
void SearchFiles();
void SearchFilesRange(FilesIndexRange range);
void FindAllFiles();
void ProcessFile(char* fileName, char* filePath, unsigned fileSize);
