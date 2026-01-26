#include "KernelCommon.pch.h"
#include "FIntrin.h"

// unsigned long __segmentlimit(unsigned long selector)
// {
// 	unsigned long limit;
// 	__asm __volatile("lsl %1, %0" : "=r" (limit) : "r" (selector));
// 	return limit;
// }

// void cpuid(int CPUInfo[4], int InfoType)
// {
// 	__asm __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType));
// }

// void cpuidex(int CPUInfo[4], int InfoType, int ECXValue)
// {
// 	__asm __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType), "c" (ECXValue));
// }