#pragma once

typedef struct _DEBUG_PROCESS_TABLE
{
	MY_LIST list_entry;        //指向DEBUG_PROCESS对象
	FAST_MUTEX Mutex;          //互斥锁
}DEBUG_PROCESS_TABLE, * PDEBUG_PROCESS_TABLE;

typedef struct _PROTECTOBJ
{
	MY_LIST list_entry;        //保护对象列表
	FAST_MUTEX Mutex;          //互斥锁
}PROTECTOBJ, * PPROTECTOBJ;

typedef struct _DEBUGGER_TABLE
{
	MY_LIST list_entry;        //指向DEBUGGER_DATA对象
	FAST_MUTEX Mutex;          //互斥锁
}DEBUGGER_TABLE, * PDEBUGGER_TABLE;

typedef struct _BREAKPOINT_TABLE
{
	MY_LIST list_entry;  //指向BREAKPOINT_TABLE_ENTRY
	FAST_MUTEX Mutex;          //互斥锁
}BREAKPOINT_TABLE, * PBREAKPOINT_TABLE;

//虚拟句柄表
typedef struct _VIRTUAL_HANDLE_TABLE_ENTRY
{
	size_t id;
	HANDLE handle;
	PVOID Object;
	HANDLE UniqueProcessId;
	LIST_ENTRY list_entry;
}VIRTUAL_HANDLE_TABLE_ENTRY, * PVIRTUAL_HANDLE_TABLE_ENTRY;

typedef struct _VIRTUAL_HANDLE_TABLE
{
	MY_LIST list_entry;    //指向DEBUG_PROCESS对象
	FAST_MUTEX Mutex;          //互斥锁
}VIRTUAL_HANDLE_TABLE, * PVIRTUAL_HANDLE_TABLE;

//WIN10-WIN11通用OBJECT_TYPE结构体
typedef struct _OBJECT_TYPE
{
	/* 0x0000 */ struct _LIST_ENTRY TypeList;
	/* 0x0010 */ struct _UNICODE_STRING Name;
	/* 0x0020 */ void* DefaultObject;
	/* 0x0028 */ unsigned char Index;
	/* 0x0029 */ char Padding_425[3];
	/* 0x002c */ unsigned long TotalNumberOfObjects;
	/* 0x0030 */ unsigned long TotalNumberOfHandles;
	/* 0x0034 */ unsigned long HighWaterNumberOfObjects;
	/* 0x0038 */ unsigned long HighWaterNumberOfHandles;
	/* 0x003c */ long Padding_426;
	/* 0x0040 */ struct _OBJECT_TYPE_INITIALIZER TypeInfo;
	/* 0x00b8 */ struct _EX_PUSH_LOCK TypeLock;
	/* 0x00c0 */ unsigned long Key;
	/* 0x00c4 */ long Padding_427;
	/* 0x00c8 */ struct _LIST_ENTRY CallbackList;
} OBJECT_TYPE, * POBJECT_TYPE; /* size: 0x00d8 */


namespace Global
{
	/**************************** 函数指针 ****************************/

	BOOL Initialize_Global();
	VOID UnInitialize_Global();

	extern PDRIVER_OBJECT g_DriverObject;
	extern ULONG64 g_DriverBase;
	extern ULONG32 g_DriverSize;

	extern ULONG64 g_KernelBase;
	extern ULONG32 g_KernelSize;
	extern BOOLEAN g_IsInitSymbols;
	extern BOOLEAN g_DbgkInitialized;

	extern POBJECT_TYPE*         ObTypeIndexTable;
	extern FAST_MUTEX            DbgkpProcessDebugPortMutex;
	extern POBJECT_TYPE          WeDebug_DbgkDebugObjectType;
	//全局变量
	extern PVOID                 PspLoaderInitRoutine;
	extern POBJECT_TYPE*         DbgkDebugObjectType;                    //调试对象类型
	extern PULONG                PspNotifyEnableMask;
	extern PULONG                PerfGlobalGroupMask;
	extern PEX_PUSH_LOCK         PspActiveProcessLock;                   //活动进程列表锁
	extern PULONG_PTR            PspProcessSequenceNumber;               //进程序列号
	extern PLIST_ENTRY           PsActiveProcessHead;                    //活动进程列表

	extern FAST_MUTEX            LongFlagsMutex;
	extern PROTECTOBJ            g_ProtectFileObjList;                   //保护的文件对象列表
	extern PROTECTOBJ            g_ProtectWndObjList;                    //保护的窗口对象列表
	extern DEBUG_PROCESS_TABLE   g_DebugProcessList;                     //被调试的进程列表
	extern BREAKPOINT_TABLE      g_BreakpointList;                       //断点列表
	extern VIRTUAL_HANDLE_TABLE  g_VirtualHandleList;                    //虚拟句柄列表
	extern DEBUGGER_TABLE        g_DebuggerList;                         //调试器对象

	extern fnObDuplicateObject ObDuplicateObject;  //复制对象
	extern fnKeResumeThread KeResumeThread;
	extern fnKeSuspendThread KeSuspendThread;
	extern fnKeForceResumeThread KeForceResumeThread;
	extern fnKeFreezeAllThreads KeFreezeAllThreads;
	extern fnKeThawAllThreads KeThawAllThreads;
	extern fnPsGetNextProcessThread PsGetNextProcessThread; //获取进程的下一个线程
	extern fnPsQuitNextProcessThread PsQuitNextProcessThread; //线程对象解除引用
	extern fnMmGetFileNameForAddress MmGetFileNameForAddress; //通过地址获取名称信息
	extern fnMmGetFileNameForSection MmGetFileNameForSection; //通过节对象获取名称信息
	extern fnLpcRequestWaitReplyPortEx LpcRequestWaitReplyPortEx; //请求端口
	extern fnKeContextFromKframes KeContextFromKframes;
	extern fnKeContextToKframes KeContextToKframes;
	extern fnKiCheckForAtlThunk KiCheckForAtlThunk;
	extern fnKiSegSsToTrapFrame KiSegSsToTrapFrame;
	extern fnKiEspToTrapFrame KiEspToTrapFrame;
	extern fnKiDebugRoutine KiDebugRoutine;
	extern fnRtlDispatchException RtlDispatchException;
	extern fnPsCallImageNotifyRoutines PsCallImageNotifyRoutines;  //调用映像回调例程
	extern fnObGetProcessHandleCount ObGetProcessHandleCount;
	extern fnPsGetProcessSessionId PsGetProcessSessionId;
	extern fnObIsLUIDDeviceMapsEnabled ObIsLUIDDeviceMapsEnabled;
	extern fnObQueryDeviceMapInformation ObQueryDeviceMapInformation;
	extern fnMmGetImageInformation MmGetImageInformation;
	extern fnMmGetExecuteOptions MmGetExecuteOptions;
	extern fnExMapHandleToPointer ExMapHandleToPointer;
	extern fnExUnlockHandleTableEntry ExUnlockHandleTableEntry;
	extern fnZwAlpcSendWaitReceivePort ZwAlpcSendWaitReceivePort;
	extern fnMmGetSectionInformation MmGetSectionInformation;
	extern fnKiPreprocessFault KiPreprocessFault;
	extern fnDbgkpStartSystemErrorHandler DbgkpStartSystemErrorHandler;
	extern fnAlpcInitializeMessageAttribute AlpcInitializeMessageAttribute;
	extern fnAlpcGetMessageAttribute AlpcGetMessageAttribute;
	extern fnDbgkpRemoveErrorPort DbgkpRemoveErrorPort;
	extern fnDbgkOpenProcessDebugPort DbgkOpenProcessDebugPort;
	extern fnNtCreateDebugObject NtCreateDebugObject;
	extern fnNtDebugActiveProcess NtDebugActiveProcess;
	extern fnNtWaitForDebugEvent NtWaitForDebugEvent;
	extern fnKiDispatchException KiDispatchException;
	//extern fnKiDispatchException Original_KiDispatchException;
	extern fnRtlInitializeExtendedContext RtlInitializeExtendedContext;
	extern fnRtlGetExtendedContextLength RtlGetExtendedContextLength;
	extern fnRtlpCopyExtendedContext RtlpCopyExtendedContext;
	extern fnNtShutdownSystem NtShutdownSystem;
	extern fnObCreateObjectType ObCreateObjectType;
	extern fnSeAuditingWithTokenForSubcategory SeAuditingWithTokenForSubcategory;
	extern fnSeAuditProcessCreation SeAuditProcessCreation;
	extern fnPspGetJobFromSet PspGetJobFromSet;
	extern fnPspBindProcessSessionToJob PspBindProcessSessionToJob;
	extern fnPspAddProcessToJob PspAddProcessToJob;
	extern fnKiCheckForKernelApcDelivery KiCheckForKernelApcDelivery;
	extern fnDbgkCopyProcessDebugPort DbgkCopyProcessDebugPort;
	extern fnSeCreateAccessStateEx SeCreateAccessStateEx;
	extern fnObInsertObjectEx ObInsertObjectEx;
	extern fnPspValidateJobAffinityState PspValidateJobAffinityState;
	extern fnObCheckRefTraceProcess ObCheckRefTraceProcess;
	extern fnSeDeleteAccessState SeDeleteAccessState;
	extern fnPspInsertProcess PspInsertProcess;
	extern fnPspInsertThread PspInsertThread;
	extern fnNtDebugContinue NtDebugContinue;
	extern fnDbgkMapViewOfSection DbgkMapViewOfSection;
	extern fnDbgkUnMapViewOfSection DbgkUnMapViewOfSection;
	extern fnDbgkpSuppressDbgMsg DbgkpSuppressDbgMsg;
	extern fnPsReferenceProcessFilePointer PsReferenceProcessFilePointer;
	extern fnObFastReferenceObject ObFastReferenceObject;
	extern fnObFastReferenceObjectLocked ObFastReferenceObjectLocked;
	extern fnMiReferenceControlAreaFile MiReferenceControlAreaFile;
	extern fnObFastDereferenceObject ObFastDereferenceObject;
	extern fnDbgkSendSystemDllMessages DbgkSendSystemDllMessages;
	extern fnDbgkCreateThread DbgkCreateThread;
	//extern fnDbgkCreateThread Original_DbgkCreateThread;
	extern fnDbgkExitThread DbgkExitThread;
	extern fnDbgkExitProcess DbgkExitProcess;
	extern fnNtUserFindWindowEx NtUserFindWindowEx;
	//extern fnNtUserFindWindowEx Original_NtUserFindWindowEx;
	extern fnDbgkForwardException DbgkForwardException;
	extern fnPsSynchronizeWithThreadInsertion PsSynchronizeWithThreadInsertion;
	extern fnNtGetContextThread NtGetContextThread;
	//extern fnNtGetContextThread Original_NtGetContextThread;
	extern fnNtSetContextThread NtSetContextThread;
	//extern fnNtSetContextThread Original_NtSetContextThread;
	extern fnNtReadVirtualMemory NtReadVirtualMemory;
	//extern fnNtReadVirtualMemory Original_NtReadVirtualMemory;
	extern fnNtWriteVirtualMemory NtWriteVirtualMemory;
	//extern fnNtWriteVirtualMemory Original_NtWriteVirtualMemory;
	extern fnEtwTraceThread EtwTraceThread;
	extern fnExReferenceCallBackBlock ExReferenceCallBackBlock;
	extern fnExGetCallBackBlockRoutine ExGetCallBackBlockRoutine;
	extern fnExDereferenceCallBackBlock ExDereferenceCallBackBlock;
	//extern fnObReferenceObjectSafe ObReferenceObjectSafe;
	extern fnPspCatchCriticalBreak PspCatchCriticalBreak;
	extern fnLpcRequestPort LpcRequestPort;
	extern fnAlpcpSendMessage AlpcpSendMessage;
	extern fnPspW32ThreadCallout* PspW32ThreadCallout;
	extern fnPspW32ProcessCallout* PspW32ProcessCallout;
	extern fnIoCancelThreadIo IoCancelThreadIo;
	extern fnExTimerRundown ExTimerRundown;
	extern fnCmNotifyRunDown CmNotifyRunDown;
	extern fnKeRundownThread KeRundownThread;
	extern fnPspUmsUnInitThread PspUmsUnInitThread;
	extern fnKeRundownQueueEx KeRundownQueueEx;
	extern fnRtlFreeUserStack RtlFreeUserStack;
	extern fnMmDeleteTeb MmDeleteTeb;
	extern fnPspExitProcess PspExitProcess;
	extern fnSeAuditProcessExit SeAuditProcessExit;
	extern fnObKillProcess ObKillProcess;
	extern fnPspExitProcessFromJob PspExitProcessFromJob;
	extern fnKeFlushQueueApc KeFlushQueueApc;
	extern fnMmCleanProcessAddressSpace MmCleanProcessAddressSpace;
	extern fnLpcExitProcess LpcExitProcess;
	extern fnPspLegoNotifyRoutine* PspLegoNotifyRoutine;
	extern fnKeSetProcess KeSetProcess;
	extern fnKeTerminateThread KeTerminateThread;
	extern fnPspExitThread PspExitThread;
	//extern fnPspExitThread Original_PspExitThread;
	extern fnPsGetNextProcess PsGetNextProcess;
	extern fnPsTerminateProcess PsTerminateProcess;
	extern fnDbgkpCloseObject DbgkpCloseObject;
	extern fnKeStartThread KeStartThread;
	extern fnObReferenceObjectEx ObReferenceObjectEx;
	extern fnPspInitializeCpuQuotaApc PspInitializeCpuQuotaApc;
	extern fnPspAdjustThreadSecurityDescriptor PspAdjustThreadSecurityDescriptor;
	extern fnEtwTraceProcess EtwTraceProcess;
	extern fnPspCreateObjectHandle PspCreateObjectHandle;
	extern fnPspTerminateThreadByPointer PspTerminateThreadByPointer;
	extern fnKeReadyThread KeReadyThread;
	extern fnMmDeleteKernelStack MmDeleteKernelStack;
	extern fnPsCaptureExceptionPort PsCaptureExceptionPort;
	extern fnKeRaiseUserException KeRaiseUserException;
	extern fnPspCreateThread PspCreateThread;
	//extern fnPspCreateThread Original_PspCreateThread;
	extern fnNtCreateThreadEx NtCreateThreadEx;
	//extern fnNtCreateThreadEx Original_NtCreateThreadEx;
	extern fnZwProtectVirtualMemory ZwProtectVirtualMemory;
	extern fnNtProtectVirtualMemory NtProtectVirtualMemory;
	//extern fnNtProtectVirtualMemory Original_NtProtectVirtualMemory;
	extern fnPsFreezeProcess PsFreezeProcess;
	extern fnPsThawProcess PsThawProcess;
	extern fnDbgkpSuspendProcess DbgkpSuspendProcess;
	extern fnDbgkpQueueMessage DbgkpQueueMessage;
	extern fnDbgkpSectionToFileHandle DbgkpSectionToFileHandle;
	extern fnDbgkpSendApiMessage DbgkpSendApiMessage;
	extern fnDbgkpSendApiMessageLpc DbgkpSendApiMessageLpc;
	extern fnDbgkpSendErrorMessage DbgkpSendErrorMessage;
	extern fnPsSetProcessFaultInformation PsSetProcessFaultInformation;
	extern fnDbgkpPostFakeProcessCreateMessages DbgkpPostFakeProcessCreateMessages;
	//extern fnDbgkpSetProcessDebugObject DbgkpSetProcessDebugObject;
	extern fnDbgkpWakeTarget DbgkpWakeTarget;
	extern fnDbgkClearProcessDebugObject DbgkClearProcessDebugObject;
	extern fnNtRemoveProcessDebug NtRemoveProcessDebug;
	extern fnDbgkpMarkProcessPeb DbgkpMarkProcessPeb;
	extern fnDbgkpPostFakeThreadMessages DbgkpPostFakeThreadMessages;
	extern fnKiStackAttachProcess KiStackAttachProcess;
	//extern fnKiStackAttachProcess Original_KiStackAttachProcess;
	extern fnKiUnstackDetachProcess KiUnstackDetachProcess;
	extern fnNtOpenProcess NtOpenProcess;
	//extern fnNtOpenProcess Original_NtOpenProcess;
	extern fnDbgkpConvertKernelToUserStateChange DbgkpConvertKernelToUserStateChange;
	extern fnDbgkpOpenHandles DbgkpOpenHandles;
	extern fnKeCopyExceptionRecord KeCopyExceptionRecord;
	extern fnObReferenceObjectByHandleWithTag ObReferenceObjectByHandleWithTag;
	extern fnObReferenceObjectByHandleWithTag Original_ObReferenceObjectByHandleWithTag;
	extern fnObReferenceObjectByHandle ObReferenceObjectByHandle;
	//extern fnObReferenceObjectByHandle Original_ObReferenceObjectByHandle;
	extern fnObfDereferenceObjectWithTag ObfDereferenceObjectWithTag;
	//extern fnObfDereferenceObjectWithTag Original_ObfDereferenceObjectWithTag;
	extern fnObfDereferenceObject ObfDereferenceObject;
	//extern fnObfDereferenceObject Original_ObfDereferenceObject;
	extern fnKeEnterCriticalRegionThread KeEnterCriticalRegionThread;
	extern fnKeLeaveCriticalRegionThread KeLeaveCriticalRegionThread;
	extern fnMmCopyVirtualMemory MmCopyVirtualMemory;
	//extern fnMmCopyVirtualMemory Original_MmCopyVirtualMemory;
	extern fnPspCreateUserContext PspCreateUserContext;
	//extern fnPspCreateUserContext Original_PspCreateUserContext;
	extern fnPspCallThreadNotifyRoutines PspCallThreadNotifyRoutines;
	//extern fnPspCallThreadNotifyRoutines Original_PspCallThreadNotifyRoutines;
	extern fnPspAllocateThread PspAllocateThread;
	//extern fnPspAllocateThread Original_PspAllocateThread;
	extern fnObpReferenceObjectByHandleWithTag ObpReferenceObjectByHandleWithTag;
	//extern fnObpReferenceObjectByHandleWithTag Original_ObpReferenceObjectByHandleWithTag;
	extern fnMiObtainReferencedVadEx MiObtainReferencedVadEx;
	//extern fnMiObtainReferencedVadEx Original_MiObtainReferencedVadEx;
	extern fnMmProtectVirtualMemory MmProtectVirtualMemory;
	//extern fnMmProtectVirtualMemory Original_MmProtectVirtualMemory;
	extern fnZwGetContextThread ZwGetContextThread;
	extern fnPspGetContextThreadInternal PspGetContextThreadInternal;
	extern fnKeStackAttachProcess KeStackAttachProcess;
	//extern fnKeStackAttachProcess Original_KeStackAttachProcess;
	extern fnNtSetInformationDebugObject NtSetInformationDebugObject;
	extern fnNtTerminateProcess NtTerminateProcess;
	//extern fnNtTerminateProcess Original_NtTerminateProcess;
	extern fnNtSuspendThread NtSuspendThread;
	//extern fnNtSuspendThread Original_NtSuspendThread;
	extern fnNtResumeThread NtResumeThread;
	//extern fnNtResumeThread Original_NtResumeThread;
	extern fnNtQueryInformationThread NtQueryInformationThread;
	//extern fnNtQueryInformationThread Original_NtQueryInformationThread;
	extern fnValidateHwnd ValidateHwnd;
	//extern fnValidateHwnd Original_ValidateHwnd;
	extern fnNtUserWindowFromPoint NtUserWindowFromPoint;
	//extern fnNtUserWindowFromPoint Original_NtUserWindowFromPoint;
	extern fnPsGetCurrentProcessByThread PsGetCurrentProcessByThread;
	extern fnPsQuerySystemDllInfo PsQuerySystemDllInfo;
	extern fnPsWow64GetProcessNtdllType PsWow64GetProcessNtdllType;
	extern fnPspReferenceSystemDll PspReferenceSystemDll;
	extern fnMiSectionControlArea MiSectionControlArea;
	extern fnDbgkpPostModuleMessages DbgkpPostModuleMessages;
	extern fnPspImplicitAssignProcessToJob PspImplicitAssignProcessToJob;
	extern fnPspUnlockProcessListExclusive PspUnlockProcessListExclusive;
	extern fnSepDeleteAccessState SepDeleteAccessState;

}

namespace Offset
{
	namespace EProcess
	{
		extern size_t Pcb;
		extern size_t DebugPort;
		extern size_t ImageFileName;
		extern size_t WoW64Process;
		extern size_t RundownProtect;
		extern size_t ExitTime;
		extern size_t Flags;
		extern size_t SectionBaseAddress;
		extern size_t SectionObject;
		extern size_t ObjectTable;
		extern size_t UniqueProcessId;
		extern size_t Job;
		extern size_t ActiveProcessLinks;
		extern size_t SequenceNumber;
		extern size_t ProcessLock;
	}

	namespace KProcess
	{
		extern size_t DirectoryTableBase;
	}

	namespace EThread
	{
		extern size_t Tcb;
		extern size_t CrossThreadFlags;
		extern size_t Cid;
		extern size_t RundownProtect;
		extern size_t Win32StartAddress;
		extern size_t SameThreadPassiveFlags;
	}

	namespace KThread
	{
		extern size_t ApcState;
		extern size_t PreviousMode;
		extern size_t Teb;
		extern size_t Process;
		extern size_t KernelApcDisable;
		extern size_t MiscFlags;
		extern size_t TrapFrame;
		extern size_t SuspendCount;
		extern size_t SpecialApcDisable;
	}

	namespace KApcState
	{
		extern size_t Process;
	}

	namespace ImageNtHeaders64
	{
		extern size_t Signature;
		extern size_t FileHeader;
		extern size_t OptionalHeader;
	}

	namespace ImageFileHeader
	{
		extern size_t PointerToSymbolTable;
		extern size_t NumberOfSymbols;
	}

	namespace EWow64Process
	{
		extern size_t Machine;
	}

	namespace Mmvad
	{
		extern size_t Core; //_MMVAD_SHORT Core;
	}

	namespace MmvadShort
	{
		extern size_t LongFlags;
	}

	namespace HandleTable
	{
		extern size_t UniqueProcessId;
	}

	namespace ObjectType
	{
		extern size_t TypeInfo;
	}

	namespace ObjectTypeInitializer
	{
		extern size_t GenericMapping;
	}
}


