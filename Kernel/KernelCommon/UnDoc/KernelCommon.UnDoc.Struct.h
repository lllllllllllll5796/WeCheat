#pragma once

typedef struct _PiDDBCacheEntry
{
	LIST_ENTRY		List;
	UNICODE_STRING	DriverName;
	ULONG			TimeDateStamp;
	NTSTATUS		LoadStatus;
	CHAR			_0x0028[16]; // data from the shim engine, or uninitialized memory for custom drivers
}*PPiDDBCacheEntry, PiDDBCacheEntry;

typedef struct _WOW64_PROCESS
{
	wdk::PPEB32 Wow64;
} WOW64_PROCESS, * PWOW64_PROCESS;

typedef struct _EX_CALLBACK_ROUTINE_BLOCK
{
	EX_RUNDOWN_REF        RundownProtect;
	PEX_CALLBACK_FUNCTION Function;
	PVOID                 Context;
} EX_CALLBACK_ROUTINE_BLOCK, * PEX_CALLBACK_ROUTINE_BLOCK;

typedef struct _EX_FAST_REF_S
{
	union
	{
		PVOID Object;
#if defined (_WIN64)
		ULONG_PTR RefCnt : 4;
#else
		ULONG_PTR RefCnt : 3;
#endif
		ULONG_PTR Value;
	};
} EX_FAST_REF_S, * PEX_FAST_REF_S;

typedef struct _EX_CALLBACK
{
	EX_FAST_REF_S RoutineBlock;
} EX_CALLBACK, * PEX_CALLBACK;

typedef union _WOW64_APC_CONTEXT
{
	struct
	{
		ULONG Apc32BitContext;
		ULONG Apc32BitRoutine;
	}u1;

	PVOID Apc64BitContext;

} WOW64_APC_CONTEXT, * PWOW64_APC_CONTEXT;

typedef struct _NT_PROC_THREAD_ATTRIBUTE_ENTRY
{
	ULONG Attribute;    // PROC_THREAD_ATTRIBUTE_XXX
	SIZE_T Size;
	ULONG_PTR Value;
	ULONG Unknown;
} NT_PROC_THREAD_ATTRIBUTE_ENTRY, * NT_PPROC_THREAD_ATTRIBUTE_ENTRY;

typedef struct _NT_PROC_THREAD_ATTRIBUTE_LIST
{
	ULONG Length;
	NT_PROC_THREAD_ATTRIBUTE_ENTRY Entry[1];
} NT_PROC_THREAD_ATTRIBUTE_LIST, * PNT_PROC_THREAD_ATTRIBUTE_LIST;

typedef struct _SYSTEM_MODULE_ENTRY
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} SYSTEM_MODULE_ENTRY, * PSYSTEM_MODULE_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG Count;
	SYSTEM_MODULE_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct _OBJECT_ALL_INFORMATION
{
	ULONG NumberOfObjectsTypes;
	wdk::OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
} OBJECT_ALL_INFORMATION, *POBJECT_ALL_INFORMATION;

typedef struct _OBJECT_HANDLE_ATTRIBUTE_INFORMATION
{
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
}OBJECT_HANDLE_ATTRIBUTE_INFORMATION, * POBJECT_HANDLE_ATTRIBUTE_INFORMATION;

typedef struct _ACTCTXW
{
	ULONG  cbSize;
	ULONG  dwFlags;
	PWCH   lpSource;
	USHORT wProcessorArchitecture;
	USHORT wLangId;
	PWCH   lpAssemblyDirectory;
	PWCH   lpResourceName;
	PWCH   lpApplicationName;
	PVOID  hModule;
} ACTCTXW, * PACTCTXW;

typedef struct _ACTCTXW32
{
	ULONG  cbSize;
	ULONG  dwFlags;
	ULONG  lpSource;
	USHORT wProcessorArchitecture;
	USHORT wLangId;
	ULONG  lpAssemblyDirectory;
	ULONG  lpResourceName;
	ULONG  lpApplicationName;
	ULONG  hModule;
} ACTCTXW32, * PACTCTXW32;

typedef struct _CONTEXT_CHUNK
{
	/* 0x0000 */ long Offset;
	/* 0x0004 */ unsigned long Length;
} CONTEXT_CHUNK, * PCONTEXT_CHUNK; /* size: 0x0008 */

typedef struct _CONTEXT_EX
{
	/* 0x0000 */ struct _CONTEXT_CHUNK All;
	/* 0x0008 */ struct _CONTEXT_CHUNK Legacy;
	/* 0x0010 */ struct _CONTEXT_CHUNK XState;
} CONTEXT_EX, * PCONTEXT_EX; /* size: 0x0018 */

typedef struct _OBJECT_DIRECTORY_ENTRY         // 3 elements, 0x18 bytes (sizeof) 
{
	/*0x000*/     struct _OBJECT_DIRECTORY_ENTRY* ChainLink;
	/*0x008*/     VOID* Object;
	/*0x010*/     ULONG32      HashValue;
}OBJECT_DIRECTORY_ENTRY, * POBJECT_DIRECTORY_ENTRY;

typedef struct _THREAD_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	ULONG_PTR AffinityMask;
	LONG Priority;
	LONG BasePriority;
} THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;


typedef struct _OBJECT_DIRECTORY               // 6 elements, 0x150 bytes (sizeof) 
{
	struct _OBJECT_DIRECTORY_ENTRY* HashBuckets[37];
}OBJECT_DIRECTORY, * POBJECT_DIRECTORY;


// WOW64_CONTEXT is not undocumented, but it's missing from the WDK
typedef struct _WOW64_FLOATING_SAVE_AREA
{
	ULONG ControlWord;
	ULONG StatusWord;
	ULONG TagWord;
	ULONG ErrorOffset;
	ULONG ErrorSelector;
	ULONG DataOffset;
	ULONG DataSelector;
	UCHAR RegisterArea[WOW64_SIZE_OF_80387_REGISTERS];
	ULONG Cr0NpxState;
} WOW64_FLOATING_SAVE_AREA, *PWOW64_FLOATING_SAVE_AREA;

#pragma pack(push, 4)

typedef struct _WOW64_CONTEXT
{
	ULONG ContextFlags;

	ULONG Dr0;
	ULONG Dr1;
	ULONG Dr2;
	ULONG Dr3;
	ULONG Dr6;
	ULONG Dr7;

	WOW64_FLOATING_SAVE_AREA FloatSave;

	ULONG SegGs;
	ULONG SegFs;
	ULONG SegEs;
	ULONG SegDs;

	ULONG Edi;
	ULONG Esi;
	ULONG Ebx;
	ULONG Edx;
	ULONG Ecx;
	ULONG Eax;

	ULONG Ebp;
	ULONG Eip;
	ULONG SegCs;
	ULONG EFlags;
	ULONG Esp;
	ULONG SegSs;

	UCHAR ExtendedRegisters[WOW64_MAXIMUM_SUPPORTED_EXTENSION];

} WOW64_CONTEXT;
#pragma pack(pop)
typedef WOW64_CONTEXT* PWOW64_CONTEXT;

typedef struct _CALLBACK_ENTRY
{
	ULONG64 Unknow;
	ULONG64 Unknow1;
	UNICODE_STRING AltitudeString;
	LIST_ENTRY NextEntryItemList;
	ULONG64 Operations;
	PVOID ObHandle;
	PVOID ObjectType;
	ULONG64 PreCallbackAddr;
	ULONG64 PostCallbackAddr;
}CALLBACK_ENTRY, *PCALLBACK_ENTRY;

typedef struct _CALLBACK_ENTRY_ITEM {
	LIST_ENTRY EntryItemList;
	OB_OPERATION Operations;
	PCALLBACK_ENTRY CallbackEntry;
	POBJECT_TYPE ObjectType;
	POB_PRE_OPERATION_CALLBACK PreOperation;
	POB_POST_OPERATION_CALLBACK PostOperation;
	__int64 unk;
}CALLBACK_ENTRY_ITEM, *PCALLBACK_ENTRY_ITEM;

typedef struct _OBCALLBACK
{
	LIST_ENTRY nextcallback;
	POB_PRE_OPERATION_CALLBACK PreOperation;
}OBCALLBACK, *POBCALLBACK;

typedef struct _SYSTEM_THREAD
{
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
	ULONG ContextSwitchCount;
	THREAD_STATE State;
	KWAIT_REASON WaitReason;

} SYSTEM_THREAD,
* PSYSTEM_THREAD;

typedef struct _SYSTEM_PROCESSES_INFORMATION
{
	ULONG NextEntryDelta;
	ULONG ThreadCount;
	ULONG Reserved1[6];
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ProcessName;
	KPRIORITY BasePriority;
	ULONG ProcessId;
	ULONG InheritedFromProcessId;
	ULONG HandleCount;
	ULONG Reserved2[2];
	VM_COUNTERS VmCounters;
	IO_COUNTERS IoCounters;
	SYSTEM_THREAD Threads[1];

} SYSTEM_PROCESSES_INFORMATION,
* PSYSTEM_PROCESSES_INFORMATION;

typedef struct _ALPC_MESSAGE_ATTRIBUTES // 2 elements, 0x8 bytes (sizeof)
{
	/*0x000*/     ULONG32      AllocatedAttributes;
	/*0x004*/     ULONG32      ValidAttributes;
}ALPC_MESSAGE_ATTRIBUTES, * PALPC_MESSAGE_ATTRIBUTES;

typedef struct _HANDLE_TABLE_ENTRY HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct tagTHREADINFO
{
	/* 0x0000 */ struct _ETHREAD* pEThread;
}THREADINFO, * PTHREADINFO;

typedef struct _THRDESKHEAD
{
	/* 0x0000 */ void* h;
	/* 0x0008 */ unsigned long cLockObj;
	/* 0x000c */ long Padding_21;
	/* 0x0010 */ struct tagTHREADINFO* pti;
	/* 0x0018 */ struct tagDESKTOP* rpdesk;
	/* 0x0020 */ unsigned char* pSelf;
} THRDESKHEAD, * PTHRDESKHEAD; /* size: 0x0028 */

typedef struct tagWND
{
	struct _THRDESKHEAD head;
}WND, * PWND;

typedef struct _PSP_SYSTEM_DLL
{
	/* 0x0000 */ struct _EX_FAST_REF DllSection;
	/* 0x0008 */ struct _EX_PUSH_LOCK DllLock;
} PSP_SYSTEM_DLL, * PPSP_SYSTEM_DLL; /* size: 0x0010 */

typedef PVOID PMMVAD;
typedef ULONG WIN32_PROTECTION_MASK;
typedef PULONG PWIN32_PROTECTION_MASK;

