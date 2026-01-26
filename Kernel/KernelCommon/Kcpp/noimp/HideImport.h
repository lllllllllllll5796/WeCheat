#pragma once

//Setup Import Defines
#define ImpDef(a) __declspec(selectany) decltype(&a) a##Fn = nullptr;
#define ImpSet(a) a##Fn = (decltype(&a))EPtr(GetProcAdress((PVOID)KernelBase, oxorany(#a)));
#define ImpCall(a, ...) ((decltype(&a))(EPtr(a##Fn)))(__VA_ARGS__)

//A

//B

//C

//D
ImpDef(DbgPrint);
ImpDef(DbgPrintEx);
ImpDef(DbgBreakPointWithStatus);


//E
ImpDef(ExAllocatePoolWithTag);
ImpDef(ExFreePoolWithTag);
ImpDef(ExAllocatePool);
ImpDef(ExFreePool);
ImpDef(ExInitializeRundownProtection);
ImpDef(ExWaitForRundownProtectionRelease);
ImpDef(ExAcquireRundownProtection);
ImpDef(ExReleaseRundownProtection);
ImpDef(ExInitializeResourceLite);
ImpDef(ExDeleteResourceLite);
ImpDef(ExEnterCriticalRegionAndAcquireResourceShared);
ImpDef(ExEnterCriticalRegionAndAcquireResourceExclusive);
ImpDef(ExReleaseResourceAndLeaveCriticalRegion);
ImpDef(ExGetPreviousMode);
ImpDef(ExSystemTimeToLocalTime);
ImpDef(ExInterlockedInsertHeadList);
ImpDef(ExInterlockedRemoveHeadList);
ImpDef(ExRaiseStatus);
ImpDef(ExAcquireResourceSharedLite);
ImpDef(ExReleaseResourceLite);
ImpDef(ExAcquireResourceExclusiveLite);
ImpDef(ExAcquireFastMutex);
ImpDef(ExReleaseFastMutex);


//F
ImpDef(FsRtlIsNameInExpression);

//G

//H

//I
ImpDef(IoIs32bitProcess);
ImpDef(IoGetCurrentProcess);
ImpDef(IoAllocateMdl);
ImpDef(IoFreeMdl);
ImpDef(IoThreadToProcess);
ImpDef(IoCreateFile);
ImpDef(IoGetBaseFileSystemDeviceObject);
ImpDef(IoCreateFileSpecifyDeviceObjectHint);
ImpDef(IoIsSystemThread);
ImpDef(IoQueryFileDosDeviceName);

//G

//K
ImpDef(KeAttachProcess);
ImpDef(KeDelayExecutionThread);
ImpDef(KeDetachProcess);
ImpDef(KeEnterGuardedRegion);
ImpDef(KeLeaveGuardedRegion);
ImpDef(KeUserModeCallback);
ImpDef(KfRaiseIrql);
ImpDef(KeRaiseIrqlToDpcLevel);
ImpDef(KeLowerIrql);
ImpDef(KeStackAttachProcess);
ImpDef(KeUnstackDetachProcess);
ImpDef(KeInitializeGuardedMutex);
ImpDef(KeGetCurrentIrql);
ImpDef(KeAcquireGuardedMutex);
ImpDef(KeReleaseGuardedMutex);
ImpDef(KeTryToAcquireGuardedMutex);
ImpDef(KeQueryActiveProcessorCount);
ImpDef(KeGetCurrentProcessorNumber);
ImpDef(KeStallExecutionProcessor);
ImpDef(KeInitializeDpc);
ImpDef(KeSetTargetProcessorDpc);
ImpDef(KeInsertQueueDpc);
ImpDef(KeInitializeSpinLock);
ImpDef(KeAcquireInStackQueuedSpinLock);
ImpDef(KeReleaseInStackQueuedSpinLock);
ImpDef(KeAcquireInStackQueuedSpinLockAtDpcLevel);
ImpDef(KeReleaseInStackQueuedSpinLockFromDpcLevel);
ImpDef(KeInvalidateAllCaches);
ImpDef(KeGetCurrentThread);
ImpDef(KeQueryTimeIncrement);
ImpDef(KeQueryPerformanceCounter);
ImpDef(KeBugCheckEx);
ImpDef(KeBugCheck);
ImpDef(KeEnterCriticalRegion);
ImpDef(KeLeaveCriticalRegion);
ImpDef(KeAddSystemServiceTable);
ImpDef(KeInitializeEvent);
ImpDef(KeSetEvent);
ImpDef(KeWaitForSingleObject);
ImpDef(KeSignalCallDpcSynchronize);
ImpDef(KeSignalCallDpcDone);
ImpDef(KeGenericCallDpc);
ImpDef(KeAcquireSpinLockAtDpcLevel);
ImpDef(KeReleaseSpinLockFromDpcLevel);
ImpDef(KeQueryActiveProcessors);
ImpDef(KeSetSystemAffinityThreadEx);
ImpDef(KeSetSystemAffinityThread);

ImpDef(KeRevertToUserAffinityThreadEx);
ImpDef(KeFlushQueuedDpcs);
ImpDef(KeQueryActiveProcessorCountEx);
ImpDef(KeGetProcessorNumberFromIndex);
ImpDef(KeSetSystemGroupAffinityThread);
ImpDef(KeRevertToUserGroupAffinityThread);
ImpDef(KeSetImportanceDpc);
ImpDef(KeSetTargetProcessorDpcEx);
ImpDef(KeInitializeApc);
ImpDef(KeInsertQueueApc);
ImpDef(KeClearEvent);
ImpDef(KeReleaseSemaphore);
ImpDef(KeWaitForMultipleObjects);
ImpDef(KeInitializeSemaphore);
ImpDef(KeReleaseMutex);
ImpDef(KeQueryMaximumProcessorCount);
ImpDef(KeInitializeMutex);
ImpDef(KeTestAlertThread);



//L

//M
ImpDef(memcpy);
ImpDef(memset);
ImpDef(MmBuildMdlForNonPagedPool);
ImpDef(MmSecureVirtualMemory);
ImpDef(MmGetVirtualForPhysical);
ImpDef(MmMapLockedPages);
ImpDef(MmUnmapLockedPages);
ImpDef(MmIsAddressValid);
ImpDef(MmCopyVirtualMemory);
ImpDef(MmGetSystemRoutineAddress);
ImpDef(MmMapLockedPagesSpecifyCache);
ImpDef(MmProbeAndLockPages);
ImpDef(MmMapViewInSystemSpace);
ImpDef(MmUnmapViewInSystemSpace);
ImpDef(MmAllocateContiguousMemory);
ImpDef(MmFreeContiguousMemory);
ImpDef(MmAllocatePagesForMdl);
ImpDef(MmProtectMdlSystemAddress);
ImpDef(MmFreePagesFromMdl);
ImpDef(MmUnlockPages);
ImpDef(MmGetPhysicalAddress);
ImpDef(MmMapIoSpace);
ImpDef(MmUnmapIoSpace);
ImpDef(MmAllocatePagesForMdlEx);
ImpDef(MmAllocateContiguousMemorySpecifyCache);
ImpDef(MmGetPhysicalMemoryRanges);



//N
ImpDef(NtQuerySystemInformationEx);
ImpDef(NtQueryInformationProcess);


//O
ImpDef(ObReferenceObjectByHandle);
ImpDef(ObfDereferenceObject);
ImpDef(ObOpenObjectByPointer);
ImpDef(ObGetObjectType);
ImpDef(ObQueryNameString);
ImpDef(ObReferenceObjectByName);
ImpDef(ObDuplicateObject);
ImpDef(ObCloseHandle);



//P
ImpDef(PsGetProcessWow64Process);
ImpDef(PsAcquireProcessExitSynchronization);
ImpDef(PsGetProcessPeb);
ImpDef(PsLookupProcessByProcessId);
ImpDef(PsLookupThreadByThreadId);
ImpDef(PsReleaseProcessExitSynchronization);
ImpDef(PsReferencePrimaryToken);
ImpDef(PsDereferencePrimaryToken);
ImpDef(PsInitialSystemProcess);
ImpDef(PsGetProcessId);
ImpDef(PsLookupProcessThreadByCid);
ImpDef(PsCreateSystemThread);
ImpDef(PsGetCurrentThreadId);
ImpDef(PsGetThreadProcess);
ImpDef(PsGetProcessWin32Process);
ImpDef(PsGetProcessDebugPort);
ImpDef(PsSuspendProcess);
ImpDef(PsResumeProcess);
ImpDef(PsGetCurrentProcessId);
ImpDef(PsWrapApcWow64Thread);
ImpDef(PsGetCurrentThread);
ImpDef(PsSetCreateProcessNotifyRoutine);
ImpDef(PsSetCreateThreadNotifyRoutine);
ImpDef(PsRemoveCreateThreadNotifyRoutine);
ImpDef(PsSetLoadImageNotifyRoutine);
ImpDef(PsRemoveLoadImageNotifyRoutine);
ImpDef(PsGetContextThread);
ImpDef(PsSetContextThread);
ImpDef(PsIsThreadTerminating);
ImpDef(PsGetCurrentProcessWow64Process);
ImpDef(PsGetThreadTeb);
ImpDef(PsIsSystemProcess);
ImpDef(PsGetThreadId);
ImpDef(PsGetThreadProcessId);



//Q

//R
ImpDef(RtlGetVersion);
ImpDef(RtlInitAnsiString);
ImpDef(RtlInitUnicodeString);
ImpDef(RtlAnsiStringToUnicodeString);
ImpDef(RtlFreeUnicodeString);
ImpDef(RtlCompareUnicodeString);
ImpDef(RtlCreateUserThread);
ImpDef(RtlUnicodeStringToAnsiString);
ImpDef(RtlFreeAnsiString);
ImpDef(RtlPcToFileHeader);
ImpDef(RtlEqualString);
ImpDef(RtlEqualUnicodeString);
ImpDef(RtlInitString);
ImpDef(RtlCompareString);
ImpDef(RtlCreateUnicodeString);
ImpDef(RtlImageNtHeader);
ImpDef(RtlLookupElementGenericTableAvl);
ImpDef(RtlDeleteElementGenericTableAvl);
ImpDef(RtlVolumeDeviceToDosName);
ImpDef(RtlRandomEx);
ImpDef(RtlTimeToSecondsSince1970);
ImpDef(RtlInitializeGenericTable);
ImpDef(RtlLookupElementGenericTable);
ImpDef(RtlDeleteElementGenericTable);
ImpDef(RtlInsertElementGenericTable);
ImpDef(RtlGetElementGenericTable);
ImpDef(RtlCompareMemory);


//S
ImpDef(SeLocateProcessImageName);
ImpDef(strncpy);
ImpDef(strcmp);
ImpDef(strcpy);

//T

//U

//V
ImpDef(vDbgPrintEx);


//W
ImpDef(wcsncmp);
ImpDef(wcsncpy);
ImpDef(wcslen);



//X

//Y

//Z
ImpDef(ZwCreateFile);
ImpDef(ZwQueryInformationFile);
ImpDef(ZwReadFile);
ImpDef(ZwOpenFile);
ImpDef(ZwCreateSection);
ImpDef(ZwMapViewOfSection);
ImpDef(ZwQueryInformationProcess);
ImpDef(ZwLockVirtualMemory);
ImpDef(ZwUnlockVirtualMemory);
ImpDef(ZwQueryObject);
ImpDef(ZwAllocateVirtualMemory);
ImpDef(ZwClose);
ImpDef(ZwFreeVirtualMemory);
ImpDef(ZwQuerySystemInformation);
ImpDef(ZwQueryVirtualMemory);
ImpDef(ZwProtectVirtualMemory);
ImpDef(ZwUnmapViewOfSection);
ImpDef(ZwOpenDirectoryObject);
ImpDef(ZwWaitForSingleObject);
ImpDef(ZwQueryInformationThread);
ImpDef(ZwOpenKey);
ImpDef(ZwQueryValueKey);
ImpDef(ZwSetInformationFile);
ImpDef(ZwDeleteFile);
ImpDef(ZwWriteFile);
ImpDef(ZwOpenSection);
ImpDef(ZwQuerySection);
ImpDef(ZwOpenThread);



ImpDef(_wcsnicmp);




VOID InitializeHideImport(ULONG64 KernelBase);

















