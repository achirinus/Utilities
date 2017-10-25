#include "Windows.h"
#include "ShPath.h"
#include "cstdio"
#include "vector"

using namespace Slnady;

char ExeDirStr[512];
std::string shcommons = "shady-commons";
std::string shcore = "shady-core";
std::string tmpStr;
std::string commonsStr; 
std::string coreStr; 
std::string PROJECT_BEGIN = "ProjectSection(SolutionItems) = preProject";
std::string PROJECT_END = "EndProjectSection";
std::vector<Path> files;
Path Sln;
Path SlnPath;


size_t strLength(const char *str)
{
	size_t result = 0;
	const char* temp = str;
	while (*temp++) result++;

	return result;
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

bool endsWith(char* str, char* end)
{
	size_t sizeStr = strLength(str);
	size_t sizeEnd = strLength(end);
	if (sizeEnd > sizeStr) return false;
	bool result = strCompare((str + (sizeStr - sizeEnd)), end);

	return result;
}

void getAllFilesForSolution()
{
	char dirName[256];
	GetCurrentDirectoryA(256, dirName);
	strcat_s(dirName, 256, "\\*");
	WIN32_FIND_DATAA comFindData;
	HANDLE comFindHandle = FindFirstFileA(dirName, &comFindData);
	while (FindNextFileA(comFindHandle, &comFindData))
	{
		if (comFindData.cFileName[0] == '.') continue;
		if (comFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			SetCurrentDirectoryA(comFindData.cFileName);
			getAllFilesForSolution();
		}
		else
		{
			if (endsWith(comFindData.cFileName, ".cpp") ||
				endsWith(comFindData.cFileName, ".hpp") ||
				endsWith(comFindData.cFileName, ".h") ||
				endsWith(comFindData.cFileName, ".c"))
			{
				char dirName2[256];
				GetCurrentDirectoryA(256, dirName2);
				std::string temp{ dirName2 };
				temp += '\\';
				temp += comFindData.cFileName;
				Path tempPath{ temp };
				files.push_back(tempPath.getRelativeTo(SlnPath));
			}
		}
	}
	SetCurrentDirectoryA("..");
	FindClose(comFindHandle);
}

std::string ReadSlnContents(Path& slnPath)
{
	char* contents = nullptr;

	FILE* pFile;
	fopen_s(&pFile, slnPath.getString().c_str(), "r");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		size_t fileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		contents = new char[fileSize];
		fread_s(contents, fileSize, sizeof(char), fileSize, pFile);
		fclose(pFile);
	}
	return std::string{contents};
}

void WriteSlnContents(Path& slnPath, std::string& str)
{
	FILE* pFile;
	fopen_s(&pFile, slnPath.getString().c_str(), "w");
	if (pFile)
	{
		fwrite(str.c_str(), sizeof(char), str.size(), pFile);
		fclose(pFile);
	}
}


int main(int argc, char* argv[])
{
	
#ifdef _DEBUG
	strcpy_s(ExeDirStr, 512, "D:\\workspace\\Shady");
#else
	GetCurrentDirectoryA(512, ExeDirStr);
#endif
	if (argc < 2) return 0;
	tmpStr = ExeDirStr;
	commonsStr = tmpStr + "\\" + shcommons;
	coreStr = tmpStr + "\\" + shcore;
	Path BasePath(ExeDirStr);
	Path SlnArg(argv[1]);
	Sln = BasePath + SlnArg;
	SlnPath = Sln;
	SlnPath.back();
	std::string SlnContents = ReadSlnContents(Sln);
	size_t projBeginIndex = SlnContents.find(PROJECT_BEGIN);
	if (projBeginIndex != std::string::npos)
	{
		projBeginIndex += PROJECT_BEGIN.size();
		size_t projEndIndex = SlnContents.find(PROJECT_END, projBeginIndex);
		SlnContents.erase(projBeginIndex, projEndIndex - projBeginIndex);
		SlnContents.insert(projBeginIndex++, "\n");
		SetCurrentDirectoryA(commonsStr.c_str());
		getAllFilesForSolution();
		SetCurrentDirectoryA("..");
		SetCurrentDirectoryA(coreStr.c_str());
		getAllFilesForSolution();
		SetCurrentDirectoryA("..");
		for (Path& path : files)
		{
			std::string temp = path.getString();
			std::string fin = temp + " = ";
			fin += temp;
			fin += "\n";
			SlnContents.insert(projBeginIndex, fin);
		}
		WriteSlnContents(Sln, SlnContents);
	}
	

	return 0;
}