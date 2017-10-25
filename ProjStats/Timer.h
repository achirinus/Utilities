#pragma once

#include "Utils.h"

struct Timer
{
	LARGE_INTEGER freq;
	LARGE_INTEGER startTicks;
	Timer()
	{
		QueryPerformanceFrequency(&freq);
	}
	void start()
	{
		QueryPerformanceCounter(&startTicks);
	}
	int64 getElapsedTime()
	{
		LARGE_INTEGER stopTicks;
		QueryPerformanceCounter(&stopTicks);
		LARGE_INTEGER elapsedTicks;
		elapsedTicks.QuadPart= stopTicks.QuadPart - startTicks.QuadPart;
		elapsedTicks.QuadPart *= 1000;
		elapsedTicks.QuadPart /= freq.QuadPart;

		return elapsedTicks.QuadPart;
	}
};
