#pragma once

typedef BOOLEAN(NTAPI* PLDR_INIT_ROUTINE)(
	_In_ PVOID DllHandle,
	_In_ ULONG Reason,
	_In_opt_ PVOID Context
	);

typedef VOID(NTAPI* PKNORMAL_ROUTINE)
(
	PVOID NormalContext,
	PVOID SystemArgument1,
	PVOID SystemArgument2
);

typedef VOID(NTAPI* PKKERNEL_ROUTINE)
(
	PRKAPC Apc,
	PKNORMAL_ROUTINE* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2
);

typedef VOID(NTAPI* PKRUNDOWN_ROUTINE)
(
	PRKAPC Apc
);

using fnZwReadFile = NTSTATUS(NTAPI*)(
	_In_     HANDLE           FileHandle,
	_In_opt_ HANDLE           Event,
	_In_opt_ PIO_APC_ROUTINE  ApcRoutine,
	_In_opt_ PVOID            ApcContext,
	_Out_    PIO_STATUS_BLOCK IoStatusBlock,
	_Out_    PVOID            Buffer,
	_In_     ULONG            Length,
	_In_opt_ PLARGE_INTEGER   ByteOffset,
	_In_opt_ PULONG           Key
	);

using fnZwCreateFile = NTSTATUS(NTAPI*)(
	_Out_    PHANDLE            FileHandle,
	_In_     ACCESS_MASK        DesiredAccess,
	_In_     POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_    PIO_STATUS_BLOCK   IoStatusBlock,
	_In_opt_ PLARGE_INTEGER     AllocationSize,
	_In_     ULONG              FileAttributes,
	_In_     ULONG              ShareAccess,
	_In_     ULONG              CreateDisposition,
	_In_     ULONG              CreateOptions,
	_In_opt_ PVOID              EaBuffer,
	_In_     ULONG              EaLength
	);

using fnZwCreateThreadEx = NTSTATUS(NTAPI*)
(
	OUT PHANDLE ThreadHandle,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID StartAddress,
	IN PVOID Parameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID ByteBuffer
	);

using fnZwReadVirtualMemory = NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PVOID BaseAddress,
	OUT PVOID Buffer,
	IN ULONG BufferLength,
	OUT PULONG ReturnLength OPTIONAL
	);

using fnZwWriteVirtualMemory = NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PVOID BaseAddress,
	OUT PVOID Buffer,
	IN ULONG BufferLength,
	OUT PULONG ReturnLength OPTIONAL
	);

using fnZwProtectVirtualMemory = NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PVOID* BaseAddress,
	IN SIZE_T* NumberOfBytesToProtect,
	IN ULONG     NewAccessProtection,
	OUT PULONG     OldAccessProtection
	);

using fnZwQueryInformationProcess =  NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

using fnNtQueryInformationThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN THREADINFOCLASS ThreadInformationClass,
	IN OUT PVOID ThreadInformation,
	IN ULONG ThreadInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

using fnZwQuerySystemInformation = NTSTATUS(NTAPI*)(
	__in wdk::SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
	__in ULONG SystemInformationLength,
	__out_opt PSIZE_T ReturnLength
	);

using fnNtQuerySystemInformation =  NTSTATUS(NTAPI*)(
	IN wdk::SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

using fnPsReferenceProcessFilePointer = NTSTATUS(NTAPI*) 
    (
		IN PEPROCESS, 
		OUT PFILE_OBJECT*
	);

using fnIoCreateDriver = NTSTATUS(NTAPI*)(PUNICODE_STRING, PDRIVER_INITIALIZE);

using fnIoDeleteDriver = VOID(NTAPI*)(PDRIVER_OBJECT);

using fnHalGetInterruptVector = decltype(&HalGetInterruptVector);


using fnZwTerminateThread = NTSTATUS(NTAPI*)(
	HANDLE hThread, ULONG uExitCode
	);

using fnNtGetContextThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN OUT PCONTEXT Context
	);

using fnNtSetContextThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN PCONTEXT Context
	);

using fnNtSuspendThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	OUT PULONG SuspendCount OPTIONAL);


using fnNtResumeThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	OUT PULONG SuspendCount OPTIONAL);

using fnNtContinue = NTSTATUS(NTAPI*)(
	IN PCONTEXT Context,
	BOOLEAN RaiseAlert
	);

using fnNtDuplicateObject = NTSTATUS(NTAPI*)(
	IN HANDLE SourceProcessHandle,
	IN HANDLE SourceHandle,
	IN HANDLE TargetProcessHandle,
	OUT PHANDLE TargetHandle,
	IN ACCESS_MASK DesiredAccess OPTIONAL,
	IN ULONG HandleAttributes,
	IN ULONG Options
	);

using fnKeRaiseUserException = NTSTATUS(NTAPI*)(
	IN NTSTATUS ExceptionCode
	);

using fnNtSetInformationThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN THREADINFOCLASS ThreadInformationClass,
	IN PVOID ThreadInformation,
	IN ULONG ThreadInformationLength
	);

using fnNtSetInformationProcess = NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	IN PVOID ProcessInformation,
	IN ULONG ProcessInformationLength
	);

typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

using fnNtCreateEvent = NTSTATUS(NTAPI*)(
	OUT PHANDLE             EventHandle,
	IN ACCESS_MASK          DesiredAccess,
	IN POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
	IN EVENT_TYPE           EventType,
	IN BOOLEAN              InitialState);

/// <summary>
/// 这个函数在win7以上是取Wow64Process的PEB XP取的是WOW64_PROCESS结构体指针
/// </summary>
using fnPsGetProcessWow64Process = wdk::PPEB(NTAPI*)(
	IN PEPROCESS Process);

/// <summary>
/// 取x64 Process的Peb
/// </summary>
using fnPsGetProcessPeb = wdk::PPEB(NTAPI*)(
	IN PEPROCESS Process);

using fnPsSuspendProcess = NTSTATUS(NTAPI*)(
	PEPROCESS Process
	);

using fnNtOpenProcess = NTSTATUS(NTAPI*)(
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	);

using fnNtOpenThread = NTSTATUS(NTAPI*)(
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	);

using fnNtTerminateThread = NTSTATUS(NTAPI*)(
	_In_opt_ HANDLE ThreadHandle,
	_In_ NTSTATUS ExitStatus
	);

using fnNtTerminateProcess = NTSTATUS(NTAPI*)(
	_Inout_ PEPROCESS pEprocess,
	_In_    DWORD  dwExitCode
	);

using fnPspTerminateThreadByPointer = NTSTATUS(NTAPI*)(
	IN PETHREAD Thread,
	IN NTSTATUS ExitStatus
	);

using fnNtSetInformationThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN THREADINFOCLASS ThreadInfoClass,
	IN PVOID ThreadInformation,
	IN ULONG ThreadInformationLength
	);

using fnNtClose = NTSTATUS(NTAPI*)(
	IN HANDLE handle
	);


using fnNtDeviceIoControlFile = NTSTATUS(NTAPI*) (
	IN HANDLE               FileHandle,
	IN HANDLE               Event OPTIONAL,
	IN PIO_APC_ROUTINE      ApcRoutine OPTIONAL,
	IN PVOID                ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK    IoStatusBlock,
	IN ULONG                IoControlCode,
	IN PVOID                InputBuffer OPTIONAL,
	IN ULONG                InputBufferLength,
	OUT PVOID               OutputBuffer OPTIONAL,
	IN ULONG                OutputBufferLength);

using fnNtQueryObject = NTSTATUS(NTAPI*)(
	IN HANDLE Handle OPTIONAL,
	IN wdk::OBJECT_INFORMATION_CLASS ObjectInformationClass,
	OUT PVOID ObjectInformation OPTIONAL,
	IN ULONG ObjectInformationLength,
	OUT PULONG ReturnLength OPTIONAL);

using fnNtSetContextThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN PCONTEXT Context);


using fnNtSystemDebugControl = NTSTATUS(NTAPI*)(
	IN SYSDBG_COMMAND Command,
	IN PVOID InputBuffer,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer,
	IN ULONG OutputBufferLength,
	OUT PULONG ReturnLength);

using fnNtQueryInformationProcess = NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength);

using fnNtProtectVirtualMemory = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle,
	PVOID* BaseAddress,
	SIZE_T* NumberOfBytesToProtect,
	ULONG NewAccessProtection,
	PULONG OldAccessProtection);

using fnNtWriteVirtualMemory = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	PVOID Buffer,
	SIZE_T BufferLength,
	PSIZE_T ReturnLength);

using fnNtReadVirtualMemory = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	PVOID Buffer,
	SIZE_T BufferLength,
	PSIZE_T ReturnLength);

using fnNtQueryVirtualMemory = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	wdk::MEMORY_INFORMATION_CLASS MemoryInformationClass,
	PVOID MemoryInformation,
	SIZE_T MemoryInformationLength,
	PSIZE_T ReturnLength);


using fnNtCreateThreadEx = NTSTATUS(NTAPI*)(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID lpStartAddress,
	IN PVOID lpParameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID lpBytesBuffer);

using fnNtLockVirtualMemory = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle,
	PVOID* BaseAddress,
	PSIZE_T NumberOfBytesToLock,
	ULONG_PTR LockType);

using fnNtUserFindWindowEx = ULONG(NTAPI*)(
	IN HWND hwndParent,
	IN HWND hwndChild,
	IN PUNICODE_STRING pstrClassName OPTIONAL,
	IN PUNICODE_STRING pstrWindowName OPTIONAL,
	IN DWORD dwType);

using fnNtUserQueryWindow = UINT_PTR(NTAPI*)(
	IN ULONG WindowHandle,
	IN ULONG TypeInformation);

using fnNtUserGetForegroundWindow = ULONG(NTAPI*)(VOID);

using fnNtCreateFile = NTSTATUS(NTAPI*)(
	_Out_ PHANDLE FileHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_opt_ PLARGE_INTEGER AllocationSize,
	_In_ ULONG FileAttributes,
	_In_ ULONG ShareAccess,
	_In_ ULONG CreateDisposition,
	_In_ ULONG CreateOptions,
	_In_reads_bytes_opt_(EaLength) PVOID EaBuffer,
	_In_ ULONG EaLength);


using fnNtOpenFile = NTSTATUS(NTAPI*)(
	_Out_ PHANDLE FileHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_ ULONG ShareAccess,
	_In_ ULONG OpenOptions);

using fnNtTraceControl = NTSTATUS(NTAPI*)(
	_In_ ULONG FunctionCode,
	_In_reads_bytes_opt_(InBufferLen) PVOID InBuffer,
	_In_ ULONG InBufferLen,
	_Out_writes_bytes_opt_(OutBufferLen) PVOID OutBuffer,
	_In_ ULONG OutBufferLen,
	_Out_ PULONG ReturnLength
	);

using fnNtUserCallOneParam = DWORD_PTR(NTAPI*)(
	HDC Param,
	DWORD Routine
	);

using fnPsCallImageNotifyRoutines = VOID(NTAPI*)(IN PUNICODE_STRING ImageName, IN HANDLE ProcessId, OUT PIMAGE_INFO_EX ImageInfoEx, IN PVOID FileObject);

using fnKiCheckForKernelApcDelivery = VOID(NTAPI*)();

using fnPsGetNextProcessThread = PETHREAD(NTAPI*)(
	IN PEPROCESS Process,
	IN PETHREAD Thread);

using fnPsSuspendThread = NTSTATUS(NTAPI*)(
	PETHREAD Thread,
	OUT PULONG PreviousSuspendCount OPTIONAL);

using fnPsResumeThread  = NTSTATUS(NTAPI*)(
	PETHREAD Thread,
	OUT PULONG PreviousSuspendCount OPTIONAL);

using fnObDuplicateObject =	NTSTATUS(NTAPI*)(
	IN PEPROCESS SourceProcess,
	IN HANDLE SourceHandle,
	IN PEPROCESS TargetProcess OPTIONAL,
	OUT PHANDLE TargetHandle OPTIONAL,
	IN ACCESS_MASK DesiredAccess,
	IN ULONG HandleAttributes,
	IN ULONG Options,
	IN KPROCESSOR_MODE PreviousMode);

using fnPsSuspendProcess = NTSTATUS(NTAPI*)(
	PEPROCESS Process);

using fnPsResumeProcess = NTSTATUS(NTAPI*)(
	PEPROCESS Process);

using fnMmGetFileNameForAddress = NTSTATUS(NTAPI*)(
	IN PVOID ProcessVa,
	OUT PUNICODE_STRING FileName);

using fnNtQueryInformationThread = NTSTATUS(NTAPI*)(
	HANDLE          ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID           ThreadInformation,
	ULONG           ThreadInformationLength,
	PULONG          ReturnLength);

using fnNtGetContextThread = NTSTATUS(NTAPI*)(
	IN HANDLE ThreadHandle,
	IN OUT PCONTEXT Context);

using fnNtSetContextThread = NTSTATUS(NTAPI*)(
	__in HANDLE ThreadHandle,
	__in PCONTEXT ThreadContext);

using fnNtSetInformationThread = NTSTATUS(NTAPI*)(
	HANDLE threadHandle,
	THREADINFOCLASS threadInformationClass,
	PVOID threadInformation,
	ULONG threadInformationLength);

using fnNtQueryInformationProcess = NTSTATUS(NTAPI*)(
	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	_Out_ PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength);

using fnNtReadVirtualMemory = NTSTATUS(NTAPI*)(
	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress,
	_Out_ PVOID Buffer,
	_In_ SIZE_T BufferSize,
	_Out_opt_ PSIZE_T NumberOfBytesRead);

using fnNtWriteVirtualMemory = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	VOID* Buffer,
	SIZE_T BufferSize,
	PSIZE_T NumberOfBytesWritten);

using fnNtDebugActiveProcess = NTSTATUS(NTAPI*)(
	HANDLE ProcessHandle, HANDLE DebugObjectHandle);

using fnZwFlushInstructionCache = NTSTATUS(NTAPI*)(
	HANDLE Process,
	PVOID BaseAddress,
	SIZE_T Size);

using fnDbgkForwardException = BOOLEAN(NTAPI*)(
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN BOOLEAN DebugException,
	IN BOOLEAN SecondChance);

using fnDbgkpSectionToFileHandle = HANDLE(NTAPI*)(
	IN PVOID SectionObject);

using fnExLockUserBuffer = NTSTATUS(NTAPI*)(
	__inout_bcount(Length) PVOID Buffer,
	__in ULONG Length,
	__in KPROCESSOR_MODE ProbeMode,
	__in LOCK_OPERATION LockMode,
	__deref_out PVOID* LockedBuffer,
	__deref_out PVOID* LockVariable);

using fnExUnlockUserBuffer = VOID(NTAPI*)(__inout PVOID LockVariable);

using fnPsGetProcessImageFileName = PCHAR(NTAPI*)(PEPROCESS Process);

using fnDbgkpSuspendProcess = BOOLEAN(NTAPI*)(IN PEPROCESS Process);

using fnDbgkpResumeProcess = VOID(NTAPI*)(IN PEPROCESS Process);

using fnPsQuerySystemDllInfo = PVOID(NTAPI*)(int index);

using fnPsWow64GetProcessNtdllType = SYSTEM_DLL_TYPE(__fastcall* )(IN PEPROCESS Process);

using fnPspReferenceSystemDll = PSECTION(__fastcall* )(IN PPSP_SYSTEM_DLL SystemDll);

using fnPspImplicitAssignProcessToJob = NTSTATUS(__fastcall* )(IN PEJOB Job,
	IN PEPROCESS Process,
	IN ULONG Flags);

using fnPspUnlockProcessListExclusive = VOID(__fastcall* )(IN PKTHREAD Thread);

using fnSepDeleteAccessState = VOID(__fastcall* )(IN PACCESS_STATE PassedAccessState);

using fnDbgkpPostModuleMessages = NTSTATUS(NTAPI*)(
	IN PEPROCESS Process,
	IN PETHREAD Thread,
	IN PVOID DebugObject);

using fnRtlDispatchException = VOID(NTAPI*)(IN PEXCEPTION_RECORD ExceptionRecord, IN PCONTEXT ContextRecord);

using fnObpCallPreOperationCallbacks = ULONG64(NTAPI*)(
	POBJECT_TYPE pObjectType, 
	POB_PRE_OPERATION_INFORMATION OperationInformation,
	PVOID bNeedCallPost);

using fnNtUserGetThreadState =  HANDLE(NTAPI*)(ULONG Routine);

using fnvDbgPrintExWithPrefix = ULONG(NTAPI*) (
	IN PCCH    Prefix,
	IN ULONG   ComponentId,
	IN ULONG   Level,
	IN PCCH    Format,
	IN va_list arglist);

//--------------Shadow函数----------------
typedef PWND(__fastcall* fnValiDateHwnd)(_In_ HWND hwnd);

typedef HWND(__fastcall* fnNtUserWindowFromPoint)(POINT Point);


