#include "WeVt.pch.h"
#include "WeVt.Global.h"
#include "WeVt.Trace.h"
#include "WeVt.Global.tmh"


namespace Global
{
	PDRIVER_OBJECT g_DriverObject = nullptr;
	ULONG64 g_DriverBase = 0;
	ULONG32 g_DriverSize = 0;

	ULONG64 g_KernelBase = 0;
	ULONG32 g_KernelSize = 0;

	BOOLEAN g_SuportVT = FALSE;
	BOOLEAN g_HypervisorRunning = FALSE;

	fnPsGetNextProcess PsGetNextProcess;

	BOOLEAN Initialize_Global()
	{
		g_KernelProcess = IoGetCurrentProcess();

		OSVERSIONINFOW OsVersion;

		NTSTATUS Status = RtlGetVersion(&OsVersion);
		if (!NT_SUCCESS(Status))
		{
			LOG_DEBUG("[-] RtlGetVersion ʧ��\r\n");
			return FALSE;
		}

		g_CurrentWindowsBuildNumber = OsVersion.dwBuildNumber;

		LOG_DEBUG("[+] ��ǰϵͳ�汾:%d\r\n", g_CurrentWindowsBuildNumber);

		return TRUE;
	}

	VOID UnInitialize_Global()
	{

	}
}