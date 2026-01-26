#include "UserCommon.pch.h"
#include "UserCommon.time.h"

uint64_t GetTickCountInMicroSeconds()
{
	LARGE_INTEGER PerformanceCount, Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&PerformanceCount);
	return PerformanceCount.QuadPart * 1000000 / Frequency.QuadPart;
}

float GetTimeSeconds()
{
	return GetTickCountInMicroSeconds() / 1000000.0f;
}
