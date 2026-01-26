#pragma once

namespace KernelCommon
{
	typedef struct _SYSCALLNUMBERS
	{
		SHORT NtSetInformationThread;
		SHORT NtQueryInformationProcess;
		SHORT NtQueryObject;
		SHORT NtSystemDebugControl;
		SHORT NtSetContextThread;
		SHORT NtQuerySystemInformation;
		SHORT NtGetContextThread;
		SHORT NtClose;
		SHORT NtQueryInformationThread;
		SHORT NtCreateThreadEx;
		SHORT NtCreateFile;
		SHORT NtCreateProcessEx;
		SHORT NtYieldExecution;
		SHORT NtQuerySystemTime;
		SHORT NtQueryPerformanceCounter;
		SHORT NtContinue;
		SHORT NtQueryInformationJobObject;
		SHORT NtCreateUserProcess;
		SHORT NtGetNextProcess;
		SHORT NtOpenProcess;
		SHORT NtOpenThread;
		SHORT NtSetInformationProcess;
	} SYSCALLNUMBERS;

	typedef struct _WIN32KSYSCALLNUMBERS
	{
		SHORT NtUserFindWindowEx;
		SHORT NtUserBuildHwndList;
		SHORT NtUserQueryWindow;
		SHORT NtUserGetForegroundWindow;
		SHORT NtUserGetThreadState;
		SHORT NtUserGetClassName;
		SHORT NtUserInternalGetWindowText;
		SHORT NtUserWindowFromPoint;
		SHORT NtUserChildWindowFromPointEx;
		SHORT NtUserRealChildWindowFromPoint;
	} WIN32KSYSCALLNUMBERS;

	typedef struct _NTAPIOFFSET
	{
		ULONG Offset_ObjectTable;
		ULONG Offset_SectionBaseAddress;
		ULONG Offset_Win32StartAddress;
		ULONG Offset_SectionObject;
		ULONG Offset_PreviousMode;
		ULONG Offset_SuspendCount;

		ULONG Offset_SeAuditProcessCreationInfo;
		ULONG Offset_BypassProcessFreezeFlag;

		ULONG Offset_ThreadHideFromDebuggerFlag;
		ULONG Offset_ThreadBreakOnTerminationFlag;
		ULONG Offset_PicoContext;
		ULONG Offset_RestrictSetThreadContext;
	} NTAPIOFFSET;

	extern SYSCALLNUMBERS g_SyscallNumbers;
	extern WIN32KSYSCALLNUMBERS g_Win32KSyscallNumbers;
	extern NTAPIOFFSET g_NtapiOffsets;

	//
	// This is a system thread
	//

#define PS_CROSS_THREAD_FLAGS_SYSTEM               0x00000010UL

	BOOL Is_Win7_7600();
	BOOL Is_Win7_7601();
	BOOL Is_Win8_9200();
	BOOL Is_Win8_9600();
	BOOL Is_Win7();
	BOOL Is_Win8();
	BOOL Is_Win10();
	BOOL Is_Win11();

	//-------------------
	ULONG GetObjectTable_Offset();
	ULONG GetSectionBaseAddress_Offset();
	ULONG GetWin32StartAddress_Offset();
	ULONG GetActiveProcessLinks_Offset();
	ULONG GetSectionObject_Offset();
	ULONG GetPreviousMode_Offset();

	PULONG_PTR GetCrossThreadFlagsPointer(PETHREAD Thread);
	PEX_RUNDOWN_REF GetThreadRundownProtectPointer(PETHREAD Thread);
	ULONG32 PspGetThreadGrantAccess(PETHREAD Thread);
	ULONG32 GetCrossThreadFlags(PETHREAD Thread);
	VOID SetCrossThreadFlags(PETHREAD Thread, UINT32 aFlags);
	VOID SetProcessDebugPort(PEPROCESS Process, PDEBUG_OBJECT Port);
	BOOL IS_SYSTEM_THREAD(PETHREAD Thread);
	PEX_RUNDOWN_REF GetProcessRundownProtectPointer(PEPROCESS Process);
	PVOID PspGetProcessSectionObject(PEPROCESS Process);
	PVOID PspGetProcessSectionBaseAddress(PEPROCESS Process);
	PVOID PspGetThreadStartAddress(PETHREAD Thread);
	PVOID PspGetThreadWin32StartAddress(PETHREAD Thread);
	PVOID PsGetProcessDebugPortPointer(PEPROCESS Process);
	PVOID PsGetProcessFlagsPointer(PEPROCESS Process);
	ULONG32 PsGetProcessFlags(PEPROCESS Process);
	UINT32 GetThreadClonedThread(PETHREAD Thread);
	UINT32 GetThreadInserted(PETHREAD Thread);
	CLIENT_ID GetThreadCid(PETHREAD Thread);
	PCLIENT_ID PspGetThreadCIDPointer(PETHREAD Thread);
	ULONG32 GetActiveThreads(PEPROCESS Process);

	INT32 GetProcessExitStatus(PEPROCESS Process);
	NTSTATUS GetThreadExitStatus(PETHREAD Thread);

	INT32 GetLastThreadExitStatus(PEPROCESS Process);
	PLARGE_INTEGER GetProcessExitTimePointer(PEPROCESS Process);
	::PPEB GetProcessPeb(PEPROCESS Process);
	PWOW64_PROCESS GetProcessWow64Process(PEPROCESS Process);
	PDEBUG_OBJECT GetProcessDebugPort(PEPROCESS Process);
	PEPROCESS PsGetProcessByThread(PETHREAD Thread);
	VOID SetProcessDebugActive(PEPROCESS Process, BOOL DebugActive);
	BOOLEAN GetProcessDebugActive(PEPROCESS Process);
	PVOID PspGetThreadTebPointer(PETHREAD Thread);
	PVOID NtCurrentTeb(VOID);
	BOOL GetSuppressDebugMsg(PVOID pTeb64);
	BOOL IsSystemThread(PETHREAD Thread);
	UINT8 GetThreadApcStateIndex(PETHREAD Thread);
	PVOID GetThreadInitialStack(PETHREAD Thread);
	PEX_RUNDOWN_REF GetExAcquireRundownProtection(PETHREAD Thread);
	PVOID GetThreadProcess(PETHREAD Thread);

	BOOL Get_SSDT_Numbers();
	BOOL Get_ShadowSSDT_Numbers();
	BOOL Get_Offsets_InStruct();

}