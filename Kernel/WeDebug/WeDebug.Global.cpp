#include "WeDebug.pch.h"
#include "WeDebug.Global.h"



namespace Global
{
	PDRIVER_OBJECT g_DriverObject = nullptr;
	ULONG64 g_DriverBase = 0;
	ULONG32 g_DriverSize = 0;

	ULONG64 g_KernelBase = 0;
	ULONG32 g_KernelSize = 0;
    BOOLEAN g_IsInitSymbols = FALSE;
    BOOLEAN g_DbgkInitialized = FALSE;

    POBJECT_TYPE*         ObTypeIndexTable;

    FAST_MUTEX            DbgkpProcessDebugPortMutex;             //进程调试端口互斥锁
    POBJECT_TYPE          WeDebug_DbgkDebugObjectType;            //自建的调试对象类型
	//全局变量
	PVOID                 PspLoaderInitRoutine;
	POBJECT_TYPE*         DbgkDebugObjectType;                    //调试对象类型
	PULONG                PspNotifyEnableMask;
	PULONG                PerfGlobalGroupMask;
	PEX_PUSH_LOCK         PspActiveProcessLock;                   //活动进程列表锁
	PULONG_PTR            PspProcessSequenceNumber;               //进程序列号
	PLIST_ENTRY           PsActiveProcessHead;                    //活动进程列表

	FAST_MUTEX            LongFlagsMutex;
	PROTECTOBJ            g_ProtectFileObjList;                   //保护的文件对象列表
	PROTECTOBJ            g_ProtectWndObjList;                    //保护的窗口对象列表
	DEBUG_PROCESS_TABLE   g_DebugProcessList;                     //被调试的进程列表
	BREAKPOINT_TABLE      g_BreakpointList;                       //断点列表
	VIRTUAL_HANDLE_TABLE  g_VirtualHandleList;                    //虚拟句柄列表
    DEBUGGER_TABLE        g_DebuggerList;                         //调试器对象

 	fnObDuplicateObject ObDuplicateObject;  //复制对象
 	fnKeResumeThread KeResumeThread;
 	fnKeSuspendThread KeSuspendThread;
 	fnKeForceResumeThread KeForceResumeThread;
 	fnKeFreezeAllThreads KeFreezeAllThreads;
 	fnKeThawAllThreads KeThawAllThreads;
 	fnPsGetNextProcessThread PsGetNextProcessThread; //获取进程的下一个线程
 	fnPsQuitNextProcessThread PsQuitNextProcessThread; //线程对象解除引用
 	fnMmGetFileNameForAddress MmGetFileNameForAddress; //通过地址获取名称信息
 	fnMmGetFileNameForSection MmGetFileNameForSection; //通过节对象获取名称信息
 	fnLpcRequestWaitReplyPortEx LpcRequestWaitReplyPortEx; //请求端口
 	fnKeContextFromKframes KeContextFromKframes;
 	fnKeContextToKframes KeContextToKframes;
 	fnKiCheckForAtlThunk KiCheckForAtlThunk;
 	fnKiSegSsToTrapFrame KiSegSsToTrapFrame;
 	fnKiEspToTrapFrame KiEspToTrapFrame;
 	fnKiDebugRoutine KiDebugRoutine;
 	fnRtlDispatchException RtlDispatchException;
 	fnPsCallImageNotifyRoutines PsCallImageNotifyRoutines;  //调用映像回调例程
 	fnObGetProcessHandleCount ObGetProcessHandleCount;
 	fnPsGetProcessSessionId PsGetProcessSessionId;
 	fnObIsLUIDDeviceMapsEnabled ObIsLUIDDeviceMapsEnabled;
 	fnObQueryDeviceMapInformation ObQueryDeviceMapInformation;
 	fnMmGetImageInformation MmGetImageInformation;
 	fnMmGetExecuteOptions MmGetExecuteOptions;
 	fnExMapHandleToPointer ExMapHandleToPointer;
 	fnExUnlockHandleTableEntry ExUnlockHandleTableEntry;
 	fnZwAlpcSendWaitReceivePort ZwAlpcSendWaitReceivePort;
 	fnMmGetSectionInformation MmGetSectionInformation;
 	fnKiPreprocessFault KiPreprocessFault;
 	fnDbgkpStartSystemErrorHandler DbgkpStartSystemErrorHandler;
 	fnAlpcInitializeMessageAttribute AlpcInitializeMessageAttribute;
 	fnAlpcGetMessageAttribute AlpcGetMessageAttribute;
 	fnDbgkpRemoveErrorPort DbgkpRemoveErrorPort;
 	fnDbgkOpenProcessDebugPort DbgkOpenProcessDebugPort;
 	fnNtCreateDebugObject NtCreateDebugObject;
 	fnNtDebugActiveProcess NtDebugActiveProcess;
 	fnNtWaitForDebugEvent NtWaitForDebugEvent;
 	fnKiDispatchException KiDispatchException;
 	//fnKiDispatchException Original_KiDispatchException;
 	fnRtlInitializeExtendedContext RtlInitializeExtendedContext;
 	fnRtlGetExtendedContextLength RtlGetExtendedContextLength;
 	fnRtlpCopyExtendedContext RtlpCopyExtendedContext;
 	fnNtShutdownSystem NtShutdownSystem;
 	fnObCreateObjectType ObCreateObjectType;
 	fnSeAuditingWithTokenForSubcategory SeAuditingWithTokenForSubcategory;
 	fnSeAuditProcessCreation SeAuditProcessCreation;
 	fnPspGetJobFromSet PspGetJobFromSet;
 	fnPspBindProcessSessionToJob PspBindProcessSessionToJob;
 	fnPspAddProcessToJob PspAddProcessToJob;
 	fnKiCheckForKernelApcDelivery KiCheckForKernelApcDelivery;
 	fnDbgkCopyProcessDebugPort DbgkCopyProcessDebugPort;
 	fnSeCreateAccessStateEx SeCreateAccessStateEx;
 	fnObInsertObjectEx ObInsertObjectEx;
 	fnPspValidateJobAffinityState PspValidateJobAffinityState;
 	fnObCheckRefTraceProcess ObCheckRefTraceProcess;
 	fnSeDeleteAccessState SeDeleteAccessState;
 	fnPspInsertProcess PspInsertProcess;
 	fnPspInsertThread PspInsertThread;
 	fnNtDebugContinue NtDebugContinue;
 	fnDbgkMapViewOfSection DbgkMapViewOfSection;
 	fnDbgkUnMapViewOfSection DbgkUnMapViewOfSection;
 	fnDbgkpSuppressDbgMsg DbgkpSuppressDbgMsg;
 	fnPsReferenceProcessFilePointer PsReferenceProcessFilePointer;
 	fnObFastReferenceObject ObFastReferenceObject;
 	fnObFastReferenceObjectLocked ObFastReferenceObjectLocked;
 	fnMiReferenceControlAreaFile MiReferenceControlAreaFile;
 	fnObFastDereferenceObject ObFastDereferenceObject;
 	fnDbgkSendSystemDllMessages DbgkSendSystemDllMessages;
 	fnDbgkCreateThread DbgkCreateThread;
 	//fnDbgkCreateThread Original_DbgkCreateThread;
 	fnDbgkExitThread DbgkExitThread;
 	fnDbgkExitProcess DbgkExitProcess;
 	fnNtUserFindWindowEx NtUserFindWindowEx;
 	//fnNtUserFindWindowEx Original_NtUserFindWindowEx;
 	fnDbgkForwardException DbgkForwardException;
 	fnPsSynchronizeWithThreadInsertion PsSynchronizeWithThreadInsertion;
 	fnNtGetContextThread NtGetContextThread;
 	//fnNtGetContextThread Original_NtGetContextThread;
 	fnNtSetContextThread NtSetContextThread;
 	//fnNtSetContextThread Original_NtSetContextThread;
 	fnNtReadVirtualMemory NtReadVirtualMemory;
 	//fnNtReadVirtualMemory Original_NtReadVirtualMemory;
 	fnNtWriteVirtualMemory NtWriteVirtualMemory;
 	//fnNtWriteVirtualMemory Original_NtWriteVirtualMemory;
 	fnEtwTraceThread EtwTraceThread;
 	fnExReferenceCallBackBlock ExReferenceCallBackBlock;
 	fnExGetCallBackBlockRoutine ExGetCallBackBlockRoutine;
 	fnExDereferenceCallBackBlock ExDereferenceCallBackBlock;
 	fnObReferenceObjectSafe ObReferenceObjectSafe;
 	fnPspCatchCriticalBreak PspCatchCriticalBreak;
 	fnLpcRequestPort LpcRequestPort;
 	fnAlpcpSendMessage AlpcpSendMessage;
 	fnPspW32ThreadCallout* PspW32ThreadCallout;
 	fnPspW32ProcessCallout* PspW32ProcessCallout;
 	fnIoCancelThreadIo IoCancelThreadIo;
 	fnExTimerRundown ExTimerRundown;
 	fnCmNotifyRunDown CmNotifyRunDown;
 	fnKeRundownThread KeRundownThread;
 	fnPspUmsUnInitThread PspUmsUnInitThread;
 	fnKeRundownQueueEx KeRundownQueueEx;
 	fnRtlFreeUserStack RtlFreeUserStack;
 	fnMmDeleteTeb MmDeleteTeb;
 	fnPspExitProcess PspExitProcess;
 	fnSeAuditProcessExit SeAuditProcessExit;
 	fnObKillProcess ObKillProcess;
 	fnPspExitProcessFromJob PspExitProcessFromJob;
 	fnKeFlushQueueApc KeFlushQueueApc;
 	fnMmCleanProcessAddressSpace MmCleanProcessAddressSpace;
 	fnLpcExitProcess LpcExitProcess;
 	fnPspLegoNotifyRoutine* PspLegoNotifyRoutine;
 	fnKeSetProcess KeSetProcess;
 	fnKeTerminateThread KeTerminateThread;
 	fnPspExitThread PspExitThread;
 	//fnPspExitThread Original_PspExitThread;
 	fnPsGetNextProcess PsGetNextProcess;
 	fnPsTerminateProcess PsTerminateProcess;
 	fnDbgkpCloseObject DbgkpCloseObject;
 	fnKeStartThread KeStartThread;
 	fnObReferenceObjectEx ObReferenceObjectEx;
 	fnPspInitializeCpuQuotaApc PspInitializeCpuQuotaApc;
 	fnPspAdjustThreadSecurityDescriptor PspAdjustThreadSecurityDescriptor;
 	fnEtwTraceProcess EtwTraceProcess;
 	fnPspCreateObjectHandle PspCreateObjectHandle;
 	fnPspTerminateThreadByPointer PspTerminateThreadByPointer;
 	fnKeReadyThread KeReadyThread;
 	fnMmDeleteKernelStack MmDeleteKernelStack;
 	fnPsCaptureExceptionPort PsCaptureExceptionPort;
 	fnKeRaiseUserException KeRaiseUserException;
 	fnPspCreateThread PspCreateThread;
 	//fnPspCreateThread Original_PspCreateThread;
 	fnNtCreateThreadEx NtCreateThreadEx;
 	//fnNtCreateThreadEx Original_NtCreateThreadEx;
 	fnZwProtectVirtualMemory ZwProtectVirtualMemory;
 	fnNtProtectVirtualMemory NtProtectVirtualMemory;
 	//fnNtProtectVirtualMemory Original_NtProtectVirtualMemory;
 	fnPsFreezeProcess PsFreezeProcess;
 	fnPsThawProcess PsThawProcess;
 	fnDbgkpSuspendProcess DbgkpSuspendProcess;
 	fnDbgkpQueueMessage DbgkpQueueMessage;
 	fnDbgkpSectionToFileHandle DbgkpSectionToFileHandle;
 	fnDbgkpSendApiMessage DbgkpSendApiMessage;
 	fnDbgkpSendApiMessageLpc DbgkpSendApiMessageLpc;
 	fnDbgkpSendErrorMessage DbgkpSendErrorMessage;
 	fnPsSetProcessFaultInformation PsSetProcessFaultInformation;
 	fnDbgkpPostFakeProcessCreateMessages DbgkpPostFakeProcessCreateMessages;
 	//fnDbgkpSetProcessDebugObject DbgkpSetProcessDebugObject;
 	fnDbgkpWakeTarget DbgkpWakeTarget;
 	fnDbgkClearProcessDebugObject DbgkClearProcessDebugObject;
 	fnNtRemoveProcessDebug NtRemoveProcessDebug;
 	fnDbgkpMarkProcessPeb DbgkpMarkProcessPeb;
 	fnDbgkpPostFakeThreadMessages DbgkpPostFakeThreadMessages;
 	fnKiStackAttachProcess KiStackAttachProcess;
    //fnKiStackAttachProcess Original_KiStackAttachProcess;
    fnKiUnstackDetachProcess KiUnstackDetachProcess;
 	fnNtOpenProcess NtOpenProcess;
 	//fnNtOpenProcess Original_NtOpenProcess;
 	fnDbgkpConvertKernelToUserStateChange DbgkpConvertKernelToUserStateChange;
 	fnDbgkpOpenHandles DbgkpOpenHandles;
 	fnKeCopyExceptionRecord KeCopyExceptionRecord;
 	fnObReferenceObjectByHandleWithTag ObReferenceObjectByHandleWithTag;
 	fnObReferenceObjectByHandleWithTag Original_ObReferenceObjectByHandleWithTag;
 	fnObReferenceObjectByHandle ObReferenceObjectByHandle;
 	//fnObReferenceObjectByHandle Original_ObReferenceObjectByHandle;
 	fnObfDereferenceObjectWithTag ObfDereferenceObjectWithTag;
 	//fnObfDereferenceObjectWithTag Original_ObfDereferenceObjectWithTag;
 	fnObfDereferenceObject ObfDereferenceObject;
 	//fnObfDereferenceObject Original_ObfDereferenceObject;
 	fnKeEnterCriticalRegionThread KeEnterCriticalRegionThread;
 	fnKeLeaveCriticalRegionThread KeLeaveCriticalRegionThread;
 	fnMmCopyVirtualMemory MmCopyVirtualMemory;
 	//fnMmCopyVirtualMemory Original_MmCopyVirtualMemory;
 	fnPspCreateUserContext PspCreateUserContext;
 	//fnPspCreateUserContext Original_PspCreateUserContext;
 	fnPspCallThreadNotifyRoutines PspCallThreadNotifyRoutines;
 	//fnPspCallThreadNotifyRoutines Original_PspCallThreadNotifyRoutines;
 	fnPspAllocateThread PspAllocateThread;
 	//fnPspAllocateThread Original_PspAllocateThread;
 	fnObpReferenceObjectByHandleWithTag ObpReferenceObjectByHandleWithTag;
 	//fnObpReferenceObjectByHandleWithTag Original_ObpReferenceObjectByHandleWithTag;
 	fnMiObtainReferencedVadEx MiObtainReferencedVadEx;
 	//fnMiObtainReferencedVadEx Original_MiObtainReferencedVadEx;
 	fnMmProtectVirtualMemory MmProtectVirtualMemory;
 	//fnMmProtectVirtualMemory Original_MmProtectVirtualMemory;
 	fnZwGetContextThread ZwGetContextThread;
 	fnPspGetContextThreadInternal PspGetContextThreadInternal;
 	fnKeStackAttachProcess KeStackAttachProcess;
 	//fnKeStackAttachProcess Original_KeStackAttachProcess;
 	fnNtSetInformationDebugObject NtSetInformationDebugObject;
 	fnNtTerminateProcess NtTerminateProcess;
 	//fnNtTerminateProcess Original_NtTerminateProcess;
 	fnNtSuspendThread NtSuspendThread;
 	//fnNtSuspendThread Original_NtSuspendThread;
 	fnNtResumeThread NtResumeThread;
 	//fnNtResumeThread Original_NtResumeThread;
 	fnNtQueryInformationThread NtQueryInformationThread;
 	//fnNtQueryInformationThread Original_NtQueryInformationThread;
 	fnValidateHwnd ValidateHwnd;
 	//fnValidateHwnd Original_ValidateHwnd;
 	fnNtUserWindowFromPoint NtUserWindowFromPoint;
 	//fnNtUserWindowFromPoint Original_NtUserWindowFromPoint;
 	fnPsGetCurrentProcessByThread PsGetCurrentProcessByThread;
 	fnPsQuerySystemDllInfo PsQuerySystemDllInfo;
 	fnPsWow64GetProcessNtdllType PsWow64GetProcessNtdllType;
 	fnPspReferenceSystemDll PspReferenceSystemDll;
 	fnMiSectionControlArea MiSectionControlArea;
 	fnDbgkpPostModuleMessages DbgkpPostModuleMessages;
 	fnPspImplicitAssignProcessToJob PspImplicitAssignProcessToJob;
 	fnPspUnlockProcessListExclusive PspUnlockProcessListExclusive;
 	fnSepDeleteAccessState SepDeleteAccessState;

	BOOL Initialize_Global()
	{
		g_KernelProcess = IoGetCurrentProcess();

		OSVERSIONINFOW OsVersion;

		NTSTATUS Status = RtlGetVersion(&OsVersion);
		if (!NT_SUCCESS(Status))
		{
			LOG_DEBUG("[-] RtlGetVersion 失败\r\n");
			return FALSE;
		}

		g_CurrentWindowsBuildNumber = OsVersion.dwBuildNumber;

		LOG_DEBUG("[+] 当前系统版本:%d\r\n", g_CurrentWindowsBuildNumber);

		ExInitializeFastMutex(&LongFlagsMutex);
		InitializeList(&g_ProtectFileObjList.list_entry, &g_ProtectFileObjList.Mutex);
		InitializeList(&g_ProtectWndObjList.list_entry, &g_ProtectWndObjList.Mutex);
		InitializeList(&g_DebuggerList.list_entry, &g_DebuggerList.Mutex);
		InitializeList(&g_DebugProcessList.list_entry, &g_DebugProcessList.Mutex);
		InitializeList(&g_BreakpointList.list_entry, &g_BreakpointList.Mutex);
		InitializeList(&g_VirtualHandleList.list_entry, &g_VirtualHandleList.Mutex);

		return TRUE;
	}

	VOID UnInitialize_Global()
	{

	}
}

namespace Offset
{
	namespace EProcess
	{
		size_t Pcb;
		size_t DebugPort;
		size_t ImageFileName;
		size_t WoW64Process;
		size_t RundownProtect;
		size_t ExitTime;
		size_t Flags;
		size_t SectionBaseAddress;
		size_t SectionObject;
		size_t ObjectTable;
		size_t UniqueProcessId;
		size_t Job;
		size_t ActiveProcessLinks;
		size_t SequenceNumber;
		size_t ProcessLock;
	}

	namespace KProcess
	{
		size_t DirectoryTableBase;
	}

	namespace EThread
	{
		size_t Tcb;
		size_t CrossThreadFlags;
		size_t Cid;
		size_t RundownProtect;
		size_t Win32StartAddress;
		size_t SameThreadPassiveFlags;
	}

	namespace KThread
	{
		size_t ApcState;
		size_t PreviousMode;
		size_t Teb;
		size_t Process;
		size_t KernelApcDisable;
		size_t MiscFlags;
		size_t TrapFrame;
		size_t SuspendCount;
		size_t SpecialApcDisable;
	}

	namespace KApcState
	{
		size_t Process;
	}

	namespace ImageNtHeaders64
	{
		size_t Signature;
		size_t FileHeader;
		size_t OptionalHeader;
	}

	namespace ImageFileHeader
	{
		size_t PointerToSymbolTable;
		size_t NumberOfSymbols;
	}

	namespace EWow64Process
	{
		size_t Machine;
	}

	namespace Mmvad
	{
		size_t Core; //_MMVAD_SHORT Core;
	}

	namespace MmvadShort
	{
		size_t LongFlags;
	}

	namespace HandleTable
	{
		size_t UniqueProcessId;
	}

	namespace ObjectType
	{
		size_t TypeInfo;
	}

	namespace ObjectTypeInitializer
	{
		size_t GenericMapping;
	}
}




