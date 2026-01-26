#include "pch.h"
#include "Global.h"

namespace Global
{
	DWORD g_dwNumberOfProcessors;  //逻辑处理器数量
	PROCESS_INFO g_ProcessInfo = { 0 };
	HANDLE g_hGeneralDriverDevice = INVALID_HANDLE_VALUE;
	DWORD g_TargetPid;   //目标进程的pid
	ULONG64 g_TargetCr3; //目标进程的cr3
	PVOID BaseThreadInitThunk;

	//注意事项: 当我们hook 系统的函数转到我们自定义的函数里时
	//我们自定义的函数所采用的调用约定必须设置为__stdcall
	//因为如果将hook项目编译为32位dll时是需要严格的函数调用约定的
	//否则可能会导致栈不平衡

	PFN_DEBUGACTIVEPROCESS Sys_DebugActiveProcess;
	PFN_NTDEBUGACTIVEPROCESS Sys_NtDebugActiveProcess;
	PFN_DBGUIISSUEREMOTEBREAKIN Sys_DbgUiIssueRemoteBreakin;
	PFN_NTCREATEUSERPROCESS Sys_NtCreateUserProcess;
	PFN_WAITFORDEBUGEVENT Sys_WaitForDebugEvent;
	PFN_CONTINUEDEBUGEVENT Sys_ContinueDebugEvent;
	PFN_OUTPUTDEBUGSTRINGA Sys_OutputDebugStringA;
	PFN_OUTPUTDEBUGSTRINGW Sys_OutputDebugStringW;
	PFN_DBGUIDEBUGACTIVEPROCESS Sys_DbgUiDebugActiveProcess;
	PFN_SETTHREADCONTEXT Sys_SetThreadContext;
	PFN_GETTHREADCONTEXT Sys_GetThreadContext;
	PFN_VIRTUALPROTECTEX Sys_VirtualProtectEx;
	PFN_WRITEPROCESSMEMORY Sys_WriteProcessMemory;
	PFN_READPROCESSMEMORY Sys_ReadProcessMemory;
	PFN_NTDEBUGCONTINUE Sys_NtDebugContinue;
}



