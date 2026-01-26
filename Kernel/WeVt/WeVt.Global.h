#pragma once

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

namespace Global
{
	BOOL Initialize_Global();
	VOID UnInitialize_Global();

	extern PDRIVER_OBJECT g_DriverObject;
	extern ULONG64 g_DriverBase;
	extern ULONG32 g_DriverSize;

	extern ULONG64 g_KernelBase;
	extern ULONG32 g_KernelSize;

	extern BOOL g_SuportVT;
	extern BOOL g_HypervisorRunning;
}


