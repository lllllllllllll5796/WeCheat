#pragma once

#define DBGK_EXCEPTIONPORT_MASK           7
#define DBGK_KILL_PROCESS_ON_EXIT         (0x1)
#define DBGK_ALL_FLAGS                    (DBGK_KILL_PROCESS_ON_EXIT)

#define DBGK_THREAD_DEBUG                               0x01
#define DBGK_PROCESS_DEBUG                              0x02
#define DBGK_OBJECT_DEBUG                               0x04
#define DBGK_MESSAGE_DEBUG                              0x08
#define DBGK_EXCEPTION_DEBUG                            0x10

#define _SEH2_TRY __try
#define _SEH2_FINALLY __finally
#define _SEH2_EXCEPT(...) __except(__VA_ARGS__)
#define _SEH2_END
#define _SEH2_GetExceptionInformation() (GetExceptionInformation())
#define _SEH2_GetExceptionCode() (GetExceptionCode())
#define _SEH2_AbnormalTermination() (AbnormalTermination())
#define _SEH2_YIELD(STMT_) STMT_
#define _SEH2_LEAVE __leave
#define _SEH2_VOLATILE

#ifndef _DBGK_DEBUG_
#define DBGKTRACE(x, fmt, ...)
#else
#define DBGKTRACE(x, fmt, ...) LOG_DEBUG(fmt, ##__VA_ARGS__)
#endif

#ifndef ProbeForWriteHandle
#define ProbeForWriteHandle(Address) {                                   \
	if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {            \
		*(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;             \
	}                                                                    \
	*(volatile HANDLE *)(Address) = *(volatile HANDLE *)(Address);       \
}
#endif

typedef struct _DEBUGGER_TABLE_ENTRY
{
	LIST_ENTRY list_entry;
	DWORD dwPid;
	ULONG64 fileData;
	ULONG64 fileData2;
} DEBUGGER_TABLE_ENTRY, * PDEBUGGER_TABLE_ENTRY;

union PROCESSFLAGS {
	unsigned long Flags;
	struct
	{
		unsigned long CreateReported : 1;
		unsigned long NoDebugInherit : 1;
		unsigned long ProcessExiting : 1;
		unsigned long ProcessDelete : 1;
		unsigned long ManageExecutableMemoryWrites : 1;
		unsigned long VmDeleted : 1;
		unsigned long OutswapEnabled : 1;
		unsigned long Outswapped : 1;
		unsigned long FailFastOnCommitFail : 1;
		unsigned long Wow64VaSpace4Gb : 1;
		unsigned long AddressSpaceInitialized : 2;
		unsigned long SetTimerResolution : 1;
		unsigned long BreakOnTermination : 1;
		unsigned long DeprioritizeViews : 1;
		unsigned long WriteWatch : 1;
		unsigned long ProcessInSession : 1;
		unsigned long OverrideAddressSpace : 1;
		unsigned long HasAddressSpace : 1;
		unsigned long ImageNotifyDone : 1;
		unsigned long PdeUpdateNeeded : 1;
		unsigned long VdmAllowed : 1;
		unsigned long ProcessRundown : 1;
		unsigned long ProcessInserted : 1;
		unsigned long DefaultPagePriority : 3;
		unsigned long ProcessSelfDelete : 1;
		unsigned long SetTimerResolutionLink : 1;
	};
};

typedef struct _PSP_OBJECT_CREATION_STATE
{
	ACCESS_STATE AccessState;
	UCHAR AuxData[0xE0];
	ULONG HandleAttributes;
	KPROCESSOR_MODE PreviousMode;
	HANDLE NewHandle;
} PSP_OBJECT_CREATION_STATE, * PPSP_OBJECT_CREATION_STATE;

typedef struct _WIN11_PS_SYSTEM_DLL_INFO
{
	ULONG Flags;
	LONG Padding;
	UNICODE_STRING DllPath;
	PVOID BaseAddress;
	USHORT* DbgName;
} WIN11_PS_SYSTEM_DLL_INFO, * PWIN11_PS_SYSTEM_DLL_INFO;

typedef struct _PSP_SYSTEM_DLL_DATA
{
	PPSP_SYSTEM_DLL SystemDll;
	WIN11_PS_SYSTEM_DLL_INFO DllInfo;
} PSP_SYSTEM_DLL_DATA, * PPSP_SYSTEM_DLL_DATA;

union CROSSTHREADFLAGS {
	unsigned long All;
	struct
	{
		unsigned long Terminated : 1;
		unsigned long ThreadInserted : 1;
		unsigned long HideFromDebugger : 1;
		unsigned long ActiveImpersonationInfo : 1;
		unsigned long HardErrorsAreDisabled : 1;
		unsigned long BreakOnTermination : 1;
		unsigned long SkipCreationMsg : 1;
		unsigned long SkipTerminationMsg : 1;
		unsigned long CopyTokenOnOpen : 1;
		unsigned long ThreadIoPriority : 3;
		unsigned long ThreadPagePriority : 3;
		unsigned long RundownFail : 1;
		unsigned long UmsForceQueueTermination : 1;
		unsigned long IndirectCpuSets : 1;
		unsigned long DisableDynamicCodeOptOut : 1;
		unsigned long ExplicitCaseSensitivity : 1;
		unsigned long PicoNotifyExit : 1;
		unsigned long DbgWerUserReportActive : 1;
		unsigned long ForcedSelfTrimActive : 1;
		unsigned long SamplingCoverage : 1;
		unsigned long ReservedCrossThreadFlags : 8;
	};
};

union WeDebug_Dr6 {
	uint64_t flags;
	struct
	{
		uint64_t B0 : 1;
		uint64_t B1 : 1;
		uint64_t B2 : 1;
		uint64_t B3 : 1;
		uint64_t Reserved_1 : 7;
		uint64_t BLD : 1;
		uint64_t Reserved_2 : 1;
		uint64_t BD : 1;
		uint64_t BS : 1;
		uint64_t BT : 1;
		uint64_t RTM : 1;
	};
};

#pragma pack(push, 8)
typedef struct _Debugport_Struct
{
	unsigned __int64 id;
	size_t DebugPort;
} Debugport_Struct, * PDebugport_Struct;
#pragma pack(pop)

inline unsigned short WeDebug_GetProcessMachine(PVOID Process)
{
	if (Process == nullptr)
	{
		return 0;
	}

	if (Offset::EProcess::Machine != 0 && Offset::EProcess::Machine != 0xFFFFFFFF)
	{
		return *reinterpret_cast<unsigned short*>(reinterpret_cast<size_t>(Process) + Offset::EProcess::Machine);
	}

	const size_t wow64 = *reinterpret_cast<size_t*>(reinterpret_cast<size_t>(Process) + Offset::EProcess::WoW64Process);
	if (wow64 != 0 && Offset::EWow64Process::Machine != 0 && Offset::EWow64Process::Machine != 0xFFFFFFFF)
	{
		return *reinterpret_cast<unsigned short*>(wow64 + Offset::EWow64Process::Machine);
	}

	return 0;
}

FORCEINLINE VOID PspLockProcessListExclusive(IN PETHREAD CurrentThread)
{
	UNREFERENCED_PARAMETER(CurrentThread);
	KeEnterGuardedRegion();
	ExAcquirePushLockExclusive(reinterpret_cast<PULONG_PTR>(Global::PspActiveProcessLock));
}

FORCEINLINE VOID PspUnlockProcessExclusive(IN PEPROCESS Process, IN PETHREAD CurrentThread)
{
	UNREFERENCED_PARAMETER(CurrentThread);
	const size_t ptr_ProcessLock = reinterpret_cast<size_t>(Process) + Offset::EProcess::ProcessLock;
	ExReleasePushLockExclusive(reinterpret_cast<PULONG_PTR>(ptr_ProcessLock));
	KeLeaveCriticalRegion();
}

inline bool WeDebug_IsWow64Process(PVOID Process)
{
	const size_t wow64 = *reinterpret_cast<size_t*>(reinterpret_cast<size_t>(Process) + Offset::EProcess::WoW64Process);
	const unsigned short machine = WeDebug_GetProcessMachine(Process);
	return wow64 != 0 && (machine == IMAGE_FILE_MACHINE_I386 || machine == IMAGE_FILE_MACHINE_ARMNT);
}
