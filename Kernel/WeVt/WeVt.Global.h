#pragma once

// 禁止使用全局 min/max 宏：会污染后续头文件中的标识符与位域名。
// 需要时在源文件中包含 WeVt.MinMax.h 并使用 WEVT_MIN / WEVT_MAX。

namespace Global
{
	BOOLEAN Initialize_Global();
	VOID UnInitialize_Global();

	extern PDRIVER_OBJECT g_DriverObject;
	extern ULONG64 g_DriverBase;
	extern ULONG32 g_DriverSize;

	extern ULONG64 g_KernelBase;
	extern ULONG32 g_KernelSize;

	extern BOOLEAN g_SuportVT;
	extern BOOLEAN g_HypervisorRunning;

	extern fnPsGetNextProcess PsGetNextProcess;
}


