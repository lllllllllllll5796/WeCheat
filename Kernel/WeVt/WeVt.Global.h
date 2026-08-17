#pragma once

namespace Global
{
	namespace ethread_offset
	{
		extern size_t Tcb;
		extern size_t CrossThreadFlags;
		extern size_t Cid;
		extern size_t RundownProtect;
	}

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


