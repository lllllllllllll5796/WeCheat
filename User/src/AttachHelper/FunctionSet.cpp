#include "pch.h"
#include "FunctionSet.h"
#include "Global.h"
#include "Hook.h"
#include "Ioctl.h"

#ifdef _WIN64
#define CONTEXT_EXTENDED_REGISTERS 0
#endif // _WIN64

//附加进程时
//(1)NewNtDebugActiveProcess
//(2)NewDbgUiIssueRemoteBreakin
//(3)NewWaitForDebugEvent
//(4)NewGetThreadContext   //频繁(一直被调用)
//(5)NewWaitForDebugEvent  //频繁(命中断点时不会被调用)
//(6)NewSetThreadContext   //断点被命中是调用1次(剥离进程的时候会调用多次)

NTSTATUS NTAPI NewNtDebugActiveProcess(
	_In_ HANDLE ProcessHandle,
	_In_ HANDLE DebugObjectHandle)
{
	NTSTATUS Status;

	LOG_DEBUG("[+] NewNtDebugActiveProcess\n");

	DWORD BytesReturned = 0;
	RING3_PROCESS_CR3 ProcessInfo = { 0 };

	ProcessInfo.ProcessHandle = (ULONG64)ProcessHandle;

	if (SendUserDataToDriver(IOCTL_WEDEBUG_GetProcessCr3,
		&ProcessInfo,
		sizeof(RING3_PROCESS_CR3),
		&ProcessInfo,
		sizeof(RING3_PROCESS_CR3),
		&BytesReturned))
	{
		Global::g_TargetPid = GetProcessId(ProcessHandle);
		Global::g_TargetCr3 = ProcessInfo.Cr3;

		LOG_DEBUG("g_TargetPid: %d\n", Global::g_TargetPid);
		LOG_DEBUG("g_TargetCr3: 0x%p\n", Global::g_TargetCr3);

		if (!Global::g_TargetPid || !Global::g_TargetCr3)
		{
			ReportSeriousError(oxorany("pid 或 cr3为空"));
			return STATUS_UNSUCCESSFUL;
		}

		//连接调试器
		Status = Global::Sys_NtDebugActiveProcess(ProcessHandle, DebugObjectHandle);
		return Status;
	}
	else
	{
		ReportSeriousError(oxorany("无法获取目标进程cr3"));
		return STATUS_UNSUCCESSFUL;
	}
}

NTSTATUS
NTAPI
NewDbgUiIssueRemoteBreakin(IN HANDLE Process)
{
	HANDLE hThread;
	CLIENT_ID ClientId;
	NTSTATUS Status;

	LOG_DEBUG("[+] NewDbgUiIssueRemoteBreakin\n");

	return Global::Sys_DbgUiIssueRemoteBreakin(Process);
}

BOOL
WINAPI
NewWaitForDebugEvent(
	__in LPDEBUG_EVENT lpDebugEvent,
	__in DWORD dwMilliseconds
)
{
	BOOL bRet = FALSE;

	//LOG_DEBUG("[+] NewWaitForDebugEvent\n");

	bRet = Global::Sys_WaitForDebugEvent(lpDebugEvent, dwMilliseconds);

	return bRet;
}

BOOL
WINAPI
NewContinueDebugEvent(
	_In_ DWORD dwProcessId,
	_In_ DWORD dwThreadId,
	_In_ DWORD dwContinueStatus
)
{
	return Global::Sys_ContinueDebugEvent(dwProcessId, dwThreadId, dwContinueStatus);
}

VOID
WINAPI
NewOutputDebugStringA(
	_In_opt_ LPCSTR lpOutputString
)
{
	//return;
	return Global::Sys_OutputDebugStringA(lpOutputString);
}

VOID
WINAPI
NewOutputDebugStringW(
	_In_opt_ LPCWSTR lpOutputString
)
{
	//return;
	return Global::Sys_OutputDebugStringW(lpOutputString);
}

//断点命中,调试剥离进程的时候会走
BOOL
WINAPI
NewSetThreadContext(
	_In_ HANDLE hThread,
	_In_ CONTEXT* lpContext
)
{
	LOG_DEBUG("[+] NewSetThreadContext\n");

	return Global::Sys_SetThreadContext(hThread, lpContext);  //调用系统函数  
}

BOOL WINAPI NewGetThreadContext(
	_In_ HANDLE    hThread,
	_Inout_ LPCONTEXT lpContext
)
{
	//LOG_DEBUG("[+] NewGetThreadContext\n");

	//调用原函数
	BOOL bSuccess = Global::Sys_GetThreadContext(hThread, lpContext);

	return bSuccess;
}

BOOL
WINAPI
NewWriteProcessMemory(
	_In_ HANDLE hProcess,
	_In_ LPVOID lpBaseAddress,
	_In_reads_bytes_(nSize) LPCVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T* lpNumberOfBytesWritten
)
{
	//LOG_DEBUG("[+] NewWriteProcessMemory\n");

	return Global::Sys_WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

BOOL
WINAPI
NewReadProcessMemory(
	_In_ HANDLE hProcess,
	_In_ LPCVOID lpBaseAddress,
	_Out_writes_bytes_to_(nSize, *lpNumberOfBytesRead) LPVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T* lpNumberOfBytesRead
)
{
	//LOG_DEBUG("[+] NewReadProcessMemory\n");

	return Global::Sys_ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
}

BOOL
WINAPI
NewVirtualProtectEx(
	_In_ HANDLE hProcess,
	_In_ LPVOID lpAddress,
	_In_ SIZE_T dwSize,
	_In_ DWORD flNewProtect,
	_Out_ PDWORD lpflOldProtect
)
{
	//LOG_DEBUG("[+] NewVirtualProtectEx\n");

	BOOL bResult = Global::Sys_VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect, lpflOldProtect);
	return bResult;
}
//---------------------------------------------
void Hook_NtDebugActiveProcess()
{
	Global::Sys_NtDebugActiveProcess = (Global::PFN_NTDEBUGACTIVEPROCESS)NtDebugActiveProcess;
	assert(Global::Sys_NtDebugActiveProcess);
	if (Global::Sys_NtDebugActiveProcess)
	{
		HookOn((PVOID*)&Global::Sys_NtDebugActiveProcess, NewNtDebugActiveProcess, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] NtDebugActiveProcess空指针\n");
	}
}

void UnHook_NtDebugActiveProcess()
{
	if (Global::Sys_NtDebugActiveProcess)
	{
		HookOff((PVOID*)&Global::Sys_NtDebugActiveProcess, NewNtDebugActiveProcess, GetCurrentThread());
	}
}

void Hook_DbgUiIssueRemoteBreakin()
{
	Global::Sys_DbgUiIssueRemoteBreakin = (Global::PFN_DBGUIISSUEREMOTEBREAKIN)DbgUiIssueRemoteBreakin;
	assert(Global::Sys_DbgUiIssueRemoteBreakin);
	if (Global::Sys_DbgUiIssueRemoteBreakin)
	{
		HookOn((PVOID*)&Global::Sys_DbgUiIssueRemoteBreakin, NewDbgUiIssueRemoteBreakin, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] DbgUiIssueRemoteBreakin空指针\n");
	}
}

void UnHook_DbgUiIssueRemoteBreakin()
{
	if (Global::Sys_DbgUiIssueRemoteBreakin)
	{
		HookOff((PVOID*)&Global::Sys_DbgUiIssueRemoteBreakin, NewDbgUiIssueRemoteBreakin, GetCurrentThread());
	}
}

void Hook_WaitForDebugEvent()
{
	Global::Sys_WaitForDebugEvent = (Global::PFN_WAITFORDEBUGEVENT)WaitForDebugEvent;
	assert(Global::Sys_WaitForDebugEvent);
	if (Global::Sys_WaitForDebugEvent)
	{
		HookOn((PVOID*)&Global::Sys_WaitForDebugEvent, NewWaitForDebugEvent, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] WaitForDebugEvent空指针\n");
	}
}

void UnHook_WaitForDebugEvent()
{
	if (Global::Sys_WaitForDebugEvent)
	{
		HookOff((PVOID*)&Global::Sys_WaitForDebugEvent, NewWaitForDebugEvent, GetCurrentThread());
	}
}

void Hook_ContinueDebugEvent()
{
	Global::Sys_ContinueDebugEvent = (Global::PFN_CONTINUEDEBUGEVENT)ContinueDebugEvent;
	assert(Global::Sys_ContinueDebugEvent);
	if (Global::Sys_ContinueDebugEvent)
	{
		HookOn((PVOID*)&Global::Sys_ContinueDebugEvent, NewContinueDebugEvent, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] ContinueDebugEvent空指针\n");
	}
}

void UnHook_ContinueDebugEvent()
{
	if (Global::Sys_ContinueDebugEvent)
	{
		HookOff((PVOID*)&Global::Sys_ContinueDebugEvent, NewContinueDebugEvent, GetCurrentThread());
	}
}

//Hook OutputDebugStringA/W处理敏感日志检测
void Hook_OutputDebugStringA()
{
	Global::Sys_OutputDebugStringA = (Global::PFN_OUTPUTDEBUGSTRINGA)OutputDebugStringA;
	assert(Global::Sys_OutputDebugStringA);
	if (Global::Sys_OutputDebugStringA)
	{
		HookOn((PVOID*)&Global::Sys_OutputDebugStringA, NewOutputDebugStringA, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] OutputDebugStringA空指针\n");
	}
}

void UnHook_OutputDebugStringA()
{
	if (Global::Sys_OutputDebugStringA)
	{
		HookOff((PVOID*)&Global::Sys_OutputDebugStringA, NewOutputDebugStringA, GetCurrentThread());
	}
}

void Hook_OutputDebugStringW()
{
	Global::Sys_OutputDebugStringW = (Global::PFN_OUTPUTDEBUGSTRINGW)OutputDebugStringW;
	assert(Global::Sys_OutputDebugStringW);
	if (Global::Sys_OutputDebugStringW)
	{
		HookOn((PVOID*)&Global::Sys_OutputDebugStringW, NewOutputDebugStringW, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] OutputDebugStringW空指针\n");
	}
}

void UnHook_OutputDebugStringW()
{
	if (Global::Sys_OutputDebugStringW)
	{
		HookOff((PVOID*)&Global::Sys_OutputDebugStringW, NewOutputDebugStringW, GetCurrentThread());
	}
}


void Hook_SetThreadContext()
{
	Global::Sys_SetThreadContext = (Global::PFN_SETTHREADCONTEXT)SetThreadContext;
	assert(Global::Sys_SetThreadContext);
	if (Global::Sys_SetThreadContext)
	{
		HookOn((PVOID*)&Global::Sys_SetThreadContext, NewSetThreadContext, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] SetThreadContext空指针\n");
	}
}

void UnHook_SetThreadContext()
{
	if (Global::Sys_SetThreadContext)
	{
		HookOff((PVOID*)&Global::Sys_SetThreadContext, NewSetThreadContext, GetCurrentThread());
	}
}

void Hook_GetThreadContext()
{
	Global::Sys_GetThreadContext = (Global::PFN_GETTHREADCONTEXT)GetThreadContext;
	assert(Global::Sys_GetThreadContext);
	if (Global::Sys_GetThreadContext)
	{
		HookOn((PVOID*)&Global::Sys_GetThreadContext, NewGetThreadContext, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] Sys_GetThreadContext空指针\n");
	}
}

void UnHook_GetThreadContext()
{
	if (Global::Sys_GetThreadContext)
	{
		HookOff((PVOID*)&Global::Sys_GetThreadContext, NewGetThreadContext, GetCurrentThread());
	}
}

void Hook_WriteProcessMemory()
{
	Global::Sys_WriteProcessMemory = (Global::PFN_WRITEPROCESSMEMORY)WriteProcessMemory;
	assert(Global::Sys_WriteProcessMemory);
	if (Global::Sys_WriteProcessMemory)
	{
		HookOn((PVOID*)&Global::Sys_WriteProcessMemory, NewWriteProcessMemory, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] Sys_WriteProcessMemory空指针\n");
	}
}

void UnHook_WriteProcessMemory()
{
	if (Global::Sys_WriteProcessMemory)
	{
		HookOff((PVOID*)&Global::Sys_WriteProcessMemory, NewWriteProcessMemory, GetCurrentThread());
	}
}

void Hook_ReadProcessMemory()
{
	Global::Sys_ReadProcessMemory = (Global::PFN_READPROCESSMEMORY)ReadProcessMemory;
	assert(Global::Sys_ReadProcessMemory);
	if (Global::Sys_ReadProcessMemory)
	{
		HookOn((PVOID*)&Global::Sys_ReadProcessMemory, NewReadProcessMemory, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] Sys_ReadProcessMemory空指针\n");
	}
}

void UnHook_ReadProcessMemory()
{
	if (Global::Sys_ReadProcessMemory)
	{
		HookOff((PVOID*)&Global::Sys_ReadProcessMemory, NewReadProcessMemory, GetCurrentThread());
	}
}

void Hook_VirtualProtectEx()
{
	Global::Sys_VirtualProtectEx = (Global::PFN_VIRTUALPROTECTEX)VirtualProtectEx;
	assert(Global::Sys_VirtualProtectEx);
	if (Global::Sys_VirtualProtectEx)
	{
		HookOn((PVOID*)&Global::Sys_VirtualProtectEx, NewVirtualProtectEx, GetCurrentThread());
	}
	else
	{
		LOG_DEBUG("[-] Sys_VirtualProtectEx空指针\n");
	}
}

void UnHook_VirtualProtectEx()
{
	if (Global::Sys_VirtualProtectEx)
	{
		HookOff((PVOID*)&Global::Sys_VirtualProtectEx, NewVirtualProtectEx, GetCurrentThread());
	}
}