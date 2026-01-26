#pragma once

namespace Global
{
	BOOL Initialize_Global();
	VOID UnInitialize_Global();

	extern PDRIVER_OBJECT g_DriverObject;
	extern ULONG64 g_DriverBase;
	extern ULONG32 g_DriverSize;

	extern ULONG64 g_KernelBase;
	extern ULONG32 g_KernelSize;
}



