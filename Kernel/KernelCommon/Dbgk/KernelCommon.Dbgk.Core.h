#pragma once

/* compatibility macros */
#define STILL_ACTIVE                        STATUS_PENDING
#define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT     STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND      STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT        STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION     STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW              STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK           STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW             STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO        STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW              STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION          STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR             STATUS_IN_PAGE_ERROR
#define EXCEPTION_ILLEGAL_INSTRUCTION       STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW            STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION       STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE                STATUS_GUARD_PAGE_VIOLATION
#define EXCEPTION_INVALID_HANDLE            STATUS_INVALID_HANDLE
#define EXCEPTION_POSSIBLE_DEADLOCK         STATUS_POSSIBLE_DEADLOCK
#define CONTROL_C_EXIT                      STATUS_CONTROL_C_EXIT

//
// Internal Exception Codes
//
#define KI_EXCEPTION_INTERNAL           0x10000000
#define KI_EXCEPTION_ACCESS_VIOLATION   (KI_EXCEPTION_INTERNAL | 0x04)

#define DBGK_MAX_MODULE_MSGS            500

#define PROCESS_TERMINATE                  (0x0001)  
#define PROCESS_CREATE_THREAD              (0x0002)  
#define PROCESS_SET_SESSIONID              (0x0004)  
#define PROCESS_VM_OPERATION               (0x0008)  
#define PROCESS_VM_READ                    (0x0010)  
#define PROCESS_VM_WRITE                   (0x0020)  
#define PROCESS_DUP_HANDLE                 (0x0040)  
#define PROCESS_CREATE_PROCESS             (0x0080)  
#define PROCESS_SET_QUOTA                  (0x0100)  
#define PROCESS_SET_INFORMATION            (0x0200)  
#define PROCESS_QUERY_INFORMATION          (0x0400)  
#define PROCESS_SUSPEND_RESUME             (0x0800)  
#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)  
#define PROCESS_SET_LIMITED_INFORMATION    (0x2000)  

#define DBGK_PROCESS_ALL_ACCESS SYNCHRONIZE | PROCESS_VM_WRITE | \
								PROCESS_VM_READ | PROCESS_VM_OPERATION | \
								PROCESS_TERMINATE| PROCESS_SUSPEND_RESUME | \
								PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION | \
								PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_QUERY_INFORMATION | \
								PROCESS_DUP_HANDLE| PROCESS_CREATE_THREAD | PROCESS_CREATE_PROCESS

//
#define THREAD_TERMINATE                 (0x0001)  
#define THREAD_SUSPEND_RESUME            (0x0002)  
#define THREAD_GET_CONTEXT               (0x0008)  
#define THREAD_SET_CONTEXT               (0x0010)  
#define THREAD_QUERY_INFORMATION         (0x0040)  
#define THREAD_SET_INFORMATION           (0x0020)  
#define THREAD_SET_THREAD_TOKEN          (0x0080)
#define THREAD_IMPERSONATE               (0x0100)
#define THREAD_DIRECT_IMPERSONATION      (0x0200)
#define THREAD_SET_LIMITED_INFORMATION   (0x0400)  // winnt
#define THREAD_QUERY_LIMITED_INFORMATION (0x0800)  // winnt
#define THREAD_RESUME                    (0x1000)  // winnt

#define DBGK_THREAD_ALL_ACCESS  SYNCHRONIZE | THREAD_DIRECT_IMPERSONATION | \
								THREAD_GET_CONTEXT | THREAD_IMPERSONATE | \
								THREAD_QUERY_INFORMATION | THREAD_QUERY_LIMITED_INFORMATION | \
								THREAD_SET_CONTEXT|THREAD_SET_INFORMATION | THREAD_SET_LIMITED_INFORMATION | \
								THREAD_SET_THREAD_TOKEN | THREAD_SUSPEND_RESUME | THREAD_TERMINATE

//
// Debug APIs
//
#define EXCEPTION_DEBUG_EVENT       1
#define CREATE_THREAD_DEBUG_EVENT   2
#define CREATE_PROCESS_DEBUG_EVENT  3
#define EXIT_THREAD_DEBUG_EVENT     4
#define EXIT_PROCESS_DEBUG_EVENT    5
#define LOAD_DLL_DEBUG_EVENT        6
#define UNLOAD_DLL_DEBUG_EVENT      7
#define OUTPUT_DEBUG_STRING_EVENT   8
#define RIP_EVENT                   9

#define DEBUG_OBJECT_DELETE_PENDING (0x1) // Debug object is delete pending.
#define DEBUG_OBJECT_KILL_ON_CLOSE  (0x2) // Kill all debugged processes on close

#define DEBUG_KILL_ON_CLOSE  (0x1) // Kill all debuggees on last handle close
#define PROCESS_SUSPEND_RESUME             (0x0800) 

#define DEBUG_EVENT_READ            (0x01)  // Event had been seen by win32 app
#define DEBUG_EVENT_NOWAIT          (0x02)  // No waiter on this. Just free the pool
#define DEBUG_EVENT_INACTIVE        (0x04)  // The message is in inactive. It may be activated or deleted later
#define DEBUG_EVENT_RELEASE         (0x08)  // Release rundown protection on this thread
#define DEBUG_EVENT_PROTECT_FAILED  (0x10)  // Rundown protection failed to be acquired on this thread
#define DEBUG_EVENT_SUSPEND         (0x20)  // Resume thread on continue
#define DEBUG_EVENT_SPECIAL         (0x40)  // Message may be redundant with a message generated during attach

#define DBGKP_API_SEND_SUSPEND          0x00000001UL    // Freeze the process on message send
#define DBGKP_API_SEND_SPECIAL_MSG      0x00000002UL    // Mark special messages for identification post attach

#define DBGKP_API_SEND_ALL_FLAGS    \
	(DBGKP_API_SEND_SUSPEND | DBGKP_API_SEND_SPECIAL_MSG)

//
// This is the name of the event used to signal when the system error port is registered.
// The user-mode reporting service (WerSvc) registers the error port when it is up and ready
// to serve requests.
//
#define SYSTEM_ERROR_PORT_READY_EVENT_NAME  L"\\KernelObjects\\SystemErrorPortReady"

//
// Portable LPC Types for 32/64-bit compatibility
//
#ifdef USE_LPC6432
	#define LPC_CLIENT_ID CLIENT_ID64
	#define LPC_SIZE_T ULONGLONG
	#define LPC_PVOID ULONGLONG
	#define LPC_HANDLE ULONGLONG
#else
	#define LPC_CLIENT_ID CLIENT_ID
	#define LPC_SIZE_T SIZE_T
	#define LPC_PVOID PVOID
	#define LPC_HANDLE HANDLE
#endif

// Used to signify that the delete APC has been queued or the
// thread has called PspExitThread itself.
//
#define PS_CROSS_THREAD_FLAGS_TERMINATED           0x00000001UL

//
// Thread create failed
//

#define PS_CROSS_THREAD_FLAGS_DEADTHREAD           0x00000002UL

//
// Debugger isn't shown this thread
//

#define PS_CROSS_THREAD_FLAGS_HIDEFROMDBG          0x00000004UL

//
// Thread is impersonating
//

#define PS_CROSS_THREAD_FLAGS_IMPERSONATING        0x00000008UL

//
// This is a system thread
//

#define PS_CROSS_THREAD_FLAGS_SYSTEM               0x00000010UL

//
// Hard errors are disabled for this thread
//

#define PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED 0x00000020UL

//
// We should break in when this thread is terminated
//

#define PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION 0x00000040UL

//
// This thread should skip sending its create thread message
//
#define PS_CROSS_THREAD_FLAGS_SKIP_CREATION_MSG    0x00000080UL

//
// This thread should skip sending its final thread termination message
//
#define PS_CROSS_THREAD_FLAGS_SKIP_TERMINATION_MSG 0x00000100UL

//
// Debug Object Access Masks
//
#define DEBUG_OBJECT_WAIT_STATE_CHANGE      0x0001
#define DEBUG_OBJECT_ADD_REMOVE_PROCESS     0x0002
#define DEBUG_OBJECT_SET_INFORMATION        0x0004
#define DEBUG_OBJECT_ALL_ACCESS             (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x0F)

#define PS_PROCESS_FLAGS_CREATE_REPORTED        0x00000001UL // Create process debug call has occurred
#define PS_PROCESS_FLAGS_NO_DEBUG_INHERIT       0x00000002UL // Don't inherit debug port
#define PS_PROCESS_FLAGS_PROCESS_EXITING        0x00000004UL // PspExitProcess entered
#define PS_PROCESS_FLAGS_PROCESS_DELETE         0x00000008UL // Delete process has been issued
#define PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES      0x00000010UL // Wow64 split pages
#define PS_PROCESS_FLAGS_VM_DELETED             0x00000020UL // VM is deleted
#define PS_PROCESS_FLAGS_OUTSWAP_ENABLED        0x00000040UL // Outswap enabled
#define PS_PROCESS_FLAGS_OUTSWAPPED             0x00000080UL // Outswapped
#define PS_PROCESS_FLAGS_FORK_FAILED            0x00000100UL // Fork status
#define PS_PROCESS_FLAGS_WOW64_4GB_VA_SPACE     0x00000200UL // Wow64 process with 4gb virtual address space
#define PS_PROCESS_FLAGS_ADDRESS_SPACE1         0x00000400UL // Addr space state1
#define PS_PROCESS_FLAGS_ADDRESS_SPACE2         0x00000800UL // Addr space state2
#define PS_PROCESS_FLAGS_SET_TIMER_RESOLUTION   0x00001000UL // SetTimerResolution has been called
#define PS_PROCESS_FLAGS_BREAK_ON_TERMINATION   0x00002000UL // Break on process termination
#define PS_PROCESS_FLAGS_CREATING_SESSION       0x00004000UL // Process is creating a session
#define PS_PROCESS_FLAGS_USING_WRITE_WATCH      0x00008000UL // Process is using the write watch APIs
#define PS_PROCESS_FLAGS_IN_SESSION             0x00010000UL // Process is in a session
#define PS_PROCESS_FLAGS_OVERRIDE_ADDRESS_SPACE 0x00020000UL // Process must use native address space (Win64 only)
#define PS_PROCESS_FLAGS_HAS_ADDRESS_SPACE      0x00040000UL // This process has an address space
#define PS_PROCESS_FLAGS_LAUNCH_PREFETCHED      0x00080000UL // Process launch was prefetched
#define PS_PROCESS_INJECT_INPAGE_ERRORS         0x00100000UL // Process should be given inpage errors - hardcoded in trap.asm too
#define PS_PROCESS_FLAGS_VM_TOP_DOWN            0x00200000UL // Process memory allocations default to top-down
#define PS_PROCESS_FLAGS_IMAGE_NOTIFY_DONE      0x00400000UL // We have sent a message for this image
#define PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED      0x00800000UL // The system PDEs need updating for this process (NT32 only)
#define PS_PROCESS_FLAGS_VDM_ALLOWED            0x01000000UL // Process allowed to invoke NTVDM support
#define PS_PROCESS_FLAGS_SMAP_ALLOWED           0x02000000UL // Process allowed to invoke SMAP support
#define PS_PROCESS_FLAGS_CREATE_FAILED          0x04000000UL // Process create failed

#define PS_PROCESS_FLAGS_DEFAULT_IO_PRIORITY    0x38000000UL // The default I/O priority for created threads. (3 bits)

#define PS_PROCESS_FLAGS_PRIORITY_SHIFT         27

#define PS_PROCESS_FLAGS_EXECUTE_SPARE1         0x40000000UL //
#define PS_PROCESS_FLAGS_EXECUTE_SPARE2         0x80000000UL //

//
// Define debug object access types. No security is present on this object.
//
#define DEBUG_READ_EVENT        (0x0001)
#define DEBUG_PROCESS_ASSIGN    (0x0002)
#define DEBUG_SET_INFORMATION   (0x0004)
#define DEBUG_QUERY_INFORMATION (0x0008)
#define DEBUG_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|DEBUG_READ_EVENT|DEBUG_PROCESS_ASSIGN|\
									DEBUG_SET_INFORMATION|DEBUG_QUERY_INFORMATION)



#define PS_SET_BITS(Flags, Flag) \
		RtlInterlockedSetBitsDiscardReturn (Flags, Flag)

#define PS_TEST_SET_BITS(Flags, Flag) \
		RtlInterlockedSetBits (Flags, Flag)

#define THREAD_TO_PROCESS(Thread) ((Thread)->ThreadsProcess)

#define POOL_QUOTA_FAIL_INSTEAD_OF_RAISE 8
#define POOL_RAISE_IF_ALLOCATION_FAILURE 16

typedef enum _LPC_TYPE {
	LPC_NEW_MESSAGE,
	LPC_REQUEST,
	LPC_REPLY,
	LPC_DATAGRAM,
	LPC_LOST_REPLY,
	LPC_PORT_CLOSED,
	LPC_CLIENT_DIED,
	LPC_EXCEPTION,
	LPC_DEBUG_EVENT,
	LPC_ERROR_EVENT,
	LPC_CONNECTION_REQUEST
} LPC_TYPE;

#define THREAD_QUERY_INFORMATION         (0x0040)  

#define PROCESS_TERMINATE                  (0x0001)  
#define PROCESS_CREATE_THREAD              (0x0002)   
#define PROCESS_VM_OPERATION               (0x0008)  
#define PROCESS_VM_READ                    (0x0010)  
#define PROCESS_VM_WRITE                   (0x0020)  
#define PROCESS_SET_INFORMATION            (0x0200)  
#define PROCESS_QUERY_INFORMATION          (0x0400) 


#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteHandle(
	IN PHANDLE Address
)

{

	if (Address >= (HANDLE* const)MM_USER_PROBE_ADDRESS) {
		Address = (HANDLE* const)MM_USER_PROBE_ADDRESS;
	}

	*((volatile HANDLE*)Address) = *Address;
	return;
}

#else

#define ProbeForWriteHandle(Address) {                                       \
	if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
		*(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
	}                                                                        \
																				\
	*(volatile HANDLE *)(Address) = *(volatile HANDLE *)(Address);           \
}

#endif


#if defined(_AMD64_)
FORCEINLINE
VOID
ProbeForWriteUlong(
	IN PULONG Address
) {
	if (Address >= (ULONG* const)MM_USER_PROBE_ADDRESS) {
		Address = (ULONG* const)MM_USER_PROBE_ADDRESS;
	}

	*((volatile ULONG*)Address) = *Address;
	return;
}
#else
#define ProbeForWriteUlong(Address) {                                        \
	if ((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) {                 \
		*(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
	}                                                                        \
																				\
	*(volatile ULONG *)(Address) = *(volatile ULONG *)(Address);             \
}

#endif

#define DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER(hdrs,field) \
		((hdrs)->OptionalHeader.field)

#define DBGKP_API_SEND_ALL_FLAGS	(0x3)

#define DBGKP_PORT_FLAGS_DELETE_V   0x0UL
#define DBGKP_PORT_FLAGS_DELETE     (1UL << DBGKP_PORT_FLAGS_DELETE_V)

#define DBGKP_ERROR_PORT_TAG        'PgbD'
#define DBGKP_MAX_ERROR_MSGS        32

#define DBGKP_ERROR_MSG_FILTER      0x00000001UL

typedef struct _DBGKP_ERROR_PORT
{
	__volatile ULONG RefCount;
	__volatile ULONG Flags;
	HANDLE PortHandle;          // ALPC error reporting handle
} DBGKP_ERROR_PORT, * PDBGKP_ERROR_PORT;

//
// Debug Object Information Structures
//
typedef struct _DEBUG_OBJECT_KILL_PROCESS_ON_EXIT_INFORMATION
{
	ULONG KillProcessOnExit;
} DEBUG_OBJECT_KILL_PROCESS_ON_EXIT_INFORMATION, * PDEBUG_OBJECT_KILL_PROCESS_ON_EXIT_INFORMATION;

typedef enum _SYSTEM_DLL_TYPE  // 7 elements, 0x4 bytes
{
	PsNativeSystemDll = 0 /*0x0*/,
	PsWowX86SystemDll = 1 /*0x1*/,
	PsWowArm32SystemDll = 2 /*0x2*/,
	PsWowAmd64SystemDll = 3 /*0x3*/,
	PsWowChpeX86SystemDll = 4 /*0x4*/,
	PsVsmEnclaveRuntimeDll = 5 /*0x5*/,
	PsSystemDllTotalTypes = 6 /*0x6*/
}SYSTEM_DLL_TYPE, * PSYSTEM_DLL_TYPE;

typedef struct _PEB_LDR_DATA                            // 9 elements, 0x58 bytes (sizeof) 
{
	ULONG32      Length;
	UINT8        Initialized;
	VOID* SsHandle;
	struct _LIST_ENTRY InLoadOrderModuleList;
	struct _LIST_ENTRY InMemoryOrderModuleList;
	struct _LIST_ENTRY InInitializationOrderModuleList;
	VOID* EntryInProgress;
	UINT8        ShutdownInProgress;
	VOID* ShutdownThreadId;
}PEB_LDR_DATA, * PPEB_LDR_DATA;

// symbols
typedef struct _LDR_SERVICE_TAG_RECORD
{
	struct _LDR_SERVICE_TAG_RECORD* Next;
	ULONG ServiceTag;
} LDR_SERVICE_TAG_RECORD, * PLDR_SERVICE_TAG_RECORD;

// symbols
typedef struct _LDRP_CSLIST
{
	PSINGLE_LIST_ENTRY Tail;
} LDRP_CSLIST, * PLDRP_CSLIST;

// symbols
typedef enum _LDR_DDAG_STATE
{
	LdrModulesMerged = -5,
	LdrModulesInitError = -4,
	LdrModulesSnapError = -3,
	LdrModulesUnloaded = -2,
	LdrModulesUnloading = -1,
	LdrModulesPlaceHolder = 0,
	LdrModulesMapping = 1,
	LdrModulesMapped = 2,
	LdrModulesWaitingForDependencies = 3,
	LdrModulesSnapping = 4,
	LdrModulesSnapped = 5,
	LdrModulesCondensed = 6,
	LdrModulesReadyToInit = 7,
	LdrModulesInitializing = 8,
	LdrModulesReadyToRun = 9
} LDR_DDAG_STATE;

// symbols
typedef struct _LDR_DDAG_NODE
{
	LIST_ENTRY Modules;
	PLDR_SERVICE_TAG_RECORD ServiceTagList;
	ULONG LoadCount;
	ULONG LoadWhileUnloadingCount;
	ULONG LowestLink;
	union
	{
		LDRP_CSLIST Dependencies;
		SINGLE_LIST_ENTRY RemovalLink;
	};
	LDRP_CSLIST IncomingDependencies;
	LDR_DDAG_STATE State;
	SINGLE_LIST_ENTRY CondenseLink;
	ULONG PreorderNumber;
} LDR_DDAG_NODE, * PLDR_DDAG_NODE;

// rev
typedef struct _LDR_DEPENDENCY_RECORD
{
	SINGLE_LIST_ENTRY DependencyLink;
	PLDR_DDAG_NODE DependencyNode;
	SINGLE_LIST_ENTRY IncomingDependencyLink;
	PLDR_DDAG_NODE IncomingDependencyNode;
} LDR_DEPENDENCY_RECORD, * PLDR_DEPENDENCY_RECORD;

// symbols
typedef enum _LDR_DLL_LOAD_REASON
{
	LoadReasonStaticDependency,
	LoadReasonStaticForwarderDependency,
	LoadReasonDynamicForwarderDependency,
	LoadReasonDelayloadDependency,
	LoadReasonDynamicLoad,
	LoadReasonAsImageLoad,
	LoadReasonAsDataLoad,
	LoadReasonEnclavePrimary, // REDSTONE3
	LoadReasonEnclaveDependency,
	LoadReasonUnknown = -1
} LDR_DLL_LOAD_REASON, * PLDR_DLL_LOAD_REASON;

// typedef struct _LDR_DATA_TABLE_ENTRY
// {
// 	LIST_ENTRY InLoadOrderLinks;
// 	LIST_ENTRY InMemoryOrderLinks;
// 	union
// 	{
// 		LIST_ENTRY InInitializationOrderLinks;
// 		LIST_ENTRY InProgressLinks;
// 	};
// 	PVOID DllBase;
// 	PLDR_INIT_ROUTINE EntryPoint;
// 	ULONG SizeOfImage;
// 	UNICODE_STRING FullDllName;
// 	UNICODE_STRING BaseDllName;
// 	union
// 	{
// 		UCHAR FlagGroup[4];
// 		ULONG Flags;
// 		struct
// 		{
// 			ULONG PackagedBinary : 1;
// 			ULONG MarkedForRemoval : 1;
// 			ULONG ImageDll : 1;
// 			ULONG LoadNotificationsSent : 1;
// 			ULONG TelemetryEntryProcessed : 1;
// 			ULONG ProcessStaticImport : 1;
// 			ULONG InLegacyLists : 1;
// 			ULONG InIndexes : 1;
// 			ULONG ShimDll : 1;
// 			ULONG InExceptionTable : 1;
// 			ULONG ReservedFlags1 : 2;
// 			ULONG LoadInProgress : 1;
// 			ULONG LoadConfigProcessed : 1;
// 			ULONG EntryProcessed : 1;
// 			ULONG ProtectDelayLoad : 1;
// 			ULONG ReservedFlags3 : 2;
// 			ULONG DontCallForThreads : 1;
// 			ULONG ProcessAttachCalled : 1;
// 			ULONG ProcessAttachFailed : 1;
// 			ULONG CorDeferredValidate : 1;
// 			ULONG CorImage : 1;
// 			ULONG DontRelocate : 1;
// 			ULONG CorILOnly : 1;
// 			ULONG ChpeImage : 1;
// 			ULONG ReservedFlags5 : 2;
// 			ULONG Redirected : 1;
// 			ULONG ReservedFlags6 : 2;
// 			ULONG CompatDatabaseProcessed : 1;
// 		};
// 	};
// 	USHORT ObsoleteLoadCount;
// 	USHORT TlsIndex;
// 	LIST_ENTRY HashLinks;
// 	ULONG TimeDateStamp;
// 	struct _ACTIVATION_CONTEXT* EntryPointActivationContext;
// 	PVOID Lock; // RtlAcquireSRWLockExclusive
// 	PLDR_DDAG_NODE DdagNode;
// 	LIST_ENTRY NodeModuleLink;
// 	struct _LDRP_LOAD_CONTEXT* LoadContext;
// 	PVOID ParentDllBase;
// 	PVOID SwitchBackContext;
// 	RTL_BALANCED_NODE BaseAddressIndexNode;
// 	RTL_BALANCED_NODE MappingInfoIndexNode;
// 	ULONG_PTR OriginalBase;
// 	LARGE_INTEGER LoadTime;
// 	ULONG BaseNameHashValue;
// 	LDR_DLL_LOAD_REASON LoadReason;
// 	ULONG ImplicitPathOptions;
// 	ULONG ReferenceCount;
// 	ULONG DependentLoadFlags;
// 	UCHAR SigningLevel; // since REDSTONE2
// } LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

//
// Debug Message API Number
// 
// 指定是哪种事件
typedef enum _DBGKM_APINUMBER
{
	DbgKmExceptionApi = 0,		// 异常
	DbgKmCreateThreadApi = 1,	// 创建线程
	DbgKmCreateProcessApi = 2,	// 创建进程
	DbgKmExitThreadApi = 3,		// 线程退出
	DbgKmExitProcessApi = 4,	// 进程退出
	DbgKmLoadDllApi = 5,		// 加载DLL
	DbgKmUnloadDllApi = 6,		// 卸载DLL
	DbgKmErrorReportApi = 7,	// 内部错误
	DbgKmMaxApiNumber = 8,		// 这组常量的最大值
} DBGKM_APINUMBER;

// 异常消息
typedef struct _DBGKM_EXCEPTION
{
	EXCEPTION_RECORD ExceptionRecord;
	ULONG FirstChance;
} DBGKM_EXCEPTION, * PDBGKM_EXCEPTION;

// 创建线程消息
typedef struct _DBGKM_CREATE_THREAD
{
	ULONG SubSystemKey;
	PVOID StartAddress;
} DBGKM_CREATE_THREAD, * PDBGKM_CREATE_THREAD;

// 创建进程消息
typedef struct _DBGKM_CREATE_PROCESS
{
	ULONG SubSystemKey;
	HANDLE FileHandle;
	PVOID BaseOfImage;
	ULONG DebugInfoFileOffset;
	ULONG DebugInfoSize;
	DBGKM_CREATE_THREAD InitialThread;
} DBGKM_CREATE_PROCESS, * PDBGKM_CREATE_PROCESS;

// 退出线程消息
typedef struct _DBGKM_EXIT_THREAD
{
	NTSTATUS ExitStatus;
} DBGKM_EXIT_THREAD, * PDBGKM_EXIT_THREAD;

// 退出进程消息
typedef struct _DBGKM_EXIT_PROCESS
{
	NTSTATUS ExitStatus;
} DBGKM_EXIT_PROCESS, * PDBGKM_EXIT_PROCESS;

// 加载模块消息
typedef struct _DBGKM_LOAD_DLL
{
	HANDLE FileHandle;
	PVOID BaseOfDll;
	ULONG DebugInfoFileOffset;
	ULONG DebugInfoSize;
	PVOID NamePointer;
} DBGKM_LOAD_DLL, * PDBGKM_LOAD_DLL;

// 卸载模块消息
typedef struct _DBGKM_UNLOAD_DLL
{
	PVOID BaseAddress;
} DBGKM_UNLOAD_DLL, * PDBGKM_UNLOAD_DLL;

// 消息结构
typedef struct _DBGKM_APIMSG {
	wdk::PORT_MESSAGE h;
	DBGKM_APINUMBER ApiNumber;
	NTSTATUS ReturnedStatus;
	union {
		DBGKM_EXCEPTION Exception;
		DBGKM_CREATE_THREAD CreateThread;
		DBGKM_CREATE_PROCESS CreateProcess;
		DBGKM_EXIT_THREAD ExitThread;
		DBGKM_EXIT_PROCESS ExitProcess;
		DBGKM_LOAD_DLL LoadDll;
		DBGKM_UNLOAD_DLL UnloadDll;
	} u;
	//以上这个部分占了0x74个大小，而windows7此结构的大小是A8，下面应该是输入异常相关的信息，为此，我们要凑够0xA8个大小，不然处理异常的时候会蓝屏掉
	//UCHAR unknow[0x40];
} DBGKM_APIMSG, * PDBGKM_APIMSG;

//
// LPC Port Message
//
typedef struct _LPC_PORT_MESSAGE
{
	union
	{
		struct
		{
			CSHORT DataLength;
			CSHORT TotalLength;
		} s1;
		ULONG Length;
	} u1;
	union
	{
		struct
		{
			CSHORT Type;
			CSHORT DataInfoOffset;
		} s2;
		ULONG ZeroInit;
	} u2;
	union
	{
		CLIENT_ID ClientId;
		double DoNotUseThisField;
	};
	ULONG MessageId;
	union
	{
		LPC_SIZE_T ClientViewSize;
		ULONG CallbackId;
	};
} LPC_PORT_MESSAGE, * PLPC_PORT_MESSAGE;

typedef struct _DBGKM_ERROR_MSG {
	EXCEPTION_RECORD ExceptionRecord;
	wdk::SECTION_IMAGE_INFORMATION ImageInfo;
	union
	{
		ULONG Flags;
		struct {
			ULONG IsProtectedProcess : 1;
			ULONG IsWow64Process : 1;
			ULONG IsFilterMessage : 1;
			ULONG SpareBits : 29;
		};
	};
}DBGKM_ERROR_MSG, * PDBGKM_ERROR_MSG;

//
// DbgKm Apis are from the kernel component (Dbgk) through a process
// debug port.
//

#define DBGKM_MSG_OVERHEAD \
	(FIELD_OFFSET(DBGKM_APIMSG, u.Exception) - sizeof(LPC_PORT_MESSAGE))

#define DBGKM_API_MSG_LENGTH(TypeSize) \
	((sizeof(DBGKM_APIMSG) << 16) | (DBGKM_MSG_OVERHEAD + (TypeSize)))

#define DBGKM_FORMAT_API_MSG(m,Number,TypeSize)             \
	(m).h.u1.Length = DBGKM_API_MSG_LENGTH((TypeSize));     \
	(m).h.u2.ZeroInit = LPC_DEBUG_EVENT;                    \
	(m).ApiNumber = (Number)


typedef struct _PS_SYSTEM_DLL_INFO {

	//
	// Flags.
	// Initialized statically.
	// 

	USHORT        Flags;

	//
	// Machine type of this WoW64 NTDLL.
	// Initialized statically.
	// Examples:
	//   - IMAGE_FILE_MACHINE_I386
	//   - IMAGE_FILE_MACHINE_ARMNT
	//

	USHORT        MachineType;

	//
	// Unused, always 0.
	//

	ULONG         Reserved1;

	//
	// Path to the WoW64 NTDLL.
	// Initialized statically.
	// Examples:
	//   - "\\SystemRoot\\SysWOW64\\ntdll.dll"
	//   - "\\SystemRoot\\SysArm32\\ntdll.dll"
	//

	UNICODE_STRING Ntdll32Path;

	//
	// Image base of the DLL.
	// Initialized at runtime by PspMapSystemDll.
	// Equivalent of:
	//      RtlImageNtHeader(BaseAddress)->
	//          OptionalHeader.ImageBase;
	//

	PVOID         ImageBase;

	//
	// Contains DLL name (such as "ntdll.dll" or
	// "ntdll32.dll") before runtime initialization.
	// Initialized at runtime by MmMapViewOfSectionEx,
	// called from PspMapSystemDll.
	//

	union {
		PVOID       BaseAddress;
		PWCHAR      DllName;
	};

	//
	// Unused, always 0.
	//

	PVOID         Reserved2;

	//
	// Section relocation information.
	//

	PVOID         SectionRelocationInformation;

	//
	// Unused, always 0.
	//

	PVOID         Reserved3;

} PS_SYSTEM_DLL_INFO, * PPS_SYSTEM_DLL_INFO;

typedef struct _PS_SYSTEM_DLL {
	//
	// _SECTION* object of the DLL.
	// Initialized at runtime by PspLocateSystemDll.
	//
	union {
		wdk::EX_FAST_REF SectionObjectFastRef;
		PVOID       SectionObject;
	};

	//
	// Push lock.
	//

	EX_PUSH_LOCK  PushLock;

	//
	// System DLL information.
	// This part is returned by PsQuerySystemDllInfo.
	//

	PS_SYSTEM_DLL_INFO SystemDllInfo;
}PS_SYSTEM_DLL, * PPS_SYSTEM_DLL;


typedef struct _DEBUG_EVENT
{
	LIST_ENTRY EventList;      // Queued to event object through this
	KEVENT ContinueEvent;
	CLIENT_ID ClientId;
	PEPROCESS Process;         // Waiting process
	PETHREAD Thread;           // Waiting thread
	NTSTATUS Status;           // Status of operation
	ULONG Flags;
	PETHREAD BackoutThread;    // Backout key for faked messages
	DBGKM_APIMSG ApiMsg;       // Message being sent
} DEBUG_EVENT, * PDEBUG_EVENT;

//调试对象
typedef struct _DEBUG_OBJECT
{
	KEVENT EventsPresent;	// 指示有调试事件发生
	FAST_MUTEX Mutex;		// 用于同步的互斥对象
	LIST_ENTRY EventList;	// 保持调试事件的链表，调试消息队列
	union
	{
		ULONG Flags;
		struct
		{
			UCHAR DebuggerInactive : 1;
			UCHAR KillProcessOnExit : 1;
		};
	};
} DEBUG_OBJECT, * PDEBUG_OBJECT;

typedef struct _MINI_CONTEXT_WOW64
{
	BOOLEAN initializated;  //用户判断shadow context是否已经初始化
	/* 0x0004 */ unsigned long Dr0;
	/* 0x0008 */ unsigned long Dr1;
	/* 0x000c */ unsigned long Dr2;
	/* 0x0010 */ unsigned long Dr3;
	/* 0x0014 */ unsigned long Dr6;
	/* 0x0018 */ unsigned long Dr7;
	/* 0x00c0 */ unsigned long EFlags;
}MINI_CONTEXT_WOW64, * PMINI_CONTEXT_WOW64;

typedef struct _MINI_CONTEXT
{
	BOOLEAN initializated;  //用户判断shadow context是否已经初始化
	ULONG64 Dr0;
	ULONG64 Dr1;
	ULONG64 Dr2;
	ULONG64 Dr3;
	ULONG64 Dr6;
	ULONG64 Dr7;
	ULONG EFlags;
}MINI_CONTEXT, * PMINI_CONTEXT;

//调试进程对象
typedef struct _DEBUG_PROCESS
{
	LIST_ENTRY list_entry;
	FAST_MUTEX Mutex;           //互斥锁
	_EPROCESS* Process;         //进程对象
	PDEBUG_OBJECT DebugObject;  //调试对象
	MINI_CONTEXT Context;
	MINI_CONTEXT_WOW64 wow64Context;
}DEBUG_PROCESS, * PDEBUG_PROCESS;

//
// Debug States
//
typedef enum _DBG_STATE {
	DbgIdle,
	DbgReplyPending,
	DbgCreateThreadStateChange,
	DbgCreateProcessStateChange,
	DbgExitThreadStateChange,
	DbgExitProcessStateChange,
	DbgExceptionStateChange,
	DbgBreakpointStateChange,
	DbgSingleStepStateChange,
	DbgLoadDllStateChange,
	DbgUnloadDllStateChange
} DBG_STATE, * PDBG_STATE;

// typedef enum _DEBUGOBJECTINFOCLASS {
// 	DebugObjectFlags = 1,
// 	MaxDebugObjectInfoClass
// } DEBUGOBJECTINFOCLASS, * PDEBUGOBJECTINFOCLASS;

typedef struct _DBGUI_CREATE_THREAD {
	HANDLE HandleToThread;
	DBGKM_CREATE_THREAD NewThread;
} DBGUI_CREATE_THREAD, * PDBGUI_CREATE_THREAD;

typedef struct _DBGUI_CREATE_PROCESS {
	HANDLE HandleToProcess;
	HANDLE HandleToThread;
	DBGKM_CREATE_PROCESS NewProcess;
} DBGUI_CREATE_PROCESS, * PDBGUI_CREATE_PROCESS;

//
// User-Mode Debug State Change Structure
//
typedef struct _DBGUI_WAIT_STATE_CHANGE {
	DBG_STATE NewState;
	CLIENT_ID AppClientId;
	union {
		DBGKM_EXCEPTION Exception;
		DBGUI_CREATE_THREAD CreateThread;
		DBGUI_CREATE_PROCESS CreateProcessInfo;
		DBGKM_EXIT_THREAD ExitThread;
		DBGKM_EXIT_PROCESS ExitProcess;
		DBGKM_LOAD_DLL LoadDll;
		DBGKM_UNLOAD_DLL UnloadDll;
	} StateInfo;
} DBGUI_WAIT_STATE_CHANGE, * PDBGUI_WAIT_STATE_CHANGE;

typedef enum _DEBUG_OBJECT_INFORMATION_CLASS {
	DebugObjectFlagsInformation = 1,
	DebugObjectMaximumInfomation
}DEBUG_OBJECT_INFORMATION_CLASS, * PDEBUG_OBJECT_INFORMATION_CLASS;

typedef ULONG MM_PROTECTION_MASK;

// typedef struct _EX_FAST_REF      // 3 elements, 0x4 bytes (sizeof) 
// {
// 	union                        // 3 elements, 0x4 bytes (sizeof) 
// 	{
// 		/*0x000*/         PVOID Object;
// 		/*0x000*/         ULONG32      RefCnt : 4; // 0 BitPosition                  
// 		/*0x000*/         ULONG_PTR      Value;
// 	};
// }EX_FAST_REF, * PEX_FAST_REF;


//
// Push lock definitions
//
typedef struct _EX_PUSH_LOCK_S {

	//
	// LOCK bit is set for both exclusive and shared acquires
	//
#define EX_PUSH_LOCK_LOCK_V          ((ULONG_PTR)0x0)
#define EX_PUSH_LOCK_LOCK            ((ULONG_PTR)0x1)

//
// Waiting bit designates that the pointer has chained waiters
//

#define EX_PUSH_LOCK_WAITING         ((ULONG_PTR)0x2)

//
// Waking bit designates that we are either traversing the list
// to wake threads or optimizing the list
//

#define EX_PUSH_LOCK_WAKING          ((ULONG_PTR)0x4)

//
// Set if the lock is held shared by multiple owners and there are waiters
//

#define EX_PUSH_LOCK_MULTIPLE_SHARED ((ULONG_PTR)0x8)

//
// Total shared Acquires are incremented using this
//
#define EX_PUSH_LOCK_SHARE_INC       ((ULONG_PTR)0x10)
#define EX_PUSH_LOCK_PTR_BITS        ((ULONG_PTR)0xf)

	union {
		struct {
			ULONG_PTR Locked : 1;
			ULONG_PTR Waiting : 1;
			ULONG_PTR Waking : 1;
			ULONG_PTR MultipleShared : 1;
			ULONG_PTR Shared : sizeof(ULONG_PTR) * 8 - 4;
		};
		ULONG_PTR Value;
		PVOID Ptr;
	};
} EX_PUSH_LOCK_S, * PEX_PUSH_LOCK_S;

// typedef struct _EX_FAST_REF {
// 	union {
// 		PVOID Object;
// #if defined (_WIN64)
// 		ULONG_PTR RefCnt : 4;
// #else
// 		ULONG_PTR RefCnt : 3;
// #endif
// 		ULONG_PTR Value;
// 	};
// } EX_FAST_REF, * PEX_FAST_REF;

typedef struct _CONTROL_AREA                                      // 16 elements, 0x50 bytes (sizeof) 
{
	/*0x000*/     void* Segment;
	/*0x004*/     struct _LIST_ENTRY DereferenceList;                           // 2 elements, 0x8 bytes (sizeof)   
	/*0x00C*/     ULONG64      NumberOfSectionReferences;
	/*0x010*/     ULONG64      NumberOfPfnReferences;
	/*0x014*/     ULONG64      NumberOfMappedViews;
	/*0x018*/     ULONG64      NumberOfUserReferences;
	union                                                         // 2 elements, 0x4 bytes (sizeof)   
	{
		/*0x01C*/         ULONG32      LongFlags;
		/*0x01C*/         ULONG32	   Flags;                            // 27 elements, 0x4 bytes (sizeof)  
	}u;
	/*0x020*/     ULONG32      FlushInProgressCount;
	/*0x024*/     struct wdk::_EX_FAST_REF FilePointer;                              // 3 elements, 0x4 bytes (sizeof)   
	/*0x028*/     LONG32       ControlAreaLock;
	union                                                         // 2 elements, 0x4 bytes (sizeof)   
	{
		/*0x02C*/         ULONG32      ModifiedWriteCount;
		/*0x02C*/         ULONG32      StartingFrame;
	};
	/*0x030*/     void* WaitingForDeletion;
	union                                                         // 1 elements, 0xC bytes (sizeof)   
	{
		struct                                                    // 9 elements, 0xC bytes (sizeof)   
		{
			union                                                 // 2 elements, 0x4 bytes (sizeof)   
			{
				/*0x034*/                 ULONG32      NumberOfSystemCacheViews;
				/*0x034*/                 ULONG32      ImageRelocationStartBit;
			};
			union                                                 // 2 elements, 0x4 bytes (sizeof)   
			{
				/*0x038*/                 LONG32       WritableUserReferences;
				struct                                            // 4 elements, 0x4 bytes (sizeof)   
				{
					/*0x038*/                     ULONG32      ImageRelocationSizeIn64k : 16;   // 0 BitPosition                    
					/*0x038*/                     ULONG32      Unused : 14;                     // 16 BitPosition                   
					/*0x038*/                     ULONG32      BitMap64 : 1;                    // 30 BitPosition                   
					/*0x038*/                     ULONG32      ImageActive : 1;                 // 31 BitPosition                   
				};
			};
			union                                                 // 2 elements, 0x4 bytes (sizeof)   
			{
				/*0x03C*/                 void* SubsectionRoot;
				/*0x03C*/                 void* SeImageStub;
			};
		}e2;
	}u2;
	/*0x040*/     INT64        LockedPages;
	/*0x048*/     struct _LIST_ENTRY ViewList;                                  // 2 elements, 0x8 bytes (sizeof)   
}CONTROL_AREA, * PCONTROL_AREA;


typedef struct _MMSECTION_FLAGS {
	unsigned BeingDeleted : 1;
	unsigned BeingCreated : 1;
	unsigned BeingPurged : 1;
	unsigned NoModifiedWriting : 1;

	unsigned FailAllIo : 1;
	unsigned Image : 1;
	unsigned Based : 1;
	unsigned File : 1;

	unsigned Networked : 1;
	unsigned NoCache : 1;
	unsigned PhysicalMemory : 1;
	unsigned CopyOnWrite : 1;

	unsigned Reserve : 1;  // not a spare bit!
	unsigned Commit : 1;
	unsigned FloppyMedia : 1;
	unsigned WasPurged : 1;

	unsigned UserReference : 1;
	unsigned GlobalMemory : 1;
	unsigned DeleteOnClose : 1;
	unsigned FilePointerNull : 1;

	unsigned DebugSymbolsLoaded : 1;
	unsigned SetMappedFileIoComplete : 1;
	unsigned CollidedFlush : 1;
	unsigned NoChange : 1;

	unsigned filler0 : 1;
	unsigned ImageMappedInSystemSpace : 1;
	unsigned UserWritable : 1;
	unsigned Accessed : 1;

	unsigned GlobalOnlyPerSession : 1;
	unsigned Rom : 1;
	unsigned WriteCombined : 1;
	unsigned filler : 1;
} MMSECTION_FLAGS;

typedef struct _SEGMENT_FLAGS
{
	union
	{
		struct
		{
			USHORT TotalNumberOfPtes4132 : 10;                                //0x0
			USHORT Spare0 : 1;                                                //0x0
			USHORT SessionDriverProtos : 1;                                   //0x0
			USHORT LargePages : 1;                                            //0x0
			USHORT DebugSymbolsLoaded : 1;                                    //0x0
			USHORT WriteCombined : 1;                                         //0x0
			USHORT NoCache : 1;                                               //0x0
		};
		USHORT Short0;                                                      //0x0
	};
	union
	{
		struct
		{
			UCHAR Spare : 1;                                                  //0x2
			UCHAR DefaultProtectionMask : 5;                                  //0x2
			UCHAR Binary32 : 1;                                               //0x2
			UCHAR ContainsDebug : 1;                                          //0x2
		};
		UCHAR UChar1;                                                       //0x2
	};
	union
	{
		struct
		{
			UCHAR ForceCollision : 1;                                         //0x3
			UCHAR ImageSigningType : 3;                                       //0x3
			UCHAR ImageSigningLevel : 4;                                      //0x3
		};
		UCHAR UChar2;                                                       //0x3
	};
} SEGMENT_FLAGS;

typedef struct _SEGMENT {
	struct _CONTROL_AREA* ControlArea;                                      //0x0
	ULONG TotalNumberOfPtes;                                                //0x8
	struct _SEGMENT_FLAGS SegmentFlags;                                     //0xc
	ULONGLONG NumberOfCommittedPages;                                       //0x10
	ULONGLONG SizeOfSegment;                                                //0x18
	//...
} SEGMENT, * PSEGMENT;

typedef struct _MMADDRESS_NODE {
	union {
		LONG_PTR Balance : 2;
		struct _MMADDRESS_NODE* Parent;
	} u1;
	struct _MMADDRESS_NODE* LeftChild;
	struct _MMADDRESS_NODE* RightChild;
	ULONG_PTR StartingVpn;
	ULONG_PTR EndingVpn;
} MMADDRESS_NODE, * PMMADDRESS_NODE;

typedef struct _SECTION {
	MMADDRESS_NODE Address;
	PSEGMENT Segment;
	LARGE_INTEGER SizeOfSection;
	union {
		ULONG LongFlags;
		MMSECTION_FLAGS Flags;
	} u;
	MM_PROTECTION_MASK InitialPageProtection;
} SECTION, * PSECTION;


typedef struct _INVERTED_FUNCTION_TABLE_ENTRY                // 5 elements, 0x18 bytes (sizeof) 
{
	union                                                    // 2 elements, 0x8 bytes (sizeof)  
	{
		/*0x000*/         struct _IMAGE_RUNTIME_FUNCTION_ENTRY* FunctionTable;
		/*0x000*/         struct _DYNAMIC_FUNCTION_TABLE* DynamicTable;
	};
	/*0x008*/     VOID* ImageBase;
	/*0x010*/     ULONG32      SizeOfImage;
	/*0x014*/     ULONG32      SizeOfTable;
}INVERTED_FUNCTION_TABLE_ENTRY, * PINVERTED_FUNCTION_TABLE_ENTRY;

typedef struct _INVERTED_FUNCTION_TABLE                    // 5 elements, 0x1810 bytes (sizeof) 
{
	/*0x000*/     ULONG32      CurrentSize;
	/*0x004*/     ULONG32      MaximumSize;
	/*0x008*/     ULONG32      Epoch;
	/*0x00C*/     UINT8        Overflow;
	/*0x00D*/     UINT8        _PADDING0_[0x3];
	/*0x010*/     struct _INVERTED_FUNCTION_TABLE_ENTRY TableEntry[256];
}INVERTED_FUNCTION_TABLE, * PINVERTED_FUNCTION_TABLE;

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForReadSmallStructure(
	IN PVOID Address,
	IN SIZE_T Size,
	IN ULONG Alignment
)

/*++
Routine Description:
	Probes a structure for read access whose size is known at compile time.
	N.B. A NULL structure address is not allowed.
Arguments:
	Address - Supples a pointer to the structure.
	Size - Supplies the size of the structure.
	Alignment - Supplies the alignment of structure.
Return Value:
	None
--*/

{

	ASSERT((Alignment == 1) || (Alignment == 2) ||
		(Alignment == 4) || (Alignment == 8) ||
		(Alignment == 16));

	if ((Size == 0) || (Size >= 0x10000)) {

		ASSERT(0);

		ProbeForRead(Address, Size, Alignment);

	}
	else {
		if (((ULONG_PTR)Address & (Alignment - 1)) != 0) {
			ExRaiseDatatypeMisalignment();
		}

		if ((PUCHAR)Address >= (UCHAR* const)MM_USER_PROBE_ADDRESS) {
			Address = (UCHAR* const)MM_USER_PROBE_ADDRESS;
		}

		_ReadWriteBarrier();
		*(volatile UCHAR*)Address;
	}
}

#else

#define ProbeForReadSmallStructure(Address, Size, Alignment) {               \
	ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                       \
			((Alignment) == 4) || ((Alignment) == 8) ||                       \
			((Alignment) == 16));                                             \
	if ((Size == 0) || (Size > 0x10000)) {                                   \
		ASSERT(0);                                                           \
		ProbeForRead(Address, Size, Alignment);                              \
	} else {                                                                 \
		if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {               \
			ExRaiseDatatypeMisalignment();                                   \
		}                                                                    \
		if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {      \
			*(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;              \
		}                                                                    \
	}                                                                        \
}

#endif


FORCEINLINE
VOID
ProbeForWriteSmallStructure(
	IN PVOID Address,
	IN SIZE_T Size,
	IN ULONG Alignment
)

/*++
Routine Description:
	Probes a structure for write access whose size is known at compile time.
Arguments:
	Address - Supples a pointer to the structure.
	Size - Supplies the size of the structure.
	Alignment - Supplies the alignment of structure.
Return Value:
	None
--*/

{

	ASSERT((Alignment == 1) || (Alignment == 2) ||
		(Alignment == 4) || (Alignment == 8) ||
		(Alignment == 16));

	//
	// If the size of the structure is > 4k then call the standard routine.
	// wow64 uses a page size of 4k even on ia64.
	//

	if ((Size == 0) || (Size >= 0x1000)) {

		ASSERT(0);

		ProbeForWrite(Address, Size, Alignment);

	}
	else {
		if (((ULONG_PTR)(Address) & (Alignment - 1)) != 0) {
			ExRaiseDatatypeMisalignment();
		}

#if defined(_AMD64_)

		if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {
			Address = (UCHAR* const)MM_USER_PROBE_ADDRESS;
		}

		((volatile UCHAR*)(Address))[0] = ((volatile UCHAR*)(Address))[0];
		((volatile UCHAR*)(Address))[Size - 1] = ((volatile UCHAR*)(Address))[Size - 1];

#else

		if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {
			*((volatile UCHAR* const)MM_USER_PROBE_ADDRESS) = 0;
		}

		*(volatile UCHAR*)(Address) = *(volatile UCHAR*)(Address);
		if (Size > Alignment) {
			((volatile UCHAR*)(Address))[(Size - 1) & ~(SIZE_T)(Alignment - 1)] =
				((volatile UCHAR*)(Address))[(Size - 1) & ~(SIZE_T)(Alignment - 1)];
		}

#endif

	}
}
