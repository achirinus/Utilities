#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <Windows.h>
#include <vector>

#define FILEPATH_LENGTH 256
#define MAX_LOGGED_YEARS 5
#define DEV_YEAR 2017
#define GRAPH_LINE_CHARS 50
#define GRAPH_LINES 12
#define MAX_GRAPH_CHARS (93 * GRAPH_LINES)

namespace BuildTimer
{
	typedef struct tm Ctime;
	struct FileHeader
	{
		Ctime lastModifiedDate;
		int numOfEntries;
		double buildStartTime;
	};

	struct Entry
	{
		Ctime date;
		double buildTime;
		bool succeded;
	};
	struct EntryDay
	{
		std::vector<Entry> entries;
	};
	struct EntryMonth
	{
		EntryDay days[31];
		bool hasEntries()
		{
			bool result = false;
			
			for (int j = 0; j < 31; j++)
			{
				if (days[j].entries.size()) result = true;
			}
			return result;
		}
	};
	struct EntryYear
	{
		EntryMonth months[12];

		bool hasEntries()
		{
			bool result = false;
			for (int i = 0; i < 12; i++)
			{
				if (months[i].hasEntries()) result = true;
			}
			return result;
		}
	};

	struct RecordedStats
	{
		float totalBuildTime;
		float avgBuildTime;
		float percentFailed;
		float percentSucceded;
		int numOfEntries;
		int numOfEntriesFailed;
		int numOfEntriesSucceded;
		int hoursSpentBuilding;
		int minutesSpentBuilding;
		int secondsSpentBuilding;
		int milisecondsSpentBuilding;

		void calculateStats()
		{
			if (numOfEntries)
			{
				percentFailed = ((float)numOfEntriesFailed * 100.0f) / ((float)numOfEntries);
				percentSucceded = 100.0f - percentFailed;
				avgBuildTime = totalBuildTime / (float)numOfEntriesSucceded;
				int tempBt = (int)totalBuildTime;
				milisecondsSpentBuilding = (int)((totalBuildTime - (float)tempBt) * 100.0f);
				hoursSpentBuilding = tempBt / 3600;
				tempBt %= 3600;
				minutesSpentBuilding = tempBt / 60;
				secondsSpentBuilding = tempBt % 60;

			}
		}
	};
	
	double getTimeSecs()
	{
		LARGE_INTEGER count;
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&count);

		double result = (double)(count.QuadPart) / (double)(freq.QuadPart);
		return result;
	}
	const char* getFilePath(const char* arg)
	{
		char* result = new char[FILEPATH_LENGTH];

		if (arg[1] == ':') // Is absolute
		{
			return arg;
		}
		else // Is relative and we add cwd
		{
			DWORD index = GetCurrentDirectoryA(FILEPATH_LENGTH, result);
			if (arg[0] != '\\')
			{
				result[index++] = '\\';
			
			}
		
			strcpy_s(&result[index], FILEPATH_LENGTH - index, arg);
		}
		return result;
	}
	bool openFile(FILE** file, const char* fileName, bool create = true)
	{
		bool result = false;
		while (true)
		{
			fopen_s(file, fileName, "rb+");
			if (!*file)
			{
				if (create)
				{
					fopen_s(file, fileName, "wb");
					if (*file)
					{
						result = false;
						fclose(*file);
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				result = true;
				break;
			}
		}
		return result;
	}
	Ctime getCurrentDateTime()
	{
		Ctime result = {};
		time_t now_t = time(0);
		localtime_s(&result, &now_t);
		result.tm_mon++;
		result.tm_year += 1900;

		return result;
	}
	int clampToRange(int rangeRight, int max, int num)
	{
		int result = 0;
		if(max) result = (num * rangeRight) / max;
		return result;
	}
	

	void doBeingJob(const char* fileName)
	{
		FILE* file = nullptr;
		bool exists = openFile(&file, fileName);
		if (!file)
		{
			printf("Failed to create or open file: %s\n", fileName);
			return;
		}

		FileHeader header;
		if (exists)
		{
			fseek(file, 0, SEEK_SET);
			size_t size = fread(&header, sizeof(FileHeader), 1, file);
			if (!size)
			{
				header = {};
			}
		}
		else
		{
			header = {};
		}
		header.lastModifiedDate = getCurrentDateTime();
		header.buildStartTime = getTimeSecs();
		
		fseek(file, 0, SEEK_SET);
		fwrite(&header, sizeof(FileHeader), 1, file);
		fclose(file);
	}
	void doEndJob(const char* fileName, int errorCode)
	{
		FILE* file = nullptr;
		bool exists = openFile(&file, fileName, false);
		if (!exists)
		{
			printf("File: %s does not exist!", fileName);
			return;
		}
		FileHeader header;
		fseek(file, 0, SEEK_SET);
		size_t size = fread(&header, sizeof(FileHeader), 1, file);
		if (!size)
		{
			printf("Could not read header for file: %s!", fileName);
			return;
		}
		double now = getTimeSecs();
		double buildTime = now - header.buildStartTime;
 		header.buildStartTime = 0;
		header.numOfEntries++;
		
		fseek(file, 0, SEEK_SET);
		fwrite(&header, sizeof(FileHeader), 1, file);
		size_t seekPos = sizeof(FileHeader) + sizeof(Entry) * (header.numOfEntries - 1);
		fseek(file, seekPos, SEEK_SET);
		
		Entry entry = { getCurrentDateTime(), buildTime, (bool)(errorCode == 0)};
		fwrite(&entry, sizeof(Entry), 1, file);

		printf("Build time: %.3f seconds\n", buildTime);
	}

	void showHelp(const char* fileName)
	{
		printf("Use buildtime [b/e/s/h] fileName errorCode\n");
		printf("b - Begin (place this before your build starts)\n");
		printf("e - End (place this after your build starts)\n");
		printf("s - Show build stats\n");
		printf("h - Help. Duh, you just used it!\n");
	}

	void showStats(const char* fileName)
	{
		RecordedStats allTime = {};
		RecordedStats thisMonth = {};

		FILE* file = nullptr;
		bool exists = openFile(&file, fileName, false);
		if (!exists)
		{
			printf("File: %s does not exist!", fileName);
			return;
		}

		FileHeader header;
		fseek(file, 0, SEEK_SET);
		fread(&header, sizeof(FileHeader), 1, file);
		fseek(file, sizeof(FileHeader), SEEK_SET);
		Entry* entries = new Entry[header.numOfEntries];
		fread(entries, sizeof(Entry), header.numOfEntries, file);
		fclose(file);

		EntryYear years[MAX_LOGGED_YEARS];
		for (int i = 0; i < header.numOfEntries; i++)
		{
			int dayIndex = entries[i].date.tm_mday - 1;
			int monIndex = entries[i].date.tm_mon - 1;
			int yearIndex = entries[i].date.tm_year - DEV_YEAR;
			years[yearIndex].months[monIndex].days[dayIndex].entries.push_back(entries[i]);
		}
		
		Ctime today = getCurrentDateTime();
		for (int yIndex = 0; yIndex < MAX_LOGGED_YEARS; yIndex++)
		{

			EntryYear& year = years[yIndex];
			if (year.hasEntries())
			{
				
				for (int mIndex = 0; mIndex < 12; mIndex++)
				{
					EntryMonth month = year.months[mIndex];
					if (month.hasEntries())
					{
						for (int dIndex = 0; dIndex < 31; dIndex++)
						{
							EntryDay day = month.days[dIndex];
							for (unsigned int eIndex = 0; eIndex < day.entries.size(); eIndex++)
							{
								Entry entry = day.entries[eIndex];
								allTime.numOfEntries++;
								allTime.totalBuildTime += (float)entry.buildTime;
								entry.succeded ? allTime.numOfEntriesSucceded++ : allTime.numOfEntriesFailed++;
								if ((mIndex == today.tm_mon - 1) && (yIndex == (today.tm_year - DEV_YEAR)))
								{
									entry.succeded ? thisMonth.numOfEntriesSucceded++ : thisMonth.numOfEntriesFailed++;
									thisMonth.numOfEntries++;

									thisMonth.totalBuildTime += (float)entry.buildTime;
								}
							}
						} // for days
					} // if month has data
				} // for month
			} // if this year has data
		} // for years
		thisMonth.calculateStats();
		allTime.calculateStats();
		printf("This month's stats:\n");
		printf("Time spent building(H:M:S:MS): %d:%d:%d:%d\n", thisMonth.hoursSpentBuilding, thisMonth.minutesSpentBuilding, thisMonth.secondsSpentBuilding, thisMonth.milisecondsSpentBuilding);
		printf("Average build time: %f\n", thisMonth.avgBuildTime);
		printf("Number of builds: %d\n", thisMonth.numOfEntries);
		printf("Builds succeded: %d(%.2f%%)\n", thisMonth.numOfEntriesSucceded, thisMonth.percentSucceded);
		printf("Builds failed: %d(%.2f%%)\n", thisMonth.numOfEntriesFailed, thisMonth.percentFailed);

		printf("\n");

		printf("Lifetime stats:\n");
		printf("Time spent building(H:M:S:MS): %d:%d:%d:%d\n", allTime.hoursSpentBuilding, allTime.minutesSpentBuilding, allTime.secondsSpentBuilding, allTime.milisecondsSpentBuilding);
		printf("Average build time: %f\n", allTime.avgBuildTime);
		printf("Number of builds: %d\n", allTime.numOfEntries);
		printf("Builds succeded: %d(%.2f%%)\n", allTime.numOfEntriesSucceded, allTime.percentSucceded);
		printf("Builds failed: %d(%.2f%%)\n", allTime.numOfEntriesFailed, allTime.percentFailed);

	}

	void doYourJob(const char* job, const char* file, int errorCode = 0)
	{
		switch (job[0])
		{
		case 'b': //Begin
		case 'B':
			doBeingJob(file);
			break;
		case 'e': //End
		case 'E':
			doEndJob(file, errorCode);
			break;
		case 's': //Stats
		case 'S':
			showStats(file);
			break;
		case 'h': //Help
		case 'H':
			showHelp(file);
			break;
		default: 
			//Log incorrect command
			break;
		}
	}

	
}

using namespace BuildTimer;

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		//Error, we need at least a g/s and a filename
	}
	else if (argc == 3)
	{
		//We have go/stop and assume that no error in build
		const char* filePath = getFilePath(argv[2]);
		doYourJob(argv[1], filePath);
	}
	else
	{
		//We use all 3 arguments that we need and ignore something else
		const char* filePath = getFilePath(argv[2]);
		doYourJob(argv[1], filePath, atoi(argv[3]));
	}

	return 0;
}