#include "WeVt.pch.h"
#include "CE.ProcessList.h"
#include "CE.Extradefines.h"
#include "CE.Extraimports.h"
#include "CE.Thread.h"

namespace ProcessList
{
	using namespace Thread;
	ProcessData* processlist = NULL;
	PRTL_GENERIC_TABLE InternalProcessList = NULL;

	PEPROCESS WatcherProcess = NULL;
	BOOLEAN ProcessWatcherOpensHandles = TRUE;

	BOOLEAN CreateProcessNotifyRoutineEnabled = FALSE;
	ERESOURCE ProcesslistR;

	ProcessEventdta ProcessEventdata[50];

	UCHAR ProcessEventCount;
	PKEVENT ProcessEvent;
	//HANDLE  ProcessEventHandle;

	ThreadEventDta ThreadEventData[50];

	UCHAR ThreadEventCount;
	PKEVENT ThreadEvent;
	//HANDLE  ThreadEventHandle;

	BOOLEAN CreateThreadNotifyRoutineEnabled;

	RTL_GENERIC_COMPARE_RESULTS NTAPI ProcessListCompare(__in struct _RTL_GENERIC_TABLE* Table, __in PProcessListData FirstStruct, __in PProcessListData SecondStruct)
	{
		//LOG_DEBUG("ProcessListCompate\n");

		if (FirstStruct->ProcessID == SecondStruct->ProcessID)
			return GenericEqual;
		else
		{
			if (SecondStruct->ProcessID < FirstStruct->ProcessID)
				return GenericLessThan;
			else
				return GenericGreaterThan;
		}
	}

	PVOID NTAPI ProcessListAlloc(__in struct _RTL_GENERIC_TABLE* Table, __in CLONG ByteSize)
	{
		PVOID r = ImpCall(ExAllocatePool, PagedPool, ByteSize);
		RtlZeroMemory(r, ByteSize);

		//LOG_DEBUG("ProcessListAlloc %d\n",(int)ByteSize);
		return r;
	}

	VOID NTAPI ProcessListDealloc(__in struct _RTL_GENERIC_TABLE* Table, __in __drv_freesMem(Mem) __post_invalid PVOID Buffer)
	{
		//LOG_DEBUG("ProcessListDealloc");
		ImpCall(ExFreePool,Buffer);
	}

	VOID GetThreadData(IN PDEVICE_OBJECT  DeviceObject, IN PVOID  Context)
	{
	    struct ThreadData *tempThreadEntry;
		PETHREAD selectedthread;
		HANDLE tid;
		LARGE_INTEGER Timeout;
		PKAPC AP;
		tempThreadEntry = (ThreadData*)Context;

		LOG_DEBUG("Gathering PEThread thread\n");

		Timeout.QuadPart = -1;
		ImpCall(KeDelayExecutionThread, KernelMode, TRUE, &Timeout);

		selectedthread = NULL;

		if (ImpCall(ExAcquireResourceSharedLite, &ProcesslistR, TRUE))
		{
			tid = tempThreadEntry->ThreadID;
			AP = &tempThreadEntry->SuspendApc;
			ImpCall(PsLookupThreadByThreadId, (PVOID)tid, &selectedthread);

			if (selectedthread)
			{
				LOG_DEBUG("PEThread=%p\n", selectedthread);

				ImpCall(KeInitializeApc,
					AP,
					(PKTHREAD)selectedthread,
					(wdk::KAPC_ENVIRONMENT)0,
					(PKKERNEL_ROUTINE)Ignore,
					(PKRUNDOWN_ROUTINE)NULL,
					(PKNORMAL_ROUTINE)SuspendThreadAPCRoutine,
					KernelMode,
					NULL);

				ImpCall(ObfDereferenceObject,selectedthread);
			}
			else
			{
				LOG_DEBUG("Failed getting the pethread.\n");
			}
		}
		ImpCall(ExReleaseResourceLite,&ProcesslistR);
	}

	VOID CreateThreadNotifyRoutine(IN HANDLE  ProcessId, IN HANDLE  ThreadId, IN BOOLEAN  Create)
	{
		if (ImpCall(KeGetCurrentIrql) == PASSIVE_LEVEL)
		{
			/*if (DebuggedProcessID==(ULONG)ProcessId)
			{
			//	PsSetContextThread (bah, xp only)
			}*/

			if (ImpCall(ExAcquireResourceExclusiveLite, &ProcesslistR, TRUE))
			{
				if (ThreadEventCount < 50)
				{
					ThreadEventData[ThreadEventCount].Created = Create;
					ThreadEventData[ThreadEventCount].ProcessID = (UINT_PTR)ProcessId;
					ThreadEventData[ThreadEventCount].ThreadID = (UINT_PTR)ThreadId;

					/*	if (Create)
							DbgPrint("Create ProcessID=%x\nThreadID=%x\n",(UINT_PTR)ProcessId,(UINT_PTR)ThreadId);
							else
							DbgPrint("Destroy ProcessID=%x\nThreadID=%x\n",(UINT_PTR)ProcessId,(UINT_PTR)ThreadId);
							*/

					ThreadEventCount++;
				}
			}

			ImpCall(ExReleaseResourceLite,&ProcesslistR);

			ImpCall(KeSetEvent, ThreadEvent, 0, FALSE);
			ImpCall(KeClearEvent, ThreadEvent);
		}
	}

	VOID CreateProcessNotifyRoutine(IN HANDLE  ParentId, IN HANDLE  ProcessId, IN BOOLEAN  Create)
	{
		PEPROCESS CurrentProcess = NULL;
		HANDLE ProcessHandle = 0;

		if (ImpCall(KeGetCurrentIrql) == PASSIVE_LEVEL)
		{
			struct ProcessData* tempProcessEntry;

			//aquire a spinlock
			if (ImpCall(ExAcquireResourceExclusiveLite, &ProcesslistR, TRUE))
			{

				if (ImpCall(PsLookupProcessByProcessId, (PVOID)ProcessId, &CurrentProcess) != STATUS_SUCCESS)
				{
					ImpCall(ExReleaseResourceLite, &ProcesslistR);
					return;
				}

				if ((ProcessWatcherOpensHandles) && (WatcherProcess))
				{


					if (Create)
					{
						//Open a handle to this process

						/*

						HANDLE ph = 0;
						NTSTATUS r = ObOpenObjectByPointer(CurrentProcess, 0, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &ph);

						DbgPrint("CreateProcessNotifyRoutine: ObOpenObjectByPointer=%x  ph=%x", r, ph);
						r = ZwDuplicateObject(ZwCurrentProcess(), ph, WatcherHandle, &ProcessHandle, PROCESS_ALL_ACCESS, 0, DUPLICATE_CLOSE_SOURCE);

						DbgPrint("CreateProcessNotifyRoutine: ZwDuplicateObject=%x (handle=%x)", r, ProcessHandle);
						*/

						::KAPC_STATE oldstate;


						ImpCall(KeStackAttachProcess, (PKPROCESS)WatcherProcess, &oldstate);
						//__try
						{
							//__try
							{
								ImpCall(ObOpenObjectByPointer, CurrentProcess, 0, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &ProcessHandle);
							}
// 							__except (1)
// 							{
// 								LOG_DEBUG("Exception during ObOpenObjectByPointer");
// 							}
						}
						//__finally
						{
							ImpCall(KeUnstackDetachProcess,&oldstate);
						}

					}


					if (InternalProcessList == NULL)
					{
						InternalProcessList = (PRTL_GENERIC_TABLE)ImpCall(ExAllocatePool, PagedPool, sizeof(RTL_GENERIC_TABLE));
						if (InternalProcessList)
							ImpCall(RtlInitializeGenericTable, InternalProcessList, (PRTL_GENERIC_COMPARE_ROUTINE)ProcessListCompare, ProcessListAlloc, ProcessListDealloc, NULL);
					}

					if (InternalProcessList)
					{
						ProcessListData d, * r;

						d.ProcessID = ProcessId;
						d.PEProcess = CurrentProcess;
						d.ProcessHandle = ProcessHandle;

						r = (ProcessListData*)ImpCall(RtlLookupElementGenericTable, InternalProcessList, &d);

						if (Create)
						{
							//add it to the list
							BOOLEAN newElement = FALSE;
							if (r) //weird
							{
								LOG_DEBUG("Duplicate PID detected...\n");
								ImpCall(RtlDeleteElementGenericTable, InternalProcessList, r);
							}

							r = (ProcessListData*)ImpCall(RtlInsertElementGenericTable, InternalProcessList, &d, sizeof(d), &newElement);


							LOG_DEBUG("Added handle %x for pid %d to the list (newElement=%d r=%p)\n", (int)(UINT_PTR)d.ProcessHandle, (int)(UINT_PTR)d.ProcessID, newElement, r);
						}
						else
						{
							//remove it from the list (if it's there)
							LOG_DEBUG("Process %d destruction. r=%p\n", (int)(UINT_PTR)d.ProcessID, r);
							if (r)
							{
								LOG_DEBUG("Process that was in the list has been closed\n");
								//if (r->ProcessHandle)
								//	ZwClose(r->ProcessHandle);

								//RtlDeleteElementGenericTable(InternalProcessList, r);
								r->Deleted = 1;
							}

							if (CurrentProcess == WatcherProcess)
							{
								LOG_DEBUG("CE Closed\n");

								//ZwClose(WatcherHandle);

								CleanProcessList(); //CE closed
								WatcherProcess = 0;
							}
						}
					}
				}


				//fill in a processcreateblock with data
				if (ProcessEventCount < 50)
				{
					ProcessEventdata[ProcessEventCount].Created = Create;
					ProcessEventdata[ProcessEventCount].ProcessID = (UINT_PTR)ProcessId;
					ProcessEventdata[ProcessEventCount].PEProcess = (UINT_PTR)CurrentProcess;
					ProcessEventCount++;
				}

				//if (!HiddenDriver)
				if (FALSE) //moved till next version
				{
					if (Create)
					{

						//allocate a block of memory for the processlist

						tempProcessEntry = (ProcessData*)ImpCall(ExAllocatePool, PagedPool, sizeof(struct ProcessData));
						tempProcessEntry->ProcessID = ProcessId;
						tempProcessEntry->PEProcess = CurrentProcess;
						tempProcessEntry->Threads = NULL;

						LOG_DEBUG("Allocated a process at:%p\n", tempProcessEntry);

						if (!processlist)
						{
							processlist = tempProcessEntry;
							processlist->next = NULL;
							processlist->previous = NULL;
						}
						else
						{
							tempProcessEntry->next = processlist;
							tempProcessEntry->previous = NULL;
							processlist->previous = tempProcessEntry;
							processlist = tempProcessEntry;
						}
					}
					else
					{
						//find this process and delete it
						tempProcessEntry = processlist;
						while (tempProcessEntry)
						{
							if (tempProcessEntry->ProcessID == ProcessId)
							{
								int i;
								if (tempProcessEntry->next)
									tempProcessEntry->next->previous = tempProcessEntry->previous;

								if (tempProcessEntry->previous)
									tempProcessEntry->previous->next = tempProcessEntry->next;
								else
									processlist = tempProcessEntry->next;	//it had no previous entry, so it's the root



								/*
								if (tempProcessEntry->Threads)
								{
								struct ThreadData *tempthread,*tempthread2;
								KIRQL OldIrql2;

								tempthread=tempProcessEntry->Threads;
								tempthread2=tempthread;

								DbgPrint("Process ended. Freeing threads\n");

								while (tempthread)
								{
								tempthread=tempthread->next;
								DbgPrint("Free thread %p (next thread=%p)\n",tempthread2,tempthread);
								ExFreePool(tempthread2);
								tempthread2=tempthread;
								}

								}


								ExFreePool(tempProcessEntry);*/

								i = 0;
								tempProcessEntry = processlist;
								while (tempProcessEntry)
								{
									i++;
									tempProcessEntry = tempProcessEntry->next;
								}

								LOG_DEBUG("There are %d processes in the list\n", i);

								break;
							}
							tempProcessEntry = tempProcessEntry->next;
						}


					}

				}
			}
			ImpCall(ExReleaseResourceLite,&ProcesslistR);

			if (CurrentProcess != NULL)
				ImpCall(ObfDereferenceObject,CurrentProcess);

			//signal process event (if there's one waiting for a signal)
			if (ProcessEvent)
			{
				ImpCall(KeSetEvent, ProcessEvent, 0, FALSE);
				ImpCall(KeClearEvent, ProcessEvent);
			}
		}
	}

	VOID CreateProcessNotifyRoutineEx(IN HANDLE  ParentId, IN HANDLE  ProcessId, __in_opt PPS_CREATE_NOTIFY_INFO CreateInfo)
	{
		LOG_DEBUG("CreateProcessNotifyRoutineEx\n");
		CreateProcessNotifyRoutine(ParentId, ProcessId, CreateInfo != NULL);
	}

	HANDLE GetHandleForProcessID(IN HANDLE ProcessID)
	{
		if (InternalProcessList)
		{
			ProcessListData d, *r;

			d.ProcessID = ProcessID;
			r = (ProcessListData*)ImpCall(RtlLookupElementGenericTable, InternalProcessList, &d);
			if (r)
			{
				LOG_DEBUG("Found a handle for PID %d (%x)\n", (int)(UINT_PTR)ProcessID, (int)(UINT_PTR)r->ProcessHandle);
				return r->ProcessHandle; // r->ProcessHandle;
			}
		}

		return 0;
	}

	VOID CleanProcessList()
	{
		if (InternalProcessList)
		{
			PProcessListData li;

			if (ImpCall(ExAcquireResourceExclusiveLite,&ProcesslistR, TRUE))
			{
				::KAPC_STATE oldstate;
				BOOLEAN ChangedContext;

				if ((WatcherProcess) && (WatcherProcess != ImpCall(IoGetCurrentProcess)))
				{
					ImpCall(KeStackAttachProcess,(PKPROCESS)WatcherProcess, &oldstate);
					ChangedContext = TRUE;
				}

				while (li = (PProcessListData)ImpCall(RtlGetElementGenericTable, InternalProcessList, 0))
				{
					if ((li->ProcessHandle) && (WatcherProcess))
						ImpCall(ZwClose,li->ProcessHandle);

					ImpCall(RtlDeleteElementGenericTable,InternalProcessList, li);
				}

				ImpCall(ExFreePool,InternalProcessList);
				InternalProcessList = NULL;
			}
			ImpCall(ExReleaseResourceLite,&ProcesslistR);
		}
	}
}