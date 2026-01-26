#include "KernelCommon.pch.h"
#include "HideImport.h"

VOID InitializeHideImport(ULONG64 KernelBase)
{
	//A

	//B

	//C

	//D
	ImpSet(DbgPrint);
	ImpSet(DbgPrintEx);
	ImpSet(DbgBreakPointWithStatus);

	//E
	ImpSet(ExAllocatePoolWithTag);
	ImpSet(ExFreePoolWithTag);
	ImpSet(ExAllocatePool);
	ImpSet(ExFreePool);
	ImpSet(ExInitializeRundownProtection);
	ImpSet(ExWaitForRundownProtectionRelease);
	ImpSet(ExAcquireRundownProtection);
	ImpSet(ExReleaseRundownProtection);
	ImpSet(ExInitializeResourceLite);
	ImpSet(ExDeleteResourceLite);
	ImpSet(ExEnterCriticalRegionAndAcquireResourceShared);
	ImpSet(ExEnterCriticalRegionAndAcquireResourceExclusive);
	ImpSet(ExReleaseResourceAndLeaveCriticalRegion);
	ImpSet(ExGetPreviousMode);
	ImpSet(ExSystemTimeToLocalTime);
	ImpSet(ExInterlockedInsertHeadList);
	ImpSet(ExInterlockedRemoveHeadList);
	ImpSet(ExRaiseStatus);
	ImpSet(ExAcquireResourceSharedLite);
	ImpSet(ExReleaseResourceLite);
	ImpSet(ExAcquireResourceExclusiveLite);
	ImpSet(RtlInsertElementGenericTable);
	ImpSet(ExAcquireFastMutex);
	ImpSet(ExReleaseFastMutex);
	//F
	ImpSet(FsRtlIsNameInExpression);

	//G

	//H

	//I
	ImpSet(IoIs32bitProcess);
	ImpSet(IoGetCurrentProcess);
	ImpSet(IoAllocateMdl);
	ImpSet(IoFreeMdl);
	ImpSet(IoThreadToProcess);
	ImpSet(IoCreateFile);
	ImpSet(IoGetBaseFileSystemDeviceObject);
	ImpSet(IoCreateFileSpecifyDeviceObjectHint);
	ImpSet(IoIsSystemThread);
	ImpSet(IoQueryFileDosDeviceName);

	//G

	//K
	ImpSet(KeAttachProcess);
	ImpSet(KeDelayExecutionThread);
	ImpSet(KeDetachProcess);
	ImpSet(KeEnterGuardedRegion);
	ImpSet(KeLeaveGuardedRegion);
	ImpSet(KeUserModeCallback);
	ImpSet(KfRaiseIrql);
	ImpSet(KeRaiseIrqlToDpcLevel);
	ImpSet(KeLowerIrql);
	ImpSet(KeStackAttachProcess);
	ImpSet(KeUnstackDetachProcess);
	ImpSet(KeInitializeGuardedMutex);
	ImpSet(KeGetCurrentIrql);
	ImpSet(KeAcquireGuardedMutex);
	ImpSet(KeReleaseGuardedMutex);
	ImpSet(KeTryToAcquireGuardedMutex);
	ImpSet(KeQueryActiveProcessorCount);
	ImpSet(KeGetCurrentProcessorNumber);
	ImpSet(KeStallExecutionProcessor);
	ImpSet(KeInitializeDpc);
	ImpSet(KeSetTargetProcessorDpc);
	ImpSet(KeInsertQueueDpc);
	ImpSet(KeInitializeSpinLock);
	ImpSet(KeAcquireInStackQueuedSpinLock);
	ImpSet(KeReleaseInStackQueuedSpinLock);
	ImpSet(KeAcquireInStackQueuedSpinLockAtDpcLevel);
	ImpSet(KeReleaseInStackQueuedSpinLockFromDpcLevel);
	ImpSet(KeInvalidateAllCaches);
	ImpSet(KeGetCurrentThread);
	ImpSet(KeQueryTimeIncrement);
	ImpSet(KeQueryPerformanceCounter);
	ImpSet(KeBugCheckEx);
	ImpSet(KeBugCheck);
	ImpSet(KeEnterCriticalRegion);
	ImpSet(KeLeaveCriticalRegion);
	ImpSet(KeAddSystemServiceTable);
	ImpSet(KeInitializeEvent);
	ImpSet(KeSetEvent);
	ImpSet(KeWaitForSingleObject);
	ImpSet(KeSignalCallDpcSynchronize);
	ImpSet(KeSignalCallDpcDone);
	ImpSet(KeGenericCallDpc);
	ImpSet(KeAcquireSpinLockAtDpcLevel);
	ImpSet(KeReleaseSpinLockFromDpcLevel);
	ImpSet(KeQueryActiveProcessors);
	ImpSet(KeSetSystemAffinityThreadEx);
	ImpSet(KeSetSystemAffinityThread);
	ImpSet(KeRevertToUserAffinityThreadEx);
	ImpSet(KeFlushQueuedDpcs);
	ImpSet(KeQueryActiveProcessorCountEx);
	ImpSet(KeGetProcessorNumberFromIndex);
	ImpSet(KeSetSystemGroupAffinityThread);
	ImpSet(KeRevertToUserGroupAffinityThread);
	ImpSet(KeSetImportanceDpc);
	ImpSet(KeSetTargetProcessorDpcEx);
	ImpSet(KeInitializeApc);
	ImpSet(KeInsertQueueApc);
	ImpSet(KeClearEvent);
	ImpSet(KeReleaseSemaphore);
	ImpSet(KeWaitForMultipleObjects);
	ImpSet(KeInitializeSemaphore);
	ImpSet(KeReleaseMutex);
	ImpSet(KeQueryMaximumProcessorCount);
	ImpSet(KeInitializeMutex);
	ImpSet(KeTestAlertThread);

	//L

	//M
	ImpSet(memcpy);
	ImpSet(memset);
	ImpSet(MmBuildMdlForNonPagedPool);
	ImpSet(MmSecureVirtualMemory);
	ImpSet(MmGetVirtualForPhysical);
	ImpSet(MmMapLockedPages);
	ImpSet(MmUnmapLockedPages);
	ImpSet(MmIsAddressValid);
	ImpSet(MmCopyVirtualMemory);
	ImpSet(MmGetSystemRoutineAddress);
	ImpSet(MmMapLockedPagesSpecifyCache);
	ImpSet(MmProbeAndLockPages);
	ImpSet(MmMapViewInSystemSpace);
	ImpSet(MmUnmapViewInSystemSpace);
	ImpSet(MmAllocateContiguousMemory);
	ImpSet(MmFreeContiguousMemory);
	ImpSet(MmAllocatePagesForMdl);
	ImpSet(MmProtectMdlSystemAddress);
	ImpSet(MmFreePagesFromMdl);
	ImpSet(MmUnlockPages);
	ImpSet(MmGetPhysicalAddress);
	ImpSet(MmMapIoSpace);
	ImpSet(MmUnmapIoSpace);
	ImpSet(MmAllocatePagesForMdlEx);
	ImpSet(MmAllocateContiguousMemorySpecifyCache);
	ImpSet(MmGetPhysicalMemoryRanges);

	//N
	ImpSet(NtQuerySystemInformationEx);
	ImpSet(NtQueryInformationProcess);


	//O
	ImpSet(ObReferenceObjectByHandle);
	ImpSet(ObfDereferenceObject);
	ImpSet(ObOpenObjectByPointer);
	ImpSet(ObGetObjectType);
	ImpSet(ObQueryNameString);
	ImpSet(ObReferenceObjectByName);
	ImpSet(ObDuplicateObject);
	ImpSet(ObCloseHandle);

	//P
	ImpSet(PsGetProcessWow64Process);
	ImpSet(PsAcquireProcessExitSynchronization);
	ImpSet(PsGetProcessPeb);
	ImpSet(PsLookupProcessByProcessId);
	ImpSet(PsLookupThreadByThreadId);
	ImpSet(PsReleaseProcessExitSynchronization);
	ImpSet(PsReferencePrimaryToken);
	ImpSet(PsDereferencePrimaryToken);
	ImpSet(PsInitialSystemProcess);
	ImpSet(PsGetProcessId);
	ImpSet(PsLookupProcessThreadByCid);
	ImpSet(PsCreateSystemThread);
	ImpSet(PsGetCurrentThreadId);
	ImpSet(PsGetThreadProcess);
	ImpSet(PsGetProcessWin32Process);
	ImpSet(PsGetProcessDebugPort);
	ImpSet(PsSuspendProcess);
	ImpSet(PsResumeProcess);
	ImpSet(PsGetCurrentProcessId);
	ImpSet(PsWrapApcWow64Thread);
	ImpSet(PsGetCurrentThread);
	ImpSet(PsSetCreateProcessNotifyRoutine);
	ImpSet(PsSetCreateThreadNotifyRoutine);
	ImpSet(PsRemoveCreateThreadNotifyRoutine);
	ImpSet(PsSetLoadImageNotifyRoutine);
	ImpSet(PsRemoveLoadImageNotifyRoutine);
	ImpSet(PsGetContextThread);
	ImpSet(PsSetContextThread);
	ImpSet(PsIsThreadTerminating);
	ImpSet(PsGetCurrentProcessWow64Process);
	ImpSet(PsGetThreadTeb);
	ImpSet(PsIsSystemProcess);
	ImpSet(PsGetThreadId);
	ImpSet(PsGetThreadProcessId);

	//Q

	//R
	ImpSet(RtlGetVersion);
	ImpSet(RtlInitAnsiString);
	ImpSet(RtlInitUnicodeString);
	ImpSet(RtlAnsiStringToUnicodeString);
	ImpSet(RtlFreeUnicodeString);
	ImpSet(RtlCompareUnicodeString);
	ImpSet(RtlCreateUserThread);
	ImpSet(RtlUnicodeStringToAnsiString);
	ImpSet(RtlFreeAnsiString);
	ImpSet(RtlPcToFileHeader);
	ImpSet(RtlEqualString);
	ImpSet(RtlEqualUnicodeString);
	ImpSet(RtlInitString);
	ImpSet(RtlCompareString);
	ImpSet(RtlCreateUnicodeString);
	ImpSet(RtlImageNtHeader);
	ImpSet(RtlLookupElementGenericTableAvl);
	ImpSet(RtlDeleteElementGenericTableAvl);
	ImpSet(RtlVolumeDeviceToDosName);
	ImpSet(RtlRandomEx);
	ImpSet(RtlTimeToSecondsSince1970);
	ImpSet(RtlInitializeGenericTable);
	ImpSet(RtlLookupElementGenericTable);
	ImpSet(RtlDeleteElementGenericTable);
	ImpSet(RtlGetElementGenericTable);
	ImpSet(RtlCompareMemory);

	//S
	ImpSet(SeLocateProcessImageName);
	ImpSet(strncpy);
	ImpSet(strcmp);
	ImpSet(strcpy);
	//T

	//U

	//V
	ImpSet(vDbgPrintEx);

	//W
	ImpSet(wcsncmp);
	ImpSet(wcsncpy);
	ImpSet(wcslen);

	//X

	//Y

	//Z
	ImpSet(ZwCreateFile);
	ImpSet(ZwQueryInformationFile);
	ImpSet(ZwReadFile);
	ImpSet(ZwOpenFile);
	ImpSet(ZwCreateSection);
	ImpSet(ZwMapViewOfSection);
	ImpSet(ZwQueryInformationProcess);
	ImpSet(ZwLockVirtualMemory);
	ImpSet(ZwUnlockVirtualMemory);
	ImpSet(ZwQueryObject);
	ImpSet(ZwAllocateVirtualMemory);
	ImpSet(ZwClose);
	ImpSet(ZwFreeVirtualMemory);
	ImpSet(ZwQuerySystemInformation);
	ImpSet(ZwQueryVirtualMemory);
	ImpSet(ZwProtectVirtualMemory);
	ImpSet(ZwUnmapViewOfSection);
	ImpSet(ZwOpenDirectoryObject);
	ImpSet(ZwWaitForSingleObject);
	ImpSet(ZwQueryInformationThread);
	ImpSet(ZwOpenKey);
	ImpSet(ZwQueryValueKey);
	ImpSet(ZwSetInformationFile);
	ImpSet(ZwDeleteFile);
	ImpSet(ZwWriteFile);
	ImpSet(ZwOpenSection);
	ImpSet(ZwQuerySection);
	ImpSet(ZwOpenThread);

	ImpSet(_wcsnicmp);


	//LOG_DEBUG("内核模块起始地址:%p\r\n", KBase2);
}