#include "WeDebug.pch.h"
#include "WeDebug.DbgkApi.h"

#define WOW64_ROUND_TO_PAGES(Size)  \
        (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#define WOW64_GET_TEB32_SAFE(teb64) \
        ((PTEB32) ((ULONGLONG)teb64 + WOW64_ROUND_TO_PAGES (sizeof(TEB))))

NTSTATUS DbgkInitialize()
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNICODE_STRING Name;
	OBJECT_TYPE_INITIALIZER ObjectTypeInitializer = { 0 };
	GENERIC_MAPPING GenericMapping = { STANDARD_RIGHTS_READ | DEBUG_READ_EVENT,
										STANDARD_RIGHTS_WRITE | DEBUG_PROCESS_ASSIGN,
										STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
										DEBUG_ALL_ACCESS };

	PAGED_CODE();

	LOG_DEBUG("[+] DbgkInitialize\r\n");

	/* Initialize the process debug port mutex */
	ExInitializeFastMutex(&Global::DbgkpProcessDebugPortMutex);

	/* Create the Debug Object Type */
	RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
	RtlInitUnicodeString(&Name, L"WeDebug_DebugObject");

	ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
	ObjectTypeInitializer.SecurityRequired = TRUE;
	ObjectTypeInitializer.InvalidAttributes = 0;
	ObjectTypeInitializer.PoolType = NonPagedPool;
	ObjectTypeInitializer.ValidAccessMask = DEBUG_ALL_ACCESS;
	ObjectTypeInitializer.GenericMapping = GenericMapping;
	//ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(DEBUG_OBJECT);    
	ObjectTypeInitializer.DefaultPagedPoolCharge = 0;
	ObjectTypeInitializer.DefaultNonPagedPoolCharge = 0;
	ObjectTypeInitializer.CloseProcedure = (ObCloseMethod)DbgkpCloseObject;  //注册回调函数
	ObjectTypeInitializer.DeleteProcedure = NULL;

	//Hvm_DbgkDebugObjectType = *DbgkDebugObjectType;

	Status = ObCreateObjectType(&Name, &ObjectTypeInitializer, NULL, &Global::WeDebug_DbgkDebugObjectType);
	if (!NT_SUCCESS(Status))
	{
		Global::WeDebug_DbgkDebugObjectType = GetDebugObjectType(Name);
		if (Global::WeDebug_DbgkDebugObjectType)
		{
			Global::WeDebug_DbgkDebugObjectType->TypeInfo.CloseProcedure = (ObCloseMethod)DbgkpCloseObject;
			Status = STATUS_SUCCESS;
		}
	}
	return Status;
}

POBJECT_TYPE GetDebugObjectType(UNICODE_STRING Name)
{
	//TypeIndex下标从2开始存储的才是有效指针
	for (DWORD TypeIndex = 2; Global::ObTypeIndexTable[TypeIndex] != NULL; TypeIndex++)
	{
		if (RtlEqualUnicodeString(&Global::ObTypeIndexTable[TypeIndex]->Name, &Name, FALSE))
		{
			return Global::ObTypeIndexTable[TypeIndex];
		}
	}
	return NULL;
}

VOID DbgkUnInitialize()
{
	PAGED_CODE();

	LOG_DEBUG("[+] DbgkUnInitialize\r\n");

	//if (Global::WeDebug_DbgkDebugObjectType)
	//{
	//    ObDereferenceObject(Global::WeDebug_DbgkDebugObjectType);
	//}    
}

EXTERN_C
VOID
DbgkpCloseObject(
    IN _EPROCESS* Process,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR SystemHandleCount
)
/*++

Routine Description:

    Called by the object manager when a handle is closed to the object.

Arguments:

    Process - Process doing the close
    Object - Debug object being deleted
    GrantedAccess - Access ranted for this handle
    ProcessHandleCount - Unused and unmaintained by OB
    SystemHandleCount - Current handle count for this object

Return Value:

    None.

--*/
{
    PDEBUG_OBJECT DebugObject = (PDEBUG_OBJECT)Object;
    PDEBUG_EVENT DebugEvent;
    PLIST_ENTRY ListPtr;
    BOOLEAN Deref;
    PDEBUG_OBJECT Port;
    PDEBUG_PROCESS DebugProcess;
    size_t ptr_DebugPort = NULL;

    PAGED_CODE();

    //DbgBreakPoint();

    UNREFERENCED_PARAMETER(GrantedAccess);

    CHECK_FUNC_WITHOUT_RETURN(PsGetNextProcess);
    CHECK_FUNC_WITHOUT_RETURN(DbgkpMarkProcessPeb);
    CHECK_FUNC_WITHOUT_RETURN(PsTerminateProcess);
    CHECK_FUNC_WITHOUT_RETURN(DbgkpWakeTarget);

    //
    // If this isn't the last handle then do nothing.
    //
    if (SystemHandleCount > 1) {
        return;
    }

    ExAcquireFastMutex(&DebugObject->Mutex);

    //
    // Mark this object as going away and wake up any processes that are waiting.
    //
    DebugObject->Flags |= DEBUG_OBJECT_DELETE_PENDING;

    //
    // Remove any events and queue them to a temporary queue
    //
    ListPtr = DebugObject->EventList.Flink;
    InitializeListHead(&DebugObject->EventList);

    ExReleaseFastMutex(&DebugObject->Mutex);

    //
    // Wake anyone waiting. They need to leave this object alone now as its deleting
    //
    KeSetEvent(&DebugObject->EventsPresent, 0, FALSE);

    //
    // Loop over all processes and remove the debug port from any that still have it.
    // Debug port propagation was disabled by setting the delete pending flag above so we only have to do this
    // once. No more refs can appear now.
    // 循环遍历所有进程，并删除仍然具有调试端口的进程。通过设置删除待定标志禁用了调试端口传播，因此我们只需执行一次此操作。
    // 现在不再会出现更多引用。
    //
    for (Process = (_EPROCESS*)Global::PsGetNextProcess(NULL); Process != NULL; Process = (_EPROCESS*)Global::PsGetNextProcess((PEPROCESS)Process))
    {
        if (IsDebugTargetProcess(Process, &DebugProcess))
        {
            Port = DebugProcess->DebugObject;
            ptr_DebugPort = NULL;
        }
        else
        {
            ptr_DebugPort = (size_t)Process + Offset::EProcess::DebugPort;
            Port = *(PDEBUG_OBJECT*)ptr_DebugPort;
        }

        if (Port == DebugObject)
        {
            Deref = FALSE;
            ExAcquireFastMutex(&Global::DbgkpProcessDebugPortMutex);
            if (Port == DebugObject)
            {
                if (ptr_DebugPort)
                {
                    *(PDEBUG_OBJECT*)ptr_DebugPort = NULL;
                }
                Deref = TRUE;
            }
            ExReleaseFastMutex(&Global::DbgkpProcessDebugPortMutex);


            if (Deref)
            {
                Global::DbgkpMarkProcessPeb((PEPROCESS)Process);  //清理进程的peb结构
                //
                // If the caller wanted process deletion on debugger dying (old interface) then kill off the process.
                //
                if (DebugObject->Flags & DEBUG_OBJECT_KILL_ON_CLOSE)
                {
                    LOG_DEBUG("结束进程");
                    Global::PsTerminateProcess((_KPROCESS*)Process, STATUS_DEBUGGER_INACTIVE);
                }
                ObDereferenceObject(DebugObject);
            }
        }
    }
    //
    // Wake up all the removed threads.
    //
    while (ListPtr != &DebugObject->EventList) {
        DebugEvent = CONTAINING_RECORD(ListPtr, DEBUG_EVENT, EventList);
        ListPtr = ListPtr->Flink;
        DebugEvent->Status = STATUS_DEBUGGER_INACTIVE;
        Global::DbgkpWakeTarget(DebugEvent);
    }

    LOG_DEBUG("移除调试对象\n");
    DeleteDebugProcess(DebugObject);
}

//判断目标进程是否是被调试的进程
BOOLEAN IsDebugTargetProcess(IN _EPROCESS* Process, OUT PDEBUG_PROCESS* DebugProcess)
{
	BOOLEAN result = FALSE;
	*DebugProcess = NULL;

	PLIST_ENTRY ListHead, NextEntry;
	PDEBUG_PROCESS entry;

	ExAcquireFastMutex(&Global::g_DebugProcessList.Mutex);
	ListHead = &Global::g_DebugProcessList.list_entry.ListHead;
	NextEntry = ListHead->Flink;
	while (ListHead != NextEntry)
	{
		entry = CONTAINING_RECORD(NextEntry,
			DEBUG_PROCESS,
			list_entry);

		if (entry)
		{
			if (entry->Process == Process)
			{
				*DebugProcess = entry;
				result = TRUE;
				break;
			}
		}

		/* Move to the next entry */
		NextEntry = NextEntry->Flink;
	}
	ExReleaseFastMutex(&Global::g_DebugProcessList.Mutex);
	return result;
}

//移除元素
VOID DeleteDebugProcess(PDEBUG_OBJECT DebugObject)
{
	PLIST_ENTRY ListHead, NextEntry;
	PDEBUG_PROCESS entry;

	ExAcquireFastMutex(&Global::g_DebugProcessList.Mutex);
	ListHead = &Global::g_DebugProcessList.list_entry.ListHead;
	NextEntry = ListHead->Flink;
	while (ListHead != NextEntry)
	{
		entry = CONTAINING_RECORD(NextEntry,
			DEBUG_PROCESS,
			list_entry);

		if (entry)
		{
			if (entry->DebugObject == DebugObject)
			{
				RemoveEntryList(NextEntry);
				free_pool(entry);
				break;
			}
		}

		/* Move to the next entry */
		NextEntry = NextEntry->Flink;
	}
	ExReleaseFastMutex(&Global::g_DebugProcessList.Mutex);
}