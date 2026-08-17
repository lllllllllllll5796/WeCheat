#pragma once

typedef char i8; 
typedef unsigned char u8;

typedef int i32;
typedef unsigned int u32;

typedef short i16;
typedef unsigned short u16;

typedef __int64 i64;
typedef unsigned __int64 u64;

typedef u8* pb64;
typedef void* pv64;

// typedef enum _SYSTEM_INFORMATION_CLASS
// {
// 	SystemBasicInformation = 0x0,
// 	SystemProcessorInformation = 0x1,
// 	SystemPerformanceInformation = 0x2,
// 	SystemTimeOfDayInformation = 0x3,
// 	SystemPathInformation = 0x4,
// 	SystemProcessInformation = 0x5,
// 	SystemCallCountInformation = 0x6,
// 	SystemDeviceInformation = 0x7,
// 	SystemProcessorPerformanceInformation = 0x8,
// 	SystemFlagsInformation = 0x9,
// 	SystemCallTimeInformation = 0xa,
// 	SystemModuleInformation = 0xb,
// 	SystemLocksInformation = 0xc,
// 	SystemStackTraceInformation = 0xd,
// 	SystemPagedPoolInformation = 0xe,
// 	SystemNonPagedPoolInformation = 0xf,
// 	SystemHandleInformation = 0x10,
// 	SystemObjectInformation = 0x11,
// 	SystemPageFileInformation = 0x12,
// 	SystemVdmInstemulInformation = 0x13,
// 	SystemVdmBopInformation = 0x14,
// 	SystemFileCacheInformation = 0x15,
// 	SystemPoolTagInformation = 0x16,
// 	SystemInterruptInformation = 0x17,
// 	SystemDpcBehaviorInformation = 0x18,
// 	SystemFullMemoryInformation = 0x19,
// 	SystemLoadGdiDriverInformation = 0x1a,
// 	SystemUnloadGdiDriverInformation = 0x1b,
// 	SystemTimeAdjustmentInformation = 0x1c,
// 	SystemSummaryMemoryInformation = 0x1d,
// 	SystemMirrorMemoryInformation = 0x1e,
// 	SystemPerformanceTraceInformation = 0x1f,
// 	SystemObsolete0 = 0x20,
// 	SystemExceptionInformation = 0x21,
// 	SystemCrashDumpStateInformation = 0x22,
// 	SystemKernelDebuggerInformation = 0x23,
// 	SystemContextSwitchInformation = 0x24,
// 	SystemRegistryQuotaInformation = 0x25,
// 	SystemExtendServiceTableInformation = 0x26,
// 	SystemPrioritySeperation = 0x27,
// 	SystemVerifierAddDriverInformation = 0x28,
// 	SystemVerifierRemoveDriverInformation = 0x29,
// 	SystemProcessorIdleInformation = 0x2a,
// 	SystemLegacyDriverInformation = 0x2b,
// 	SystemCurrentTimeZoneInformation = 0x2c,
// 	SystemLookasideInformation = 0x2d,
// 	SystemTimeSlipNotification = 0x2e,
// 	SystemSessionCreate = 0x2f,
// 	SystemSessionDetach = 0x30,
// 	SystemSessionInformation = 0x31,
// 	SystemRangeStartInformation = 0x32,
// 	SystemVerifierInformation = 0x33,
// 	SystemVerifierThunkExtend = 0x34,
// 	SystemSessionProcessInformation = 0x35,
// 	SystemLoadGdiDriverInSystemSpace = 0x36,
// 	SystemNumaProcessorMap = 0x37,
// 	SystemPrefetcherInformation = 0x38,
// 	SystemExtendedProcessInformation = 0x39,
// 	SystemRecommendedSharedDataAlignment = 0x3a,
// 	SystemComPlusPackage = 0x3b,
// 	SystemNumaAvailableMemory = 0x3c,
// 	SystemProcessorPowerInformation = 0x3d,
// 	SystemEmulationBasicInformation = 0x3e,
// 	SystemEmulationProcessorInformation = 0x3f,
// 	SystemExtendedHandleInformation = 0x40,
// 	SystemLostDelayedWriteInformation = 0x41,
// 	SystemBigPoolInformation = 0x42,
// 	SystemSessionPoolTagInformation = 0x43,
// 	SystemSessionMappedViewInformation = 0x44,
// 	SystemHotpatchInformation = 0x45,
// 	SystemObjectSecurityMode = 0x46,
// 	SystemWatchdogTimerHandler = 0x47,
// 	SystemWatchdogTimerInformation = 0x48,
// 	SystemLogicalProcessorInformation = 0x49,
// 	SystemWow64SharedInformationObsolete = 0x4a,
// 	SystemRegisterFirmwareTableInformationHandler = 0x4b,
// 	SystemFirmwareTableInformation = 0x4c,
// 	SystemModuleInformationEx = 0x4d,
// 	SystemVerifierTriageInformation = 0x4e,
// 	SystemSuperfetchInformation = 0x4f,
// 	SystemMemoryListInformation = 0x50,
// 	SystemFileCacheInformationEx = 0x51,
// 	SystemThreadPriorityClientIdInformation = 0x52,
// 	SystemProcessorIdleCycleTimeInformation = 0x53,
// 	SystemVerifierCancellationInformation = 0x54,
// 	SystemProcessorPowerInformationEx = 0x55,
// 	SystemRefTraceInformation = 0x56,
// 	SystemSpecialPoolInformation = 0x57,
// 	SystemProcessIdInformation = 0x58,
// 	SystemErrorPortInformation = 0x59,
// 	SystemBootEnvironmentInformation = 0x5a,
// 	SystemHypervisorInformation = 0x5b,
// 	SystemVerifierInformationEx = 0x5c,
// 	SystemTimeZoneInformation = 0x5d,
// 	SystemImageFileExecutionOptionsInformation = 0x5e,
// 	SystemCoverageInformation = 0x5f,
// 	SystemPrefetchPatchInformation = 0x60,
// 	SystemVerifierFaultsInformation = 0x61,
// 	SystemSystemPartitionInformation = 0x62,
// 	SystemSystemDiskInformation = 0x63,
// 	SystemProcessorPerformanceDistribution = 0x64,
// 	SystemNumaProximityNodeInformation = 0x65,
// 	SystemDynamicTimeZoneInformation = 0x66,
// 	SystemCodeIntegrityInformation = 0x67,
// 	SystemProcessorMicrocodeUpdateInformation = 0x68,
// 	SystemProcessorBrandString = 0x69,
// 	SystemVirtualAddressInformation = 0x6a,
// 	SystemLogicalProcessorAndGroupInformation = 0x6b,
// 	SystemProcessorCycleTimeInformation = 0x6c,
// 	SystemStoreInformation = 0x6d,
// 	SystemRegistryAppendString = 0x6e,
// 	SystemAitSamplingValue = 0x6f,
// 	SystemVhdBootInformation = 0x70,
// 	SystemCpuQuotaInformation = 0x71,
// 	SystemNativeBasicInformation = 0x72,
// 	SystemErrorPortTimeouts = 0x73,
// 	SystemLowPriorityIoInformation = 0x74,
// 	SystemBootEntropyInformation = 0x75,
// 	SystemVerifierCountersInformation = 0x76,
// 	SystemPagedPoolInformationEx = 0x77,
// 	SystemSystemPtesInformationEx = 0x78,
// 	SystemNodeDistanceInformation = 0x79,
// 	SystemAcpiAuditInformation = 0x7a,
// 	SystemBasicPerformanceInformation = 0x7b,
// 	SystemQueryPerformanceCounterInformation = 0x7c,
// 	SystemSessionBigPoolInformation = 0x7d,
// 	SystemBootGraphicsInformation = 0x7e,
// 	SystemScrubPhysicalMemoryInformation = 0x7f,
// 	SystemBadPageInformation = 0x80,
// 	SystemProcessorProfileControlArea = 0x81,
// 	SystemCombinePhysicalMemoryInformation = 0x82,
// 	SystemEntropyInterruptTimingInformation = 0x83,
// 	SystemConsoleInformation = 0x84,
// 	SystemPlatformBinaryInformation = 0x85,
// 	SystemThrottleNotificationInformation = 0x86,
// 	SystemHypervisorProcessorCountInformation = 0x87,
// 	SystemDeviceDataInformation = 0x88,
// 	SystemDeviceDataEnumerationInformation = 0x89,
// 	SystemMemoryTopologyInformation = 0x8a,
// 	SystemMemoryChannelInformation = 0x8b,
// 	SystemBootLogoInformation = 0x8c,
// 	SystemProcessorPerformanceInformationEx = 0x8d,
// 	SystemSpare0 = 0x8e,
// 	SystemSecureBootPolicyInformation = 0x8f,
// 	SystemPageFileInformationEx = 0x90,
// 	SystemSecureBootInformation = 0x91,
// 	SystemEntropyInterruptTimingRawInformation = 0x92,
// 	SystemPortableWorkspaceEfiLauncherInformation = 0x93,
// 	SystemFullProcessInformation = 0x94,
// 	SystemKernelDebuggerInformationEx = 0x95,
// 	SystemBootMetadataInformation = 0x96,
// 	SystemSoftRebootInformation = 0x97,
// 	SystemElamCertificateInformation = 0x98,
// 	SystemOfflineDumpConfigInformation = 0x99,
// 	SystemProcessorFeaturesInformation = 0x9a,
// 	SystemRegistryReconciliationInformation = 0x9b,
// 	MaxSystemInfoClass = 0x9c,
// } SYSTEM_INFORMATION_CLASS;

//typedef struct _LDR_DATA_TABLE_ENTRY
//{
//	LIST_ENTRY InLoadOrderLinks;
//	LIST_ENTRY InMemoryOrderLinks;
//	LIST_ENTRY InInitializationOrderLinks;
//	PVOID DllBase;
//	PVOID EntryPoint;
//	ULONG SizeOfImage;
//	UNICODE_STRING FullDllName;
//	UNICODE_STRING BaseDllName;
//	ULONG Flags;
//	USHORT LoadCount;
//	USHORT TlsIndex;
//	LIST_ENTRY HashLinks;
//	ULONG TimeDateStamp;
//} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

// typedef struct _LDR_DATA_TABLE_ENTRY
// {
// 	LIST_ENTRY InLoadOrderLinks;
// 	LIST_ENTRY InMemoryOrderLinks;
// 	LIST_ENTRY InInitializationOrderLinks;
// 	PVOID DllBase;
// 	PVOID EntryPoint;
// 	ULONG SizeOfImage;
// 	UNICODE_STRING FullDllName;
// 	UNICODE_STRING BaseDllName;
// 	ULONG Flags;
// 	USHORT LoadCount;
// 	USHORT TlsIndex;
// 	LIST_ENTRY HashLinks;
// 	ULONG TimeDateStamp;
// } LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
// 
// typedef wdk::LDR_DATA_TABLE_ENTRY LDR_DATA_TABLE_ENTRY64;
// typedef wdk::PLDR_DATA_TABLE_ENTRY PLDR_DATA_TABLE_ENTRY64;

// typedef struct _PEB_LDR_DATA
// {
// 	ULONG Length;
// 	UCHAR Initialized;
// 	PVOID SsHandle;
// 	LIST_ENTRY InLoadOrderModuleList;
// 	LIST_ENTRY InMemoryOrderModuleList;
// 	LIST_ENTRY InInitializationOrderModuleList;
// } PEB_LDR_DATA, *PPEB_LDR_DATA;

// typedef struct _PEB
// {
// 	UCHAR InheritedAddressSpace;
// 	UCHAR ReadImageFileExecOptions;
// 	UCHAR BeingDebugged;
// 	UCHAR BitField;
// 	PVOID Mutant;
// 	PVOID ImageBaseAddress;
// 	wdk::PPEB_LDR_DATA Ldr;
// 	PVOID ProcessParameters;
// 	PVOID SubSystemData;
// 	PVOID ProcessHeap;
// 	PVOID FastPebLock;
// 	PVOID AtlThunkSListPtr;
// 	PVOID IFEOKey;
// 	PVOID CrossProcessFlags;
// 	PVOID KernelCallbackTable;
// 	ULONG SystemReserved;
// 	ULONG AtlThunkSListPtr32;
// 	PVOID ApiSetMap;
// } PEB, *PPEB;

// typedef struct _SYSTEM_THREAD_INFORMATION
// {
// 	LARGE_INTEGER KernelTime;
// 	LARGE_INTEGER UserTime;
// 	LARGE_INTEGER CreateTime;
// 	ULONG WaitTime;
// 	PVOID StartAddress;
// 	CLIENT_ID ClientId;
// 	KPRIORITY Priority;
// 	LONG BasePriority;
// 	ULONG ContextSwitches;
// 	ULONG ThreadState;
// 	KWAIT_REASON WaitReason;
// }SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFO
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER WorkingSetPrivateSize;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR UniqueProcessKey;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
	wdk::SYSTEM_THREAD_INFORMATION Threads[1];
}SYSTEM_PROCESS_INFO, *PSYSTEM_PROCESS_INFO;

// typedef struct _SYSTEM_MODULE
// {
// 	ULONG_PTR Reserved[2];
// 	PVOID Base;
// 	ULONG Size;
// 	ULONG Flags;
// 	USHORT Index;
// 	USHORT Unknown;
// 	USHORT LoadCount;
// 	USHORT ModuleNameOffset;
// 	CHAR ImageName[256];
// } SYSTEM_MODULE, *PSYSTEM_MODULE;

// typedef struct _SYSTEM_MODULE_INFORMATION
// {
// 	ULONG_PTR ulModuleCount;
// 	SYSTEM_MODULE Modules[1];
// } SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

// typedef struct _LDR_DATA_TABLE_ENTRY32
// {
// 	LIST_ENTRY32 InLoadOrderLinks;
// 	LIST_ENTRY32 InMemoryOrderLinks;
// 	LIST_ENTRY32 InInitializationOrderLinks;
// 	ULONG DllBase;
// 	ULONG EntryPoint;
// 	ULONG SizeOfImage;
// 	UNICODE_STRING32 FullDllName;
// 	UNICODE_STRING32 BaseDllName;
// 	ULONG Flags;
// 	USHORT LoadCount;
// 	USHORT TlsIndex;
// 	LIST_ENTRY32 HashLinks;
// 	ULONG TimeDateStamp;
// } LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

// typedef struct _PEB_LDR_DATA32
// {
// 	ULONG Length;
// 	UCHAR Initialized;
// 	ULONG SsHandle;
// 	LIST_ENTRY32 InLoadOrderModuleList;
// 	LIST_ENTRY32 InMemoryOrderModuleList;
// 	LIST_ENTRY32 InInitializationOrderModuleList;
// } PEB_LDR_DATA32, * PPEB_LDR_DATA32;

// typedef struct _PEB32
// {
// 	UCHAR InheritedAddressSpace;
// 	UCHAR ReadImageFileExecOptions;
// 	UCHAR BeingDebugged;
// 	UCHAR BitField;
// 	ULONG Mutant;
// 	ULONG ImageBaseAddress;
// 	ULONG Ldr;
// 	ULONG ProcessParameters;
// 	ULONG SubSystemData;
// 	ULONG ProcessHeap;
// 	ULONG FastPebLock;
// 	ULONG AtlThunkSListPtr;
// 	ULONG IFEOKey;
// 	ULONG CrossProcessFlags;
// 	ULONG UserSharedInfoPtr;
// 	ULONG SystemReserved;
// 	ULONG AtlThunkSListPtr32;
// 	ULONG ApiSetMap;
// } PEB32, * PPEB32;

typedef struct _SYSTEM_HANDLE
{
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;


// #define WOW64_SIZE_OF_80387_REGISTERS      80
// #define WOW64_MAXIMUM_SUPPORTED_EXTENSION     512
// 
// typedef struct _WOW64_FLOATING_SAVE_AREA {
// 	DWORD   ControlWord;
// 	DWORD   StatusWord;
// 	DWORD   TagWord;
// 	DWORD   ErrorOffset;
// 	DWORD   ErrorSelector;
// 	DWORD   DataOffset;
// 	DWORD   DataSelector;
// 	BYTE    RegisterArea[WOW64_SIZE_OF_80387_REGISTERS];
// 	DWORD   Cr0NpxState;
// } WOW64_FLOATING_SAVE_AREA;

// typedef struct _WOW64_CONTEXT {
// 
// 	//
// 	// The flags values within this flag control the contents of
// 	// a CONTEXT record.
// 	//
// 	// If the context record is used as an input parameter, then
// 	// for each portion of the context record controlled by a flag
// 	// whose value is set, it is assumed that that portion of the
// 	// context record contains valid context. If the context record
// 	// is being used to modify a threads context, then only that
// 	// portion of the threads context will be modified.
// 	//
// 	// If the context record is used as an IN OUT parameter to capture
// 	// the context of a thread, then only those portions of the thread's
// 	// context corresponding to set flags will be returned.
// 	//
// 	// The context record is never used as an OUT only parameter.
// 	//
// 
// 	DWORD ContextFlags;
// 
// 	//
// 	// This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
// 	// set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
// 	// included in CONTEXT_FULL.
// 	//
// 
// 	DWORD   Dr0;
// 	DWORD   Dr1;
// 	DWORD   Dr2;
// 	DWORD   Dr3;
// 	DWORD   Dr6;
// 	DWORD   Dr7;
// 
// 	//
// 	// This section is specified/returned if the
// 	// ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
// 	//
// 
// 	WOW64_FLOATING_SAVE_AREA FloatSave;
// 
// 	//
// 	// This section is specified/returned if the
// 	// ContextFlags word contians the flag CONTEXT_SEGMENTS.
// 	//
// 
// 	DWORD   SegGs;
// 	DWORD   SegFs;
// 	DWORD   SegEs;
// 	DWORD   SegDs;
// 
// 	//
// 	// This section is specified/returned if the
// 	// ContextFlags word contians the flag CONTEXT_INTEGER.
// 	//
// 
// 	DWORD   Edi;
// 	DWORD   Esi;
// 	DWORD   Ebx;
// 	DWORD   Edx;
// 	DWORD   Ecx;
// 	DWORD   Eax;
// 
// 	//
// 	// This section is specified/returned if the
// 	// ContextFlags word contians the flag CONTEXT_CONTROL.
// 	//
// 
// 	DWORD   Ebp;
// 	DWORD   Eip;
// 	DWORD   SegCs;              // MUST BE SANITIZED
// 	DWORD   EFlags;             // MUST BE SANITIZED
// 	DWORD   Esp;
// 	DWORD   SegSs;
// 
// 	//
// 	// This section is specified/returned if the ContextFlags word
// 	// contains the flag CONTEXT_EXTENDED_REGISTERS.
// 	// The format and contexts are processor specific
// 	//
// 
// 	BYTE    ExtendedRegisters[WOW64_MAXIMUM_SUPPORTED_EXTENSION];
// 
// } WOW64_CONTEXT;

typedef VOID(NTAPI* PKNORMAL_ROUTINE)(
	PVOID NormalContext,
	PVOID SystemArgument1,
	PVOID SystemArgument2
	);

typedef VOID(NTAPI* PKKERNEL_ROUTINE)(
	PRKAPC Apc,
	PKNORMAL_ROUTINE* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2
	);

typedef enum _KAPC_ENVIRONMENT
{
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
} KAPC_ENVIRONMENT, * PKAPC_ENVIRONMENT;

typedef VOID(NTAPI* PKRUNDOWN_ROUTINE)(PRKAPC Apc);

typedef struct
{
	LONG	offset : 12;
	LONG	type : 4;
} IMAGE_RELOC, *PIMAGE_RELOC;
typedef struct _RTL_MODULE_EXTENDED_INFO
{
	PVOID ImageBase;
	ULONG ImageSize;
	USHORT FileNameOffset;
	CHAR FullPathName[0x100];
} RTL_MODULE_EXTENDED_INFO, *PRTL_MODULE_EXTENDED_INFO;

#define NT_HEADER(ModBase) (PIMAGE_NT_HEADERS)((ULONG64)(ModBase) + ((PIMAGE_DOS_HEADER)(ModBase))->e_lfanew)
#define NT_HEADER32(ModBase) (PIMAGE_NT_HEADERS32)((ULONG64)(ModBase) + ((PIMAGE_DOS_HEADER)(ModBase))->e_lfanew)
#define IMAGE_FIRST_SECTION(NtHeader) (PIMAGE_SECTION_HEADER)(NtHeader + 1)
//#define SizeAlign(Size) ((Size + 0xFFF) & 0xFFFFFFFFFFFFF000)
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16
#define IMAGE_REL_BASED_DIR64                 10

// typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
// 	USHORT e_magic;                     // Magic number
// 	USHORT e_cblp;                      // Bytes on last page of file
// 	USHORT e_cp;                        // Pages in file
// 	USHORT e_crlc;                      // Relocations
// 	USHORT e_cparhdr;                   // Size of header in paragraphs
// 	USHORT e_minalloc;                  // Minimum extra paragraphs needed
// 	USHORT e_maxalloc;                  // Maximum extra paragraphs needed
// 	USHORT e_ss;                        // Initial (relative) SS value
// 	USHORT e_sp;                        // Initial SP value
// 	USHORT e_csum;                      // Checksum
// 	USHORT e_ip;                        // Initial IP value
// 	USHORT e_cs;                        // Initial (relative) CS value
// 	USHORT e_lfarlc;                    // File address of relocation table
// 	USHORT e_ovno;                      // Overlay number
// 	USHORT e_res[4];                    // Reserved words
// 	USHORT e_oemid;                     // OEM identifier (for e_oeminfo)
// 	USHORT e_oeminfo;                   // OEM information; e_oemid specific
// 	USHORT e_res2[10];                  // Reserved words
// 	LONG   e_lfanew;                    // File address of new exe header
// } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

// typedef struct _IMAGE_EXPORT_DIRECTORY {
// 	ULONG   Characteristics;
// 	ULONG   TimeDateStamp;
// 	USHORT  MajorVersion;
// 	USHORT  MinorVersion;
// 	ULONG   Name;
// 	ULONG   Base;
// 	ULONG   NumberOfFunctions;
// 	ULONG   NumberOfNames;
// 	ULONG   AddressOfFunctions;     // RVA from base of image
// 	ULONG   AddressOfNames;         // RVA from base of image
// 	ULONG   AddressOfNameOrdinals;  // RVA from base of image
// } IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

// typedef struct _IMAGE_SECTION_HEADER {
// 	UCHAR   Name[8];
// 	union {
// 		ULONG   PhysicalAddress;
// 		ULONG   VirtualSize;
// 	} Misc;
// 	ULONG   VirtualAddress;
// 	ULONG   SizeOfRawData;
// 	ULONG   PointerToRawData;
// 	ULONG   PointerToRelocations;
// 	ULONG   PointerToLinenumbers;
// 	USHORT  NumberOfRelocations;
// 	USHORT  NumberOfLinenumbers;
// 	ULONG   Characteristics;
// } IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

// typedef struct _IMAGE_BASE_RELOCATION {
// 	ULONG   VirtualAddress;
// 	ULONG   SizeOfBlock;
// 	//  USHORT  TypeOffset[1];
// } IMAGE_BASE_RELOCATION;

// typedef struct _IMAGE_DATA_DIRECTORY {
// 	ULONG   VirtualAddress;
// 	ULONG   Size;
// } IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

// typedef struct _IMAGE_OPTIONAL_HEADER64 {
// 	USHORT      Magic;
// 	UCHAR       MajorLinkerVersion;
// 	UCHAR       MinorLinkerVersion;
// 	ULONG       SizeOfCode;
// 	ULONG       SizeOfInitializedData;
// 	ULONG       SizeOfUninitializedData;
// 	ULONG       AddressOfEntryPoint;
// 	ULONG       BaseOfCode;
// 	ULONGLONG   ImageBase;
// 	ULONG       SectionAlignment;
// 	ULONG       FileAlignment;
// 	USHORT      MajorOperatingSystemVersion;
// 	USHORT      MinorOperatingSystemVersion;
// 	USHORT      MajorImageVersion;
// 	USHORT      MinorImageVersion;
// 	USHORT      MajorSubsystemVersion;
// 	USHORT      MinorSubsystemVersion;
// 	ULONG       Win32VersionValue;
// 	ULONG       SizeOfImage;
// 	ULONG       SizeOfHeaders;
// 	ULONG       CheckSum;
// 	USHORT      Subsystem;
// 	USHORT      DllCharacteristics;
// 	ULONGLONG   SizeOfStackReserve;
// 	ULONGLONG   SizeOfStackCommit;
// 	ULONGLONG   SizeOfHeapReserve;
// 	ULONGLONG   SizeOfHeapCommit;
// 	ULONG       LoaderFlags;
// 	ULONG       NumberOfRvaAndSizes;
// 	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
// } IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

// typedef struct _IMAGE_FILE_HEADER {
// 	USHORT  Machine;
// 	USHORT  NumberOfSections;
// 	ULONG   TimeDateStamp;
// 	ULONG   PointerToSymbolTable;
// 	ULONG   NumberOfSymbols;
// 	USHORT  SizeOfOptionalHeader;
// 	USHORT  Characteristics;
// } IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

// typedef struct _IMAGE_NT_HEADERS64 
// {
// 	ULONG Signature;
// 	IMAGE_FILE_HEADER FileHeader;
// 	IMAGE_OPTIONAL_HEADER64 OptionalHeader;
// } IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

// typedef struct _IMAGE_IMPORT_DESCRIPTOR {
// 	union {
// 		ULONG   Characteristics;            // 0 for terminating null import descriptor
// 		ULONG   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
// 	} DUMMYUNIONNAME;
// 	ULONG   TimeDateStamp;                  // 0 if not bound,
// 											// -1 if bound, and real date\time stamp
// 											//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
// 											// O.W. date/time stamp of DLL bound to (Old BIND)
// 
// 	ULONG   ForwarderChain;                 // -1 if no forwarders
// 	ULONG   Name;
// 	ULONG   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
// } IMAGE_IMPORT_DESCRIPTOR;

// typedef struct _IMAGE_IMPORT_BY_NAME {
// 	USHORT  Hint;
// 	CHAR   Name[1];
// } IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

// typedef struct _IMAGE_THUNK_DATA64 {
// 	union {
// 		ULONGLONG ForwarderString;  // PUCHAR
// 		ULONGLONG Function;         // PULONG
// 		ULONGLONG Ordinal;
// 		ULONGLONG AddressOfData;    // PIMAGE_IMPORT_BY_NAME
// 	} u1;
// } IMAGE_THUNK_DATA64;
// typedef IMAGE_THUNK_DATA64 * PIMAGE_THUNK_DATA64;

typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;
typedef IMAGE_BASE_RELOCATION UNALIGNED * PIMAGE_BASE_RELOCATION;
typedef PIMAGE_OPTIONAL_HEADER64            PIMAGE_OPTIONAL_HEADER;
typedef struct _IMAGE_NT_HEADERS64 *PIMAGE_NT_HEADERS64;
typedef IMAGE_NT_HEADERS64                  IMAGE_NT_HEADERS;

#define RVA(Instr, InstrSize) ((DWORD64)Instr + InstrSize + *(LONG*)((DWORD64)Instr + (InstrSize - sizeof(LONG))))
#define RVA2(Instr, InstrSize, Off) ((DWORD64)Instr + InstrSize + *(LONG*)((DWORD64)Instr + Off))

// #define NT_HEADER(ModBase) (PIMAGE_NT_HEADERS)((ULONG64)(ModBase) + ((PIMAGE_DOS_HEADER)(ModBase))->e_lfanew)
// #define NT_HEADER32(ModBase) (PIMAGE_NT_HEADERS32)((ULONG64)(ModBase) + ((PIMAGE_DOS_HEADER)(ModBase))->e_lfanew)

#define IMAGE_FIRST_SECTION(NtHeader) (PIMAGE_SECTION_HEADER)(NtHeader + 1)

#define SizeAlign(Size) ((Size + 0xFFF) & 0xFFFFFFFFFFFFF000)

typedef struct _PLUGPLAY_CONTROL_HANDLER_DATA
{
	ULONG32 ControlCode;
	ULONG32 ControlDataSize;
	ULONG64 ControlFunction;
	ULONG64 ControlServerSiloOk;
}PLUGPLAY_CONTROL_HANDLER_DATA, *PPLUGPLAY_CONTROL_HANDLER_DATA;

// typedef enum _REQUEST_TYPE : ULONG
// {
// 	RequestTypeWrite,
// 	RequestTypeRead,
// 	RequestTypeModule,
// 	RequestTypeAllocate,
// 	RequestTypeProtectProcess
// } REQUEST_TYPE;
// 

// typedef struct _OBJECT_CREATE_INFORMATION
// {
// 	ULONG   Attributes;
// 	HANDLE  RootDirectory;
// 	KPROCESSOR_MODE ProbeMode;
// 	ULONG   PagedPoolCharge;
// 	ULONG   NonPagedPoolCharge;
// 	ULONG   SecurityDescriptorCharge;
// 	PVOID   SecurityDescriptor;
// 	PSECURITY_QUALITY_OF_SERVICE   SecurityQos;
// 	SECURITY_QUALITY_OF_SERVICE    SecurityQualityOfService;
// } OBJECT_CREATE_INFORMATION, * POBJECT_CREATE_INFORMATION;

typedef struct _OBJECT_HEADER
{
	SSIZE_T PointerCount;
	union
	{
		SSIZE_T  HandleCount;
		PVOID   NextToFree;
	};
	EX_PUSH_LOCK Lock;
	UINT8 TypeIndex;
	union
	{
		UINT8 TraceFlags;
		struct
		{
			UINT8 DbgRefTrace : 1;
			UINT8 DbgTracePermanent : 1;
		};
	};
	UINT8 InfoMask;
	union
	{
		UINT8 Flags;
		struct
		{
			UINT8 NewObject : 1;
			UINT8 KernelObject : 1;
			UINT8 KernelOnlyAccess : 1;
			UINT8 ExclusiveObject : 1;
			UINT8 PermanentObject : 1;
			UINT8 DefaultSecurityQuota : 1;
			UINT8 SingleHandleEntry : 1;
			UINT8 DeletedInline : 1;
		};
	};

#ifdef _WIN64
	ULONG Reserved;
#endif
	union
	{
		wdk::POBJECT_CREATE_INFORMATION ObjectCreateInfo;
		PVOID QuotaBlockCharged;
	};
	PVOID SecurityDescriptor;
	QUAD Body;
} OBJECT_HEADER, * POBJECT_HEADER;

typedef struct _OBJECT_HEADER_PROCESS_INFO
{
	PEPROCESS  ExclusiveProcess;
	SIZE_T     Reserved;
} OBJECT_HEADER_PROCESS_INFO, * POBJECT_HEADER_PROCESS_INFO;

/**
 * @brief Page Map Level 4 Entry Structure
 *
 */
typedef struct _PAGE_MAP_LEVEL_4_ENTRY
{
	union
	{
		UINT64 Flags;

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Reserved1 : 1;
			UINT64 MustBeZero : 1;
			UINT64 Ignored1 : 4;
			UINT64 PageFrameNumber : 36;
			UINT64 Reserved2 : 4;
			UINT64 Ignored2 : 11;
			UINT64 ExecuteDisable : 1;
		};
	};
} PAGE_MAP_LEVEL_4_ENTRY, * PPAGE_MAP_LEVEL_4_ENTRY;

/**
 * @brief Large Page Directory Pointer Table Entry Structure
 *
 */
typedef struct _LARGE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY
{
	union
	{
		UINT64 Flags;

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Dirty : 1;
			UINT64 LargePage : 1;
			UINT64 Global : 1;
			UINT64 Ignored1 : 3;
			UINT64 Pat : 1;
			UINT64 Reserved1 : 17;
			UINT64 PageFrameNumber : 18;
			UINT64 Reserved2 : 4;
			UINT64 Ignored2 : 7;
			UINT64 ProtectionKey : 4;
			UINT64 ExecuteDisable : 1;
		};
	};
} LARGE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY, * PLARGE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY;

/**
 * @brief Page Directory Pointer Table Entry Structure
 *
 */
typedef struct _PAGE_DIRECTORY_POINTER_TABLE_ENTRY
{
	union
	{
		UINT64 Flags;

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Reserved1 : 1;
			UINT64 LargeLpage : 1;
			UINT64 Ignored1 : 4;
			UINT64 PageFrameNumber : 36;
			UINT64 Reserved2 : 4;
			UINT64 Ignored2 : 11;
			UINT64 ExecuteDisable : 1;
		};
	};
} PAGE_DIRECTORY_POINTER_TABLE_ENTRY, * PPAGE_DIRECTORY_POINTER_TABLE_ENTRY;

/**
 * @brief Large Page Directory Entry Structure
 *
 */
typedef struct _LARGE_PAGE_DIRECTORY_ENTRY
{
	union
	{
		UINT64 Flags;

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Dirty : 1;
			UINT64 LargePage : 1;
			UINT64 Global : 1;
			UINT64 Ignored1 : 3;
			UINT64 Pat : 1;
			UINT64 Reserved1 : 17;
			UINT64 PageFrameNumber : 18;
			UINT64 Reserved2 : 4;
			UINT64 Ignored2 : 7;
			UINT64 ProtectionKey : 4;
			UINT64 ExecuteDisable : 1;
		};
	};
} LARGE_PAGE_DIRECTORY_ENTRY, * PLARGE_PAGE_DIRECTORY_ENTRY;

/**
 * @brief Page Directory Entry Structure
 *
 */
typedef struct _PAGE_DIRECTORY_ENTRY
{
	union
	{
		UINT64 Flags;

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Reserved1 : 1;
			UINT64 LargePage : 1;
			UINT64 Ignored1 : 4;
			UINT64 PageFrameNumber : 36;
			UINT64 Reserved2 : 4;
			UINT64 Ignored2 : 11;
			UINT64 ExecuteDisable : 1;
		};
	};
} PAGE_DIRECTORY_ENTRY, * PPAGE_DIRECTORY_ENTRY;

/**
 * @brief Page Table Entry Structure
 *
 */
typedef struct _PAGE_TABLE_ENTRY
{
	union
	{
		UINT64 Flags;

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Dirty : 1;
			UINT64 Pat : 1;
			UINT64 Global : 1;
			UINT64 Ignored1 : 3;
			UINT64 PageFrameNumber : 36;
			UINT64 Reserved1 : 4;
			UINT64 Ignored2 : 7;
			UINT64 ProtectionKey : 4;
			UINT64 ExecuteDisable : 1;
		};
	};
} PAGE_TABLE_ENTRY, * PPAGE_TABLE_ENTRY;
/**
 * @brief Page Entries
 *
 */
typedef struct _PAGE_ENTRY
{
	union
	{
		UINT64 Flags;

		PAGE_MAP_LEVEL_4_ENTRY                   Pml4;
		LARGE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY PdptLarge; // 1GB
		PAGE_DIRECTORY_POINTER_TABLE_ENTRY       Pdpt;
		LARGE_PAGE_DIRECTORY_ENTRY               PdLarge; // 2MB
		PAGE_DIRECTORY_ENTRY                     Pd;
		PAGE_TABLE_ENTRY                         Pt;

		//
		// Common fields.
		//

		struct
		{
			UINT64 Present : 1;
			UINT64 Write : 1;
			UINT64 Supervisor : 1;
			UINT64 PageLevelWriteThrough : 1;
			UINT64 PageLevelCacheDisable : 1;
			UINT64 Accessed : 1;
			UINT64 Dirty : 1;
			UINT64 LargePage : 1;
			UINT64 Global : 1;
			UINT64 Ignored1 : 3;
			UINT64 PageFrameNumber : 36;
			UINT64 Reserved1 : 4;
			UINT64 Ignored2 : 7;
			UINT64 ProtectionKey : 4;
			UINT64 ExecuteDisable : 1;
		};
	};
} PAGE_ENTRY, * PPAGE_ENTRY;

//////////////////////////////////////////////////
//				    Definitions					//
//////////////////////////////////////////////////

#define PAGE_4KB_OFFSET ((UINT64)(1 << 12) - 1)
#define PAGE_2MB_OFFSET ((UINT64)(1 << 21) - 1)
#define PAGE_4MB_OFFSET ((UINT64)(1 << 22) - 1)
#define PAGE_1GB_OFFSET ((UINT64)(1 << 30) - 1)

// 取系统版本号
// using SystemVersion = enum _SystemVersion
// {
// 	Windows_Unknown,	        // 未知
// 	Windows7_SP1,               // 6.1.7601
// 	Windows10,					// 10.0.10240
// 	Windows10_1511,             // 10.0.10586
// 	Windows10_1607,             // 10.0.14393
// 	Windows10_1703,             // 10.0.15063
// 	Windows10_1709,             // 10.0.16299
// 	Windows10_1803,             // 10.0.17134
// 	Windows10_1809,             // 10.0.17763
// 	Windows10_1903,             // 10.0.18362
// 	Windows10_1909,				// 10.0.18363
// 	Windows10_2004,				// 10.0.19041
// 	Windows10_20H2,				// 10.0.19042
// 	Windows10_21H1,				// 11.0.19043
// 	Windows10_21H2_10,			// 11.0.19044
// 	Windows10_21H2_11,          // 11.0.22000
// };

extern "C"
{
// 	NTKERNELAPI NTSTATUS __stdcall ZwQueryInformationProcess(
// 	HANDLE ProcessHandle,
// 	PROCESSINFOCLASS ProcessInformationClass,
// 	PVOID ProcessInformation,
// 	ULONG ProcessInformationLength);

	//NTKERNELAPI         
	//	NTSTATUS
	//	NTAPI
	//	ZwSetInformationProcess(
	//		_In_ HANDLE ProcessHandle,
	//		_In_ PROCESSINFOCLASS ProcessInformationClass,
	//		_In_reads_bytes_(ProcessInformationLength) PVOID ProcessInformation,
	//		_In_ ULONG ProcessInformationLength
	//	);


	//NTKERNELAPI NTSYSAPI
	//	NTSTATUS
	//	NTAPI
	//	ZwLockVirtualMemory(
	//		IN HANDLE               ProcessHandle,
	//		IN PVOID* BaseAddress,
	//		IN OUT PULONG64           NumberOfBytesToLock,
	//		IN ULONG                LockOption);

	typedef DWORD(NTAPI* PTHREAD_START_ROUTINE)(PVOID lpThreadParameter);
	typedef struct _PROC_THREAD_ATTRIBUTE_LIST* PPROC_THREAD_ATTRIBUTE_LIST, * LPPROC_THREAD_ATTRIBUTE_LIST;

	typedef NTSTATUS(NTAPI* PUSER_THREAD_START_ROUTINE)(
		_In_ PVOID ThreadParameter
		);


// 	NTKERNELAPI
// 		VOID
// 		NTAPI
// 		KeInitializeApc(
// 			IN PKAPC Apc,
// 			IN PKTHREAD Thread,
// 			IN KAPC_ENVIRONMENT ApcStateIndex,
// 			IN PKKERNEL_ROUTINE KernelRoutine,
// 			IN PKRUNDOWN_ROUTINE RundownRoutine,
// 			IN PKNORMAL_ROUTINE NormalRoutine,
// 			IN KPROCESSOR_MODE ApcMode,
// 			IN PVOID NormalContext
// 		);

	
	//NTKERNELAPI
	//	BOOLEAN
	//	NTAPI
	//	KeInsertQueueApc(
	//		PKAPC Apc,
	//		PVOID SystemArgument1,
	//		PVOID SystemArgument2,
	//		KPRIORITY Increment
	//	);

		
		//NTKERNELAPI NTSYSAPI
		//NTSTATUS
		//NTAPI
		//ZwUnlockVirtualMemory(


		//	IN HANDLE               ProcessHandle,
		//	IN PVOID* BaseAddress,
		//	IN OUT PULONG64           NumberOfBytesToLock,
		//	IN ULONG                LockOption);

	//NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory
	//(
	//	PEPROCESS SourceProcess,
	//	PVOID SourceAddress,
	//	PEPROCESS TargetProcess,
	//	PVOID TargetAddress,
	//	SIZE_T BufferSize,
	//	KPROCESSOR_MODE PreviousMode,
	//	PSIZE_T ReturnSize
	//);
	//NTKERNELAPI wdk::PPEB PsGetProcessPeb(PEPROCESS);
	//NTKERNELAPI PVOID PsGetProcessWow64Process(PEPROCESS Process);
	NTKERNELAPI void PsReleaseProcessExitSynchronization(PEPROCESS);
	NTKERNELAPI NTSTATUS PsAcquireProcessExitSynchronization(PEPROCESS);
	NTKERNELAPI NTSTATUS KeUserModeCallback(ULONG, PVOID, ULONG, PVOID, PVOID);
	//NTKERNELAPI NTSTATUS ZwWaitForMultipleObjects(
	//	ULONG Count,
	//	HANDLE Object[],
	//	WAIT_TYPE WaitType,
	//	BOOLEAN Alertable,
	//	PLARGE_INTEGER Time);
	//NTSYSAPI __int64 __fastcall PsGetThreadWin32Thread(PETHREAD a1);
	//NTSYSAPI NTSTATUS ZwProtectVirtualMemory(HANDLE, PVOID*, SIZE_T*, ULONG, PULONG);
	//NTKERNELAPI NTSTATUS ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
	NTSYSAPI void RtlRestoreContext(struct _CONTEXT* ContextRecord, struct _EXCEPTION_RECORD* ExceptionRecord);
	NTSYSAPI NTSTATUS RtlCreateUserThread(HANDLE, PVOID, BOOLEAN, ULONG, SIZE_T, SIZE_T, PVOID, PVOID, PHANDLE, PCLIENT_ID);
	//NTKERNELAPI
	//	PVOID
	//	NTAPI
	//	PsGetThreadTeb(IN PETHREAD Thread);
// 
// 	NTKERNELAPI
// 		PVOID
// 		NTAPI
// 		PsGetCurrentProcessWow64Process();

	//NTKERNELAPI
	//	BOOLEAN
	//	NTAPI
	//	KeTestAlertThread(IN KPROCESSOR_MODE AlertMode);
}