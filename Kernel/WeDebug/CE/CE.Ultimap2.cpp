#include "WeVt.pch.h"
#include "CE.Ultimap2.h"
#pragma warning(disable:4334)

namespace Ultimap2
{
	using namespace DBKUtil;
	PSSUSPENDPROCESS PsSuspendProcess;
	PSSUSPENDPROCESS PsResumeProcess;

	KDPC RTID_DPC;

	BOOL LogKernelMode;
	BOOL LogUserMode;

	PEPROCESS CurrentTarget;
	UINT64 CurrentCR3;
	HANDLE Ultimap2Handle;
	volatile BOOLEAN UltimapActive = FALSE;
	volatile BOOLEAN isSuspended = FALSE;
	volatile BOOLEAN flushallbuffers = FALSE; //set to TRUE if all the data should be flushed
	KEVENT FlushData;

	BOOL SaveToFile;
	WCHAR OutputPath[200];

	int Ultimap2RangeCount;
	PURANGE Ultimap2Ranges = NULL;

	PVOID* Ultimap2_DataReady;

#if (NTDDI_VERSION < NTDDI_VISTA)
	//implement this function for XP
	unsigned int KeQueryMaximumProcessorCount()
	{
		CCHAR cpunr;
		KAFFINITY cpus, original;
		ULONG cpucount;

		cpucount = 0;
		cpus = KeQueryActiveProcessors();
		original = cpus;
		while (cpus)
		{
			if (cpus % 2)
				cpucount++;

			cpus = cpus / 2;
		}

		return cpucount;
	}
#endif

	typedef struct
	{
		PToPA_ENTRY ToPAHeader;
		PToPA_ENTRY ToPAHeader2;

		PVOID ToPABuffer;
		PVOID ToPABuffer2;

		PMDL ToPABufferMDL;
		PMDL ToPABuffer2MDL;

		PRTL_GENERIC_TABLE ToPALookupTable;
		PRTL_GENERIC_TABLE ToPALookupTable2;

		KEVENT Buffer2ReadyForSwap;
		KEVENT InitiateSave;

		KEVENT DataReady;
		KEVENT DataProcessed;

		UINT64 CurrentOutputBase;
		UINT64 CurrentSaveOutputBase;
		UINT64 CurrentSaveOutputMask;

		UINT64 MappedAddress; //set by WaitForData  , use with continue
		UINT64 Buffer2FlushSize; //used by WaitForData


		KDPC OwnDPC;
		HANDLE WriterThreadHandle;

		//for saveToFile mode
		HANDLE FileHandle;
		KEVENT FileAccess;
		UINT64 TraceFileSize;

		volatile BOOL Interrupted;
	}  ProcessorInfo, * PProcessorInfo;
	volatile PProcessorInfo* PInfo;

	int Ultimap2CpuCount;


	KMUTEX SuspendMutex;
	KEVENT SuspendEvent;
	HANDLE SuspendThreadHandle;
	volatile int suspendCount;
	BOOL ultimapEnabled = FALSE;
	BOOL singleToPASystem = FALSE;
	BOOL NoPMIMode = FALSE;

	void suspendThread(PVOID StartContext)
		/* Thread responsible for suspending the target process when the buffer is getting full */
	{
		NTSTATUS wr;
		//__try
		{
			while (UltimapActive)
			{
				wr = ImpCall(KeWaitForSingleObject, &SuspendEvent, Executive, KernelMode, FALSE, NULL);
				if (!UltimapActive) return;

				LOG_DEBUG("suspendThread event triggered\n");
				ImpCall(KeWaitForSingleObject, &SuspendMutex, Executive, KernelMode, FALSE, NULL);
				if (!isSuspended)
				{
					if (CurrentTarget == 0)
					{
						if (PsSuspendProcess(CurrentTarget) == 0)
							isSuspended = TRUE;
						else
							LOG_DEBUG("Failed to suspend target\n");
					}
				}
				ImpCall(KeReleaseMutex, &SuspendMutex, FALSE);
			}
		}
// 		__except (1)
// 		{
// 			LOG_DEBUG("Exception in suspendThread thread\n");
// 		}
	}

	NTSTATUS ultimap2_continue(int cpunr)
	{
		NTSTATUS r = STATUS_UNSUCCESSFUL;
		if ((cpunr < 0) || (cpunr >= Ultimap2CpuCount))
		{
			LOG_DEBUG("ultimap2_continue(%d)\n", cpunr);
			return STATUS_UNSUCCESSFUL;
		}

		if (PInfo)
		{
			PProcessorInfo pi = PInfo[cpunr];

			if (pi->MappedAddress)
			{
				ImpCall(MmUnmapLockedPages, (PVOID)(UINT_PTR)pi->MappedAddress, pi->ToPABuffer2MDL); //unmap this memory
				pi->MappedAddress = 0;
				r = STATUS_SUCCESS;
			}
			else
				LOG_DEBUG("MappedAddress was 0\n");

			LOG_DEBUG("%d DataProcessed\n", cpunr);
			ImpCall(KeSetEvent, &pi->DataProcessed, 0, FALSE); //let the next swap happen if needed
		}

		return r;
	}

	NTSTATUS ultimap2_waitForData(ULONG timeout, PULTIMAP2DATAEVENT data)
	{
		NTSTATUS r = STATUS_UNSUCCESSFUL;

		//Wait for the events in the list
		//If an event is triggered find out which one is triggered, then map that block into the usermode space and return the address and block
		//That block will be needed to continue

		if (UltimapActive)
		{
			NTSTATUS wr = STATUS_UNSUCCESSFUL;
			LARGE_INTEGER wait;
			PKWAIT_BLOCK waitblock;

			int cpunr;

			waitblock = (PKWAIT_BLOCK)ImpCall(ExAllocatePool, NonPagedPool, Ultimap2CpuCount * sizeof(KWAIT_BLOCK));
			wait.QuadPart = -10000LL * timeout;

			if (timeout == 0xffffffff) //infinite wait
				wr = ImpCall(KeWaitForMultipleObjects, Ultimap2CpuCount, Ultimap2_DataReady, WaitAny, UserRequest, UserMode, TRUE, NULL, waitblock);
			else
				wr = ImpCall(KeWaitForMultipleObjects, Ultimap2CpuCount, Ultimap2_DataReady, WaitAny, UserRequest, UserMode, TRUE, &wait, waitblock);

			ImpCall(ExFreePool,waitblock);

			LOG_DEBUG("ultimap2_waitForData wait returned %x\n", wr);

			cpunr = wr - STATUS_WAIT_0;


			if ((cpunr < Ultimap2CpuCount) && (cpunr >= 0))
			{
				PProcessorInfo pi = PInfo[cpunr];




				if (pi->Buffer2FlushSize)
				{
					if (pi->ToPABuffer2MDL)
					{
						//__try
						{

							data->Address = (UINT64)ImpCall(MmMapLockedPagesSpecifyCache, pi->ToPABuffer2MDL, UserMode, MmCached, NULL, FALSE, NormalPagePriority);

							LOG_DEBUG("MmMapLockedPagesSpecifyCache returned address %p\n", data->Address);

							if (data->Address)
							{
								data->Size = pi->Buffer2FlushSize;
								data->CpuID = cpunr;

								pi->MappedAddress = data->Address;
								r = STATUS_SUCCESS;
							}

						}
// 						__except (1)
// 						{
// 							LOG_DEBUG("ultimap2_waitForData: Failure mapping memory into waiter process. Count=%d\n", (int)MmGetMdlByteCount(pi->ToPABuffer2MDL));
// 						}
					}
					else
					{
						LOG_DEBUG("ToPABuffer2MDL is NULL. Not even gonna try\n");
					}
				}
				else
				{
					LOG_DEBUG("ultimap2_waitForData flushsize was 0\n");
				}
			}

		}

		LOG_DEBUG("ultimap2_waitForData returned %x\n", r);
		return r;
	}

	void createUltimap2OutputFile(int cpunr)
	{
		NTSTATUS r;
		PProcessorInfo pi = PInfo[cpunr];
		UNICODE_STRING usFile;
		OBJECT_ATTRIBUTES oaFile;
		IO_STATUS_BLOCK iosb;
		WCHAR Buffer[200];

#ifdef AMD64	
		LOG_DEBUG("OutputPath=%S\n", OutputPath);
		swprintf_s(Buffer, 200, oxorany(L"%sCPU%d.trace"), OutputPath, cpunr);
#else
		RtlStringCbPrintfW(Buffer, 200, L"%sCPU%d.trace", OutputPath, cpunr);
#endif

		LOG_DEBUG("Buffer=%S\n", Buffer);

		ImpCall(RtlInitUnicodeString, &usFile, Buffer);

		InitializeObjectAttributes(&oaFile, &usFile, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		LOG_DEBUG("Creating file %S\n", usFile.Buffer);

		pi->FileHandle = 0;
		ImpCall(ZwDeleteFile, &oaFile);
		r = ImpCall(ZwCreateFile, &pi->FileHandle, SYNCHRONIZE | FILE_READ_DATA | FILE_APPEND_DATA | GENERIC_ALL, &oaFile, &iosb, 0, FILE_ATTRIBUTE_NORMAL, 0, FILE_SUPERSEDE, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		LOG_DEBUG("%d: ZwCreateFile=%x\n", (int)cpunr, r);
	}

	void WriteThreadForSpecificCPU(PVOID StartContext)
	{
		int cpunr = (int)(UINT_PTR)StartContext;
		PProcessorInfo pi = PInfo[cpunr];



		IO_STATUS_BLOCK iosb;
		NTSTATUS r = STATUS_UNSUCCESSFUL;


		//LOG_DEBUG("WriteThreadForSpecificCPU %d alive", (int)StartContext);



		if (SaveToFile)
		{
			if (ImpCall(KeWaitForSingleObject, &pi->FileAccess, Executive, KernelMode, FALSE, NULL) == STATUS_SUCCESS)
			{
				createUltimap2OutputFile(cpunr);
				ImpCall(KeSetEvent, &pi->FileAccess, 0, FALSE);
			}
			else
				createUltimap2OutputFile(cpunr);
		}


		ImpCall(KeSetSystemAffinityThread, (KAFFINITY)(1 << cpunr));

		while (UltimapActive)
		{
			NTSTATUS wr = ImpCall(KeWaitForSingleObject, &pi->InitiateSave, Executive, KernelMode, FALSE, NULL);
			//DbgPrint("WriteThreadForSpecificCPU %d:  wr=%x", (int)StartContext, wr);
			if (!UltimapActive)
				break;

			if (wr == STATUS_SUCCESS)
			{
				UINT64 Size;
				ToPA_LOOKUP tl;
				PToPA_LOOKUP result;

				//DbgPrint("%d: writing buffer", (int)StartContext);

				//figure out the size
				tl.PhysicalAddress = pi->CurrentSaveOutputBase;
				tl.index = 0;
				result = (PToPA_LOOKUP)ImpCall(RtlLookupElementGenericTable, pi->ToPALookupTable2, &tl);

				if (result)
				{
					//write...
					//DbgPrint("%d: result->index=%d CurrentSaveOutputMask=%p", (int)StartContext, result->index, pi->CurrentSaveOutputMask);
					if (singleToPASystem)
						Size = pi->CurrentSaveOutputMask >> 32;
					else
						Size = ((result->index * 511) + ((pi->CurrentSaveOutputMask & 0xffffffff) >> 7)) * 4096 + (pi->CurrentSaveOutputMask >> 32);

					if (Size > 0)
					{

						if (SaveToFile)
						{
							wr = ImpCall(KeWaitForSingleObject, &pi->FileAccess, Executive, KernelMode, FALSE, NULL);
							if (wr == STATUS_SUCCESS)
							{
								if (pi->FileHandle == 0) //a usermode flush has happened
									createUltimap2OutputFile(cpunr);

								r = ImpCall(ZwWriteFile, pi->FileHandle, NULL, NULL, NULL, &iosb, pi->ToPABuffer2, (ULONG)Size, NULL, NULL);

								pi->TraceFileSize += Size;
								//DbgPrint("%d: ZwCreateFile(%p, %d)=%x\n", (int)StartContext, pi->ToPABuffer2, (ULONG)Size, r);

								ImpCall(KeSetEvent, &pi->FileAccess, 0, FALSE);
							}
						}
						else
						{
							//map ToPABuffer2 into the CE process

							//wake up a worker thread
							pi->Buffer2FlushSize = Size;
							LOG_DEBUG("%d: WorkerThread(%p, %d)=%x\n", (int)(UINT_PTR)StartContext, pi->ToPABuffer2, (ULONG)Size, r);
							ImpCall(KeSetEvent, &pi->DataReady, 0, TRUE); //a ce thread waiting in ultimap2_waitForData should now wake and process the data
							//and wait for it to finish
							r = ImpCall(KeWaitForSingleObject, &pi->DataProcessed, Executive, KernelMode, FALSE, NULL);
							LOG_DEBUG("KeWaitForSingleObject(DataProcessed)=%x", r);

						}
						//DbgPrint("%d: Writing %x bytes\n", (int)StartContext, Size);
					}


				}
				else
					LOG_DEBUG("Unexpected physical address while writing results for cpu %d  (%p)", (int)(UINT_PTR)StartContext, pi->CurrentSaveOutputBase);


				ImpCall(KeSetEvent, &pi->Buffer2ReadyForSwap, 0, FALSE);
			}
		}

		ImpCall(KeSetSystemAffinityThread, ImpCall(KeQueryActiveProcessors));

		if (pi->FileHandle)
			ImpCall(ZwClose, pi->FileHandle);

		ImpCall(KeSetEvent, &pi->Buffer2ReadyForSwap, 0, FALSE);
	}

	void ultimap2_LockFile(int cpunr)
	{
		if ((cpunr < 0) || (cpunr >= Ultimap2CpuCount))
			return;

		if (PInfo)
		{
			NTSTATUS wr;
			PProcessorInfo pi = PInfo[cpunr];

			//LOG_DEBUG("AcquireUltimap2File()\n");
			wr = ImpCall(KeWaitForSingleObject, &pi->FileAccess, Executive, KernelMode, FALSE, NULL);
			if (wr == STATUS_SUCCESS)
			{
				//LOG_DEBUG("Acquired\n");
				if (pi->FileHandle)
				{
					ImpCall(ZwClose, pi->FileHandle);
					pi->FileHandle = 0;
				}
			}
		}
	}

	void ultimap2_ReleaseFile(int cpunr)
	{
		if ((cpunr < 0) || (cpunr >= Ultimap2CpuCount))
			return;

		if (PInfo)
		{
			PProcessorInfo pi = PInfo[cpunr];
			ImpCall(KeSetEvent, &pi->FileAccess, 0, FALSE);
			//LOG_DEBUG("Released");
		}
	}

	UINT64 ultimap2_GetTraceFileSize()
	//Gets an aproximation of the filesize.  Don't take this too exact
	{
		UINT64 size = 0;

		if (PInfo)
		{
			int i;
			for (i = 0; i < Ultimap2CpuCount; i++)
				size += PInfo[i]->TraceFileSize;
		}

		return size;
	}

	void ultimap2_ResetTraceFileSize()
	{
		if (PInfo)
		{
			int i;
			for (i = 0; i < Ultimap2CpuCount; i++)
				PInfo[i]->TraceFileSize = 0;
		}
	}

	void SwitchToPABuffer(struct _KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
		/*
		DPC routine that switches the Buffer pointer and marks buffer2 that it's ready for data saving
		Only called when buffer2 is ready for flushing
		*/
	{
		//write the contents of the current cpu buffer
		PProcessorInfo pi = PInfo[KeGetCurrentProcessorNumber()];

		//DbgPrint("SwitchToPABuffer for cpu %d\n", KeGetCurrentProcessorNumber());

		if (pi)
		{
			UINT64 CTL = KernelIntrin__readmsr(IA32_RTIT_CTL);
			UINT64 Status = KernelIntrin__readmsr(IA32_RTIT_STATUS);
			PVOID temp;

			if ((Status >> 5) & 1) //Stopped
				LOG_DEBUG("%d Not all data recorded\n", KeGetCurrentProcessorNumber());


			if ((Status >> 4) & 1)
				LOG_DEBUG("ALL LOST\n");

			//only if the buffer is bigger than 2 pages.  That you can check in IA32_RTIT_OUTPUT_MASK_PTRS and IA32_RTIT_OUTPUT_BASE 
			//if (KeGetCurrentProcessorNumber() == 0)
			//	LOG_DEBUG("%d: pi->CurrentOutputBase=%p __readmsr(IA32_RTIT_OUTPUT_BASE)=%p __readmsr(IA32_RTIT_OUTPUT_MASK_PTRS)=%p", KeGetCurrentProcessorNumber(), pi->CurrentOutputBase, __readmsr(IA32_RTIT_OUTPUT_BASE), __readmsr(IA32_RTIT_OUTPUT_MASK_PTRS));


			if (pi->Interrupted == FALSE)
			{
				//return; //debug test. remove me when released

				if (!singleToPASystem)
				{
					if ((!flushallbuffers) && (((KernelIntrin__readmsr(IA32_RTIT_OUTPUT_MASK_PTRS) & 0xffffffff) >> 7) < 2))
						return; //don't flush yet
				}
				else
				{
					INT64 offset = KernelIntrin__readmsr(IA32_RTIT_OUTPUT_MASK_PTRS);

					/*if (KeGetCurrentProcessorNumber() == 0)
					{
						LOG_DEBUG("pi->CurrentOutputBase=%p\n", pi->CurrentOutputBase);
						LOG_DEBUG("offset=%p\n", offset);
					}*/

					offset = offset >> 32;

					//if (KeGetCurrentProcessorNumber() == 0)
					//	LOG_DEBUG("offset=%p\n", offset);

					if ((!flushallbuffers) && (((pi->CurrentOutputBase == 0) || (offset < 8192))))
						return; //don't flush yet
				}
			}
			else
			{
				LOG_DEBUG("%d:Flushing because of interrupt\n", KeGetCurrentProcessorNumber());
			}

			LOG_DEBUG("%d: Flush this data (%p)\n", KeGetCurrentProcessorNumber(), KernelIntrin__readmsr(IA32_RTIT_OUTPUT_MASK_PTRS));
			//LOG_DEBUG("%d: pi->CurrentOutputBase=%p __readmsr(IA32_RTIT_OUTPUT_BASE)=%p __readmsr(IA32_RTIT_OUTPUT_MASK_PTRS)=%p", KeGetCurrentProcessorNumber(), pi->CurrentOutputBase, __readmsr(IA32_RTIT_OUTPUT_BASE), __readmsr(IA32_RTIT_OUTPUT_MASK_PTRS));

			KernelIntrin__writemsr(IA32_RTIT_CTL, 0); //disable packet generation
			KernelIntrin__writemsr(IA32_RTIT_STATUS, 0);


			//LOG_DEBUG("%d: pi->CurrentOutputBase=%p __readmsr(IA32_RTIT_OUTPUT_BASE)=%p __readmsr(IA32_RTIT_OUTPUT_MASK_PTRS)=%p", KeGetCurrentProcessorNumber(), pi->CurrentOutputBase, __readmsr(IA32_RTIT_OUTPUT_BASE), __readmsr(IA32_RTIT_OUTPUT_MASK_PTRS));


			//switch the pointer to the secondary buffers
			ImpCall(KeClearEvent,&pi->Buffer2ReadyForSwap);

			//swap the buffer
			temp = pi->ToPABuffer;
			pi->ToPABuffer = pi->ToPABuffer2;
			pi->ToPABuffer2 = temp;

			//swap the MDL that describes it
			temp = pi->ToPABufferMDL;
			pi->ToPABufferMDL = pi->ToPABuffer2MDL;
			pi->ToPABuffer2MDL = (PMDL)temp;

			//swap the header
			temp = pi->ToPAHeader;
			pi->ToPAHeader = pi->ToPAHeader2;
			pi->ToPAHeader2 = (PToPA_ENTRY)temp;

			//swap the lookup table
			temp = pi->ToPALookupTable;
			pi->ToPALookupTable = pi->ToPALookupTable2;
			pi->ToPALookupTable2 = (PRTL_GENERIC_TABLE)temp;

			//lookup which entry it's pointing at
			pi->CurrentSaveOutputBase = KernelIntrin__readmsr(IA32_RTIT_OUTPUT_BASE);
			pi->CurrentSaveOutputMask = KernelIntrin__readmsr(IA32_RTIT_OUTPUT_MASK_PTRS);

			ImpCall(KeSetEvent,&pi->InitiateSave, 0, FALSE);

			pi->Interrupted = FALSE;

			//reactivate packet generation
			pi->CurrentOutputBase = ImpCall(MmGetPhysicalAddress,pi->ToPAHeader).QuadPart;

			KernelIntrin__writemsr(IA32_RTIT_OUTPUT_BASE, pi->CurrentOutputBase);
			KernelIntrin__writemsr(IA32_RTIT_OUTPUT_MASK_PTRS, 0);

			KernelIntrin__writemsr(IA32_RTIT_CTL, CTL);
		}
	}

	void WaitForWriteToFinishAndSwapWriteBuffers(BOOL interruptedOnly)
	{
		int i;

		for (i = 0; i < Ultimap2CpuCount; i++)
		{
			PProcessorInfo pi = PInfo[i];
			if ((pi->ToPABuffer2) && ((pi->Interrupted) || (!interruptedOnly)))
			{
				ImpCall(KeWaitForSingleObject, &pi->Buffer2ReadyForSwap, Executive, KernelMode, FALSE, NULL);

				if (!UltimapActive) return;

				ImpCall(KeInsertQueueDpc, &pi->OwnDPC, NULL, NULL);
			}

		}

		ImpCall(KeFlushQueuedDpcs);
	}

	void bufferWriterThread(PVOID StartContext)
	{
		//passive mode

		//wait for event
		LARGE_INTEGER Timeout;
		NTSTATUS wr;

		LOG_DEBUG("bufferWriterThread active\n");


		while (UltimapActive)
		{
			if (NoPMIMode)
				Timeout.QuadPart = -1000LL;  //- 10000LL=1 millisecond //-100000000LL = 10 seconds   -1000000LL= 0.1 second
			else
				Timeout.QuadPart = -10000LL;  //- 10000LL=1 millisecond //-100000000LL = 10 seconds   -1000000LL= 0.1 second

			//DbgPrint("%d : Wait for FlushData", cpunr());
			wr = ImpCall(KeWaitForSingleObject, &FlushData, Executive, KernelMode, FALSE, &Timeout);
			//DbgPrint("%d : After wait for FlushData", cpunr());
			//wr = KeWaitForSingleObject(&FlushData, Executive, KernelMode, FALSE, NULL);

			//DbgPrint("bufferWriterThread: Alive (wr==%x)", wr);
			if (!UltimapActive)
			{
				LOG_DEBUG("bufferWriterThread: Terminating\n");
				return;
			}

			//if (wr != STATUS_SUCCESS) continue; //DEBUG code so PMI's get triggered



			if ((wr == STATUS_SUCCESS) || (wr == STATUS_TIMEOUT))
			{
				if ((wr == STATUS_SUCCESS) && (!isSuspended))
				{
					//woken up by a dpc				
					LOG_DEBUG("FlushData event set and not suspended. Suspending target process\n");
					ImpCall(KeWaitForSingleObject, &SuspendMutex, Executive, KernelMode, FALSE, NULL);
					if (!isSuspended)
					{
						LOG_DEBUG("Still going to suspend target process\n");
						if (PsSuspendProcess(CurrentTarget) == 0)
							isSuspended = TRUE;
					}
					ImpCall(KeReleaseMutex, &SuspendMutex, FALSE);

					LOG_DEBUG("After the target has been suspended (isSuspended=%d)\n", isSuspended);
				}

				if (wr == STATUS_SUCCESS) //the filled cpu's must take preference
				{
					unsigned int i;
					BOOL found = TRUE;

					//LOG_DEBUG("bufferWriterThread: Suspended\n");


					//first flush the CPU's that complained their buffers are full
					LOG_DEBUG("Flushing full CPU\'s\n");
					while (found)
					{
						WaitForWriteToFinishAndSwapWriteBuffers(TRUE);
						if (!UltimapActive) return;

						//check if no interrupt has been triggered while this was busy ('could' happen as useless info like core ratio is still recorded)
						found = FALSE;
						for (i = 0; i < ImpCall(KeQueryMaximumProcessorCount); i++)
						{
							if (PInfo[i]->Interrupted)
							{
								LOG_DEBUG("PInfo[%d]->Interrupted\n", PInfo[i]->Interrupted);
								found = TRUE;
								break;
							}
						}
					}
				}

				//wait till the previous buffers are done writing
				//LOG_DEBUG("%d: Normal flush\n", cpunr());
				WaitForWriteToFinishAndSwapWriteBuffers(FALSE);
				//LOG_DEBUG("%d : after flush\n", cpunr());

				if (isSuspended)
				{
					ImpCall(KeWaitForSingleObject, &SuspendMutex, Executive, KernelMode, FALSE, NULL);
					if (isSuspended)
					{
						LOG_DEBUG("Resuming target process\n");
						PsResumeProcess(CurrentTarget);
						isSuspended = FALSE;
					}
					ImpCall(KeReleaseMutex, &SuspendMutex, FALSE);
				}
				//an interrupt could have fired while WaitForWriteToFinishAndSwapWriteBuffers was busy, pausing the process. If that happened, then the next KeWaitForSingleObject will exit instantly due to it being signaled 
			}
			else
				LOG_DEBUG("Unexpected wait result\n");

		}
	}

	NTSTATUS ultimap2_flushBuffers()
	{
		if (!UltimapActive)
			return STATUS_UNSUCCESSFUL;

		LOG_DEBUG("ultimap2_flushBuffers\n");

		ImpCall(KeWaitForSingleObject, &SuspendMutex, Executive, KernelMode, FALSE, NULL);
		if (CurrentTarget)
		{
			if (!isSuspended)
			{
				PsSuspendProcess(CurrentTarget);
				isSuspended = TRUE;
			}
		}
		ImpCall(KeReleaseMutex, &SuspendMutex, FALSE);

		flushallbuffers = TRUE;

		LOG_DEBUG("wait1\n");
		WaitForWriteToFinishAndSwapWriteBuffers(FALSE); //write the last saved buffer

		LOG_DEBUG("wait2\n");
		WaitForWriteToFinishAndSwapWriteBuffers(FALSE); //write the current buffer

		flushallbuffers = FALSE;
		LOG_DEBUG("after wait\n");
		ImpCall(KeWaitForSingleObject, &SuspendMutex, Executive, KernelMode, FALSE, NULL);
		if (CurrentTarget)
		{
			if (isSuspended)
			{
				PsResumeProcess(CurrentTarget);
				isSuspended = FALSE;
			}
		}
		ImpCall(KeReleaseMutex, &SuspendMutex, FALSE);

		LOG_DEBUG("ultimap2_flushBuffers exit\n");
		return STATUS_SUCCESS;
	}

	void RTIT_DPC_Handler(__in struct _KDPC* Dpc, __in_opt PVOID DeferredContext, __in_opt PVOID SystemArgument1, __in_opt PVOID SystemArgument2)
	{
		//Signal the bufferWriterThread
		ImpCall(KeSetEvent, &SuspendEvent, 0, FALSE);
		ImpCall(KeSetEvent, &FlushData, 0, FALSE);
	}

	void PMI(__in struct _KINTERRUPT* Interrupt, __in PVOID ServiceContext)
	{
		//check if caused by me, if so defer to dpc
		LOG_DEBUG("PMI\n");
		//__try
		{
			if ((KernelIntrin__readmsr(IA32_PERF_GLOBAL_STATUS) >> 55) & 1)
			{
				UINT64 Status = KernelIntrin__readmsr(IA32_RTIT_STATUS);

				LOG_DEBUG("PMI: caused by me\n");
				KernelIntrin__writemsr(IA32_PERF_GLOBAL_OVF_CTRL, (UINT64)1 << 55); //clear ToPA full status

				if ((KernelIntrin__readmsr(IA32_PERF_GLOBAL_STATUS) >> 55) & 1)
				{
					LOG_DEBUG("PMI: Failed to clear the status\n");
				}

				LOG_DEBUG("PMI: IA32_RTIT_OUTPUT_MASK_PTRS=%p\n", KernelIntrin__readmsr(IA32_RTIT_OUTPUT_MASK_PTRS));
				LOG_DEBUG("PMI: IA32_RTIT_STATUS=%p\n", Status);

				if ((Status >> 5) & 1) //Stopped
					LOG_DEBUG("PMI %d: Not all data recorded (AT THE PMI!)\n", KeGetCurrentProcessorNumber());


				LOG_DEBUG("PMI: IA32_RTIT_OUTPUT_MASK_PTRS %p\n", KernelIntrin__readmsr(IA32_RTIT_OUTPUT_MASK_PTRS));

				PInfo[KeGetCurrentProcessorNumber()]->Interrupted = TRUE;

				ImpCall(KeInsertQueueDpc, &RTID_DPC, NULL, NULL);

				//clear apic state

				VT_Apic::apic_clearPerfmon();
			}
			else
			{
				LOG_DEBUG("Unexpected PMI\n");
			}
		}
// 		__except (0)
// 		{
// 			LOG_DEBUG("PMI exception");
// 		}

	}

    void *pperfmon_hook2 = NULL;


	void ultimap2_disable_dpc(struct _KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		LOG_DEBUG("ultimap2_disable_dpc for cpu %d\n", KeGetCurrentProcessorNumber());

		//__try
		{
			if (DeferredContext) //only pause
			{
				RTIT_CTL ctl;
				LOG_DEBUG("temp disable\n");
				ctl.Value = KernelIntrin__readmsr(IA32_RTIT_CTL);
				ctl.Bits.TraceEn = 0;
				KernelIntrin__writemsr(IA32_RTIT_CTL, ctl.Value);
			}
			else
			{
				LOG_DEBUG("%d: disable all\n", KeGetCurrentProcessorNumber());


				KernelIntrin__writemsr(IA32_RTIT_CTL, 0);
				KernelIntrin__writemsr(IA32_RTIT_STATUS, 0);
				KernelIntrin__writemsr(IA32_RTIT_CR3_MATCH, 0);
				KernelIntrin__writemsr(IA32_RTIT_OUTPUT_BASE, 0);
				KernelIntrin__writemsr(IA32_RTIT_OUTPUT_MASK_PTRS, 0);
			}
		}
// 		__except (1)
// 		{
// 			LOG_DEBUG("ultimap2_disable_dpc exception\n");
// 		}
	}

	void ultimap2_setup_dpc(struct _KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		RTIT_CTL ctl;
		RTIT_STATUS s;
		int i = -1;

		//__try
		{
			ctl.Value = KernelIntrin__readmsr(IA32_RTIT_CTL);
		}
// 		__except (1)
// 		{
// 			LOG_DEBUG("ultimap2_setup_dpc: IA32_RTIT_CTL in unreadable\n");
// 			return;
// 		}

		ctl.Bits.TraceEn = 1;

		if (LogKernelMode)
			ctl.Bits.OS = 1;
		else
			ctl.Bits.OS = 0;

		if (LogUserMode)
			ctl.Bits.USER = 1;
		else
			ctl.Bits.USER = 0;

		if (CurrentCR3)
			ctl.Bits.CR3Filter = 1;
		else
			ctl.Bits.CR3Filter = 0;

		ctl.Bits.ToPA = 1;
		ctl.Bits.TSCEn = 0;
		ctl.Bits.DisRETC = 0;
		ctl.Bits.BranchEn = 1;

		if (PInfo == NULL)
			return;

		if (PInfo[KeGetCurrentProcessorNumber()]->ToPABuffer == NULL)
		{
			LOG_DEBUG("ToPA for cpu %d not setup\n", KeGetCurrentProcessorNumber());
			return;
		}

		//__try
		{
			int cpunr = KeGetCurrentProcessorNumber();
			i = 0;

			PInfo[cpunr]->CurrentOutputBase = ImpCall(MmGetPhysicalAddress,PInfo[cpunr]->ToPAHeader).QuadPart;

			KernelIntrin__writemsr(IA32_RTIT_OUTPUT_BASE, PInfo[cpunr]->CurrentOutputBase);
			i = 1;
			KernelIntrin__writemsr(IA32_RTIT_OUTPUT_MASK_PTRS, 0);
			i = 2;


			//__try
			{
				KernelIntrin__writemsr(IA32_RTIT_CR3_MATCH, CurrentCR3);
			}
// 			__except (1)
// 			{
// 				CurrentCR3 = CurrentCR3 & 0xfffffffffffff000ULL;
// 				LOG_DEBUG("Failed to set the actual CR3. Using a sanitized CR3: %llx\n", CurrentCR3);
// 			}

			i = 3;

			//ranges
			if (Ultimap2Ranges && Ultimap2RangeCount)
			{

				for (i = 0; i < Ultimap2RangeCount; i++)
				{
					ULONG msr_start = IA32_RTIT_ADDR0_A + (2 * i);
					ULONG msr_stop = IA32_RTIT_ADDR0_B + (2 * i);
					UINT64 bit = 32 + (i * 4);

					LOG_DEBUG("Range %d: (%p -> %p)\n", i, (PVOID)(UINT_PTR)(Ultimap2Ranges[i].StartAddress), (PVOID)(UINT_PTR)(Ultimap2Ranges[i].EndAddress));
					LOG_DEBUG("Writing range %d to msr %x and %x\n", i, msr_start, msr_stop);
					KernelIntrin__writemsr(msr_start, Ultimap2Ranges[i].StartAddress);
					KernelIntrin__writemsr(msr_stop, Ultimap2Ranges[i].EndAddress);

					LOG_DEBUG("bit=%d\n", bit);
					LOG_DEBUG("Value before=%llx\n", ctl.Value);
					if (Ultimap2Ranges[i].IsStopAddress)
						ctl.Value |= (UINT64)2ULL << bit; //TraceStop This stops all tracing on this cpu. Doesn't get reactivated
					else
						ctl.Value |= (UINT64)1ULL << bit; //FilterEn //not supported in the latest windows build

					LOG_DEBUG("Value after=%llx\n", ctl.Value);
				}
			}
			i = 4;

			KernelIntrin__writemsr(IA32_RTIT_STATUS, 0);
			i = 5;
			//if (KeGetCurrentProcessorNumber() == 0)
			KernelIntrin__writemsr(IA32_RTIT_CTL, ctl.Value);
			i = 6;

			s.Value = KernelIntrin__readmsr(IA32_RTIT_STATUS);
			if (s.Bits.Error)
			{
				LOG_DEBUG("Setup for cpu %d failed\n", KeGetCurrentProcessorNumber());
			}
			else
			{
				LOG_DEBUG("Setup for cpu %d succesful\n", KeGetCurrentProcessorNumber());
			}
		}
// 		__except (1)
// 		{
// 			LOG_DEBUG("Error in ultimap2_setup_dpc.  i=%d\n", i);
// 			LOG_DEBUG("ctl.Value=%p\n", ctl.Value);
// 			LOG_DEBUG("CR3=%p\n", CurrentCR3);
// 			//LOG_DEBUG("OutputBase=%p\n", __readmsr(IA32_RTIT_OUTPUT_BASE));
// 		}
	}

	int GetToPAHeaderCount(ULONG _BufferSize)
	{
		return 1 + (_BufferSize / 4096) / 511;
	}

	int getToPAHeaderSize(ULONG _BufferSize)
	{
		//511 entries per ToPA header (4096*511=2093056 bytes per ToPA header)
		//BufferSize / 2093056 = Number of ToPA headers needed
		return GetToPAHeaderCount(_BufferSize) * 4096;
	}

	RTL_GENERIC_COMPARE_RESULTS NTAPI ToPACompare(__in struct _RTL_GENERIC_TABLE* Table, __in PToPA_LOOKUP FirstStruct, __in PToPA_LOOKUP SecondStruct)
	{
		//LOG_DEBUG("Comparing %p with %p", FirstStruct->PhysicalAddress, FirstStruct->PhysicalAddress);

		if (FirstStruct->PhysicalAddress == SecondStruct->PhysicalAddress)
			return GenericEqual;
		else
		{
			if (SecondStruct->PhysicalAddress < FirstStruct->PhysicalAddress)
				return GenericLessThan;
			else
				return GenericGreaterThan;
		}
	}

	PVOID NTAPI ToPAAlloc(__in struct _RTL_GENERIC_TABLE* Table, __in CLONG ByteSize)
	{
		return ImpCall(ExAllocatePool, NonPagedPool, ByteSize);
	}

	VOID NTAPI ToPADealloc(__in struct _RTL_GENERIC_TABLE* Table, __in __drv_freesMem(Mem) __post_invalid PVOID Buffer)
	{
		ImpCall(ExFreePool, Buffer);
	}

	void* setupToPA(PToPA_ENTRY* Header, PVOID* OutputBuffer, PMDL* BufferMDL, PRTL_GENERIC_TABLE* gt, ULONG _BufferSize, int NoPMI)
	{
		ToPA_LOOKUP tl;
		PToPA_ENTRY r;
		UINT_PTR Output, Stop;
		ULONG ToPAIndex = 0;
		int PABlockSize = 0;
		int BlockSize;


		PRTL_GENERIC_TABLE x;
		int i;

		if (singleToPASystem)
		{

			PHYSICAL_ADDRESS la, ha, boundary;
			ULONG newsize;

			BlockSize = _BufferSize; //yup, only 1 single entry	


			//get the closest possible
			if (BlockSize > 64 * 1024 * 1024)
			{
				PABlockSize = 15;
				BlockSize = 128 * 1024 * 1024;
			}
			else
				if (BlockSize > 32 * 1024 * 1024)
				{
					PABlockSize = 14;
					BlockSize = 64 * 1024 * 1024;
				}
				else
					if (BlockSize > 16 * 1024 * 1024)
					{
						PABlockSize = 13;
						BlockSize = 32 * 1024 * 1024;
					}
					else
						if (BlockSize > 8 * 1024 * 1024)
						{
							PABlockSize = 12;
							BlockSize = 16 * 1024 * 1024;
						}
						else
							if (BlockSize > 4 * 1024 * 1024)
							{
								PABlockSize = 11;
								BlockSize = 8 * 1024 * 1024;
							}
							else
								if (BlockSize > 2 * 1024 * 1024)
								{
									PABlockSize = 10;
									BlockSize = 4 * 1024 * 1024;
								}
								else
									if (BlockSize > 1 * 1024 * 1024)
									{
										PABlockSize = 9;
										BlockSize = 2 * 1024 * 1024;
									}
									else
										if (BlockSize > 512 * 1024)
										{
											PABlockSize = 8;
											BlockSize = 1 * 1024 * 1024;
										}
										else
											if (BlockSize > 256 * 1024)
											{
												PABlockSize = 7;
												BlockSize = 512 * 1024;
											}
											else
												if (BlockSize > 128 * 1024)
												{
													PABlockSize = 6;
													BlockSize = 256 * 1024;
												}
												else
													if (BlockSize > 64 * 1024)
													{
														PABlockSize = 5;
														BlockSize = 128 * 1024;
													}
													else
														if (BlockSize > 32 * 1024)
														{
															PABlockSize = 4;
															BlockSize = 64 * 1024;
														}
														else
															if (BlockSize > 16 * 1024)
															{
																PABlockSize = 3;
																BlockSize = 32 * 1024;
															}
															else
																if (BlockSize > 8 * 1024)
																{
																	PABlockSize = 2;
																	BlockSize = 16 * 1024;
																}
																else
																	if (BlockSize > 4 * 1024)
																	{
																		PABlockSize = 1;
																		BlockSize = 8 * 1024;
																	}
																	else
																	{
																		PABlockSize = 0;
																		BlockSize = 4096;
																	}

			//adjust the buffersize so it is dividable by the blocksize
			newsize = BlockSize;

			LOG_DEBUG("BufferSize=%x\n", _BufferSize);
			LOG_DEBUG("BlockSize=%x (PABlockSize=%d)\n", BlockSize, PABlockSize);
			LOG_DEBUG("newsize=%x\n", newsize);


			la.QuadPart = 0;
			ha.QuadPart = 0xFFFFFFFFFFFFFFFFULL;
			boundary.QuadPart = BlockSize;

			*OutputBuffer = ImpCall(MmAllocateContiguousMemorySpecifyCache, newsize, la, ha, boundary, MmCached);
			//*OutputBuffer=MmAllocateContiguousMemory(newsize, ha);

			LOG_DEBUG("Allocated OutputBuffer at %p", ImpCall(MmGetPhysicalAddress, *OutputBuffer).QuadPart);

			_BufferSize = newsize;

			if (*OutputBuffer == NULL)
			{
				LOG_DEBUG("setupToPA (Single ToPA System): Failure allocating output buffer\n");
				return NULL;
			}

			r = (PToPA_ENTRY)ImpCall(ExAllocatePool, NonPagedPool, 4096);
			if (r == NULL)
			{
				ImpCall(MmFreeContiguousMemory, *OutputBuffer);
				*OutputBuffer = NULL;
				LOG_DEBUG("setupToPA (Single ToPA System): Failure allocating header for buffer\n");
				return NULL;
			}

		}
		else
		{
			//Not a single ToPA system
			BlockSize = 4096;

			*OutputBuffer = ImpCall(ExAllocatePool, NonPagedPool, _BufferSize);
			if (*OutputBuffer == NULL)
			{
				LOG_DEBUG("setupToPA: Failure allocating output buffer\n");
				return NULL;
			}

			r = (PToPA_ENTRY)ImpCall(ExAllocatePool, NonPagedPool, getToPAHeaderSize(_BufferSize));
			if (r == NULL)
			{
				ImpCall(ExFreePool, *OutputBuffer);
				*OutputBuffer = NULL;
				LOG_DEBUG("setupToPA: Failure allocating header for buffer\n");
				return NULL;
			}
		}


		*Header = r;

		*gt = (PRTL_GENERIC_TABLE)(ImpCall(ExAllocatePool, NonPagedPool, sizeof(RTL_GENERIC_TABLE)));

		if (*gt == NULL)
		{
			LOG_DEBUG("Failure allocating table\n");
			if (singleToPASystem)
				ImpCall(MmFreeContiguousMemory,*OutputBuffer);
			else
				ImpCall(ExFreePool,*OutputBuffer);
			*OutputBuffer = NULL;

			ImpCall(ExFreePool,*Header);
			*Header = NULL;

			return NULL;
		}

		x = *gt;

		ImpCall(RtlInitializeGenericTable, x, (PRTL_GENERIC_COMPARE_ROUTINE)ToPACompare, ToPAAlloc, ToPADealloc, NULL);


		tl.index = 0;
		tl.PhysicalAddress = ImpCall(MmGetPhysicalAddress, &r[0]).QuadPart;
		ImpCall(RtlInsertElementGenericTable, x, &tl, sizeof(tl), NULL);

		Output = (UINT_PTR)*OutputBuffer;
		Stop = Output + _BufferSize;

		*BufferMDL = ImpCall(IoAllocateMdl, *OutputBuffer, _BufferSize, FALSE, FALSE, NULL);
		ImpCall(MmBuildMdlForNonPagedPool,*BufferMDL);

		if (singleToPASystem)
		{
			r[0].Value = (UINT64)ImpCall(MmGetPhysicalAddress,(PVOID)Output).QuadPart;
			r[0].Bits.Size = PABlockSize;
			if (NoPMI)
				r[0].Bits.INT = 0;
			else
				r[0].Bits.INT = 1;
			r[0].Bits.STOP = 1;

			r[1].Value = ImpCall(MmGetPhysicalAddress,&r[0]).QuadPart;
			r[1].Bits.END = 1;
		}
		else
		{
			while (Output < Stop)
			{
				//fill in the topa entries pointing to eachother


				if ((ToPAIndex + 1) % 512 == 0)
				{
					//point it to the next ToPA table
					r[ToPAIndex].Value = ImpCall(MmGetPhysicalAddress, &r[ToPAIndex + 1]).QuadPart;
					r[ToPAIndex].Bits.END = 1;

					tl.index = tl.index++;
					tl.PhysicalAddress = ImpCall(MmGetPhysicalAddress, &r[ToPAIndex + 1]).QuadPart;
					ImpCall(RtlInsertElementGenericTable, x, &tl, sizeof(tl), NULL);
				}
				else
				{
					r[ToPAIndex].Value = (UINT64)ImpCall(MmGetPhysicalAddress,(PVOID)Output).QuadPart;
					r[ToPAIndex].Bits.Size = 0;
					Output += 4096;
				}

				ToPAIndex++;
			}

			ToPAIndex--;
			r[ToPAIndex].Bits.STOP = 1;
			i = (ToPAIndex * 90) / 100; //90%

			if ((i == (int)ToPAIndex) && (i > 0)) //don't interrupt on the very last entry (if possible)
				i--;

			if ((i > 0) && ((i + 1) % 512 == 0))
				i--;


			LOG_DEBUG("Interrupt at index %d\n", i);

			if (NoPMI)
				r[i].Bits.INT = 0;
			else
				r[i].Bits.INT = 1; //Interrupt after filling this entry 


			//and every 2nd page after this.  (in case of a rare situation where resume is called right after suspend)

			if (ToPAIndex > 0)
			{
				while (i < (int)(ToPAIndex - 1))
				{
					if (((i + 1) % 512) && (NoPMI == 0))  //anything but 0
						r[i].Bits.INT = 1;

					i += 2;
				}
			}
		}

		return (void*)r;
	}

	NTSTATUS ultimap2_pause()
	{
		if (ultimapEnabled)
		{
			forEachCpu(ultimap2_disable_dpc, (PVOID)1, NULL, NULL, NULL);
			if (UltimapActive)
			{
				flushallbuffers = TRUE;
				WaitForWriteToFinishAndSwapWriteBuffers(FALSE); //write the last saved buffer
				WaitForWriteToFinishAndSwapWriteBuffers(FALSE); //write the current buffer
				flushallbuffers = FALSE;
			}
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS ultimap2_resume()
	{
		if ((ultimapEnabled) && (PInfo))
			forEachCpu(ultimap2_setup_dpc, NULL, NULL, NULL, NULL);

		return STATUS_SUCCESS;
	}

	

	void* clear = NULL;
	BOOL RegisteredProfilerInterruptHandler;
	void SetupUltimap2(UINT32 PID, UINT32 BufferSize, WCHAR* Path, int rangeCount, PURANGE Ranges, int NoPMI, int UserMode, int KernelMode)
	{
	    pperfmon_hook2 = (void*)PMI;
		//for each cpu setup tracing
		//add the PMI interupt
		int i;
		NTSTATUS r = STATUS_UNSUCCESSFUL;
		int cpuid_r[4];

		if (Path)
		{
			LOG_DEBUG("SetupUltimap2(%x, %x, %S, %d, %p,%d,%d,%d\n", PID, BufferSize, Path, rangeCount, Ranges, NoPMI, UserMode, KernelMode);
		}
		else
		{
			LOG_DEBUG("SetupUltimap2(%x, %x, %d, %p,%d,%d,%d\n", PID, BufferSize, rangeCount, Ranges, NoPMI, UserMode, KernelMode);
		}
		
		KernelIntrin__cpuidex(cpuid_r, 0x14, 0);

		if ((cpuid_r[2] & 2) == 0)
		{
			LOG_DEBUG("Single ToPA System\n");
			singleToPASystem = TRUE;
		}

		NoPMIMode = NoPMI;
		LogKernelMode = KernelMode;
		LogUserMode = UserMode;



		LOG_DEBUG("Path[0]=%d\n", Path[0]);

		SaveToFile = (Path[0] != 0);

		if (SaveToFile)
		{
			ImpCall(wcsncpy, OutputPath, Path, 199);
			OutputPath[199] = 0;
			LOG_DEBUG("Ultimap2: SaveToFile==TRUE:  OutputPath=%S\n", OutputPath);
		}
		else
		{
			LOG_DEBUG("Ultimap2: Runtime processing\n");
		}

		if (rangeCount)
		{
			if (Ultimap2Ranges)
			{
				ImpCall(ExFreePool, Ultimap2Ranges);
				Ultimap2Ranges = NULL;
			}

			Ultimap2Ranges = (PURANGE)ImpCall(ExAllocatePool, NonPagedPool, rangeCount * sizeof(URANGE));

			for (i = 0; i < rangeCount; i++)
				Ultimap2Ranges[i] = Ranges[i];

			Ultimap2RangeCount = rangeCount;

		}
		else
			Ultimap2RangeCount = 0;


		//get the EProcess and CR3 for this PID
		if (PID)
		{
			if (ImpCall(PsLookupProcessByProcessId, (PVOID)PID, &CurrentTarget) == STATUS_SUCCESS)
			{
				//todo add specific windows version checks and hardcode offsets/ or use scans
				if (getCR3() & 0xfff)
				{
					LOG_DEBUG("Split kernel/usermode pages\n");
					//uses supervisor/usermode pagemaps			
					CurrentCR3 = *(UINT64*)((UINT_PTR)CurrentTarget + 0x278);
					if ((CurrentCR3 & 0xfffffffffffff000ULL) == 0)
					{
						LOG_DEBUG("No usermode CR3\n");
						CurrentCR3 = *(UINT64*)((UINT_PTR)CurrentTarget + 0x28);
					}

					LOG_DEBUG("CurrentCR3=%llx\n", CurrentCR3);
				}
				else
				{
					::KAPC_STATE apc_state;
					RtlZeroMemory(&apc_state, sizeof(apc_state));
					//__try
					{
						ImpCall(KeStackAttachProcess, CurrentTarget, &apc_state);
						CurrentCR3 = getCR3();
						ImpCall(KeUnstackDetachProcess,&apc_state);
					}
// 					__except (1)
// 					{
// 						LOG_DEBUG("Failure getting CR3 for this process\n");
// 						return;
// 					}
				}
			}
			else
			{
				LOG_DEBUG("Failure getting the EProcess for pid %d\n", PID);
				return;
			}

			ImpCall(ObfDereferenceObject,CurrentTarget);
		}
		else
		{
			CurrentTarget = 0;
			CurrentCR3 = 0;
		}

		LOG_DEBUG("CurrentCR3=%llx\n", CurrentCR3);

		UNICODE_STRING uFuncName;

		if (PsSuspendProcess == NULL)
		{
			ImpCall(RtlInitUnicodeString, &uFuncName, L"PsSuspendProcess");
			PsSuspendProcess = (fnPsSuspendProcess)ImpCall(MmGetSystemRoutineAddress, &uFuncName);
		}

		if (PsResumeProcess == NULL)
		{
			ImpCall(RtlInitUnicodeString, &uFuncName, L"PsResumeProcess");
			PsResumeProcess = (fnPsResumeProcess)ImpCall(MmGetSystemRoutineAddress, &uFuncName);
		}

		if ((PsSuspendProcess == NULL) || (PsResumeProcess == NULL))
		{
			LOG_DEBUG("No Suspend/Resume support\n");
			return;
		}


		ImpCall(KeInitializeDpc, &RTID_DPC, RTIT_DPC_Handler, NULL);

		ImpCall(KeInitializeEvent, &FlushData, SynchronizationEvent, FALSE);
		ImpCall(KeInitializeEvent, &SuspendEvent, SynchronizationEvent, FALSE);
		ImpCall(KeInitializeMutex, &SuspendMutex, 0);


		Ultimap2CpuCount = ImpCall(KeQueryMaximumProcessorCount);

		PInfo = (PProcessorInfo*)ImpCall(ExAllocatePool, NonPagedPool, Ultimap2CpuCount * sizeof(PProcessorInfo));
		Ultimap2_DataReady = (PVOID*)ImpCall(ExAllocatePool, NonPagedPool, Ultimap2CpuCount * sizeof(PVOID));

		if (PInfo == NULL)
		{
			LOG_DEBUG("PInfo alloc failed\n");
			return;
		}

		if (Ultimap2_DataReady == NULL)
		{
			LOG_DEBUG("Ultimap2_DataReady alloc failed\n");
			return;
		}

		for (i = 0; i < Ultimap2CpuCount; i++)
		{
			PInfo[i] = (ProcessorInfo*)ImpCall(ExAllocatePool, NonPagedPool, sizeof(ProcessorInfo));
			RtlZeroMemory(PInfo[i], sizeof(ProcessorInfo));

			ImpCall(KeInitializeEvent, &PInfo[i]->InitiateSave, SynchronizationEvent, FALSE);
			ImpCall(KeInitializeEvent, &PInfo[i]->Buffer2ReadyForSwap, NotificationEvent, TRUE);

			setupToPA(&PInfo[i]->ToPAHeader, &PInfo[i]->ToPABuffer, &PInfo[i]->ToPABufferMDL, &PInfo[i]->ToPALookupTable, BufferSize, NoPMI);
			setupToPA(&PInfo[i]->ToPAHeader2, &PInfo[i]->ToPABuffer2, &PInfo[i]->ToPABuffer2MDL, &PInfo[i]->ToPALookupTable2, BufferSize, NoPMI);

			LOG_DEBUG("cpu %d:\n", i);
			LOG_DEBUG("ToPAHeader=%p ToPABuffer=%p Size=%x\n", PInfo[i]->ToPAHeader, PInfo[i]->ToPABuffer, BufferSize);
			LOG_DEBUG("ToPAHeader2=%p ToPABuffer2=%p Size=%x\n", PInfo[i]->ToPAHeader2, PInfo[i]->ToPABuffer2, BufferSize);


			ImpCall(KeInitializeEvent, &PInfo[i]->DataReady, SynchronizationEvent, FALSE);
			ImpCall(KeInitializeEvent, &PInfo[i]->DataProcessed, SynchronizationEvent, FALSE);

			ImpCall(KeInitializeEvent, &PInfo[i]->FileAccess, SynchronizationEvent, TRUE);

			Ultimap2_DataReady[i] = &PInfo[i]->DataReady;

			ImpCall(KeInitializeDpc, &PInfo[i]->OwnDPC, SwitchToPABuffer, NULL);
			ImpCall(KeSetTargetProcessorDpc, &PInfo[i]->OwnDPC, (CCHAR)i);
		}

		UltimapActive = TRUE;
		ultimapEnabled = TRUE;

		ImpCall(PsCreateSystemThread, &SuspendThreadHandle, 0, NULL, 0, NULL, suspendThread, NULL);
		ImpCall(PsCreateSystemThread, &Ultimap2Handle, 0, NULL, 0, NULL, bufferWriterThread, NULL);

		for (i = 0; i < Ultimap2CpuCount; i++)
			ImpCall(PsCreateSystemThread, &PInfo[i]->WriterThreadHandle, 0, NULL, 0, NULL, WriteThreadForSpecificCPU, (PVOID)i);

		if ((NoPMI == FALSE) && (RegisteredProfilerInterruptHandler == FALSE))
		{

			LOG_DEBUG("Registering PMI handler\n");
			

			pperfmon_hook2 = (void*)PMI;

			r = HalSetSystemInformation(HalProfileSourceInterruptHandler, sizeof(PVOID*), &pperfmon_hook2); //hook the perfmon interrupt
			if (r == STATUS_SUCCESS)
				RegisteredProfilerInterruptHandler = TRUE;

			LOG_DEBUG("HalSetSystemInformation returned %x\n", r);

			if (r != STATUS_SUCCESS)
			{
				LOG_DEBUG("Failure hooking the permon interrupt.  Ultimap2 will not be able to use interrupts until you reboot (This can happen when the perfmon interrupt is hooked more than once. It has no restore/undo hook)\n");
			}
		}

		forEachCpu(ultimap2_setup_dpc, NULL, NULL, NULL, NULL);
	}

	void UnregisterUltimapPMI()
	{
		NTSTATUS r;

		//LOG_DEBUG("UnregisterUltimapPMI()\n");
		if (RegisteredProfilerInterruptHandler)
		{
			pperfmon_hook2 = NULL;
			r = HalSetSystemInformation(HalProfileSourceInterruptHandler, sizeof(PVOID*), &pperfmon_hook2);
			LOG_DEBUG("1: HalSetSystemInformation to disable returned %x\n", r);

			if (r == STATUS_SUCCESS)
				return;

			r = HalSetSystemInformation(HalProfileSourceInterruptHandler, sizeof(PVOID*), &clear); //unhook the perfmon interrupt
			LOG_DEBUG("2: HalSetSystemInformation to disable returned %x\n", r);

			if (r == STATUS_SUCCESS)
				return;


			r = HalSetSystemInformation(HalProfileSourceInterruptHandler, sizeof(PVOID*), 0);
			LOG_DEBUG("3: HalSetSystemInformation to disable returned %x\n", r);

		}
		else
		{
			//LOG_DEBUG("UnregisterUltimapPMI() not needed\n");
		}
			
	}

	void DisableUltimap2(void)
	{
		int i;

		//LOG_DEBUG("-------------------->DisableUltimap2<------------------\n");

		if (!ultimapEnabled)
			return;

		//LOG_DEBUG("-------------------->DisableUltimap2:Stage 1<------------------\n");

		forEachCpuAsync(ultimap2_disable_dpc, NULL, NULL, NULL, NULL);


		UltimapActive = FALSE;

		if (SuspendThreadHandle)
		{
			LOG_DEBUG("Waiting for SuspendThreadHandle\n");
			ImpCall(KeSetEvent, &SuspendEvent, 0, FALSE);
			ImpCall(ZwWaitForSingleObject, SuspendThreadHandle, FALSE, NULL);
			ImpCall(ZwClose, SuspendThreadHandle);
			SuspendThreadHandle = NULL;
		}

		if (PInfo)
		{
			for (i = 0; i < Ultimap2CpuCount; i++)
			{
				ImpCall(KeSetEvent, &PInfo[i]->DataProcessed, 0, FALSE);
				ImpCall(KeSetEvent, &PInfo[i]->DataReady, 0, FALSE);
			}
		}

		if (Ultimap2Handle)
		{
			LOG_DEBUG("Waiting for Ultimap2Handle\n");
			ImpCall(KeSetEvent, &FlushData, 0, FALSE);
			ImpCall(ZwWaitForSingleObject, Ultimap2Handle, FALSE, NULL);
			ImpCall(ZwClose, Ultimap2Handle);
			Ultimap2Handle = NULL;
		}

		if (PInfo)
		{
			LOG_DEBUG("going to deal with the PInfo data\n");
			for (i = 0; i < Ultimap2CpuCount; i++)
			{
				if (PInfo[i])
				{
					PToPA_LOOKUP li;


					ImpCall(KeSetEvent, &PInfo[i]->Buffer2ReadyForSwap, 0, FALSE);
					ImpCall(KeSetEvent, &PInfo[i]->InitiateSave, 0, FALSE);


					LOG_DEBUG("Waiting for WriterThreadHandle[%d]\n", i);
					ImpCall(ZwWaitForSingleObject, PInfo[i]->WriterThreadHandle, FALSE, NULL);
					ImpCall(ZwClose, PInfo[i]->WriterThreadHandle);

					PInfo[i]->WriterThreadHandle = NULL;

					if (PInfo[i]->ToPABufferMDL)
					{
						ImpCall(IoFreeMdl, PInfo[i]->ToPABufferMDL);
						PInfo[i]->ToPABufferMDL = NULL;
					}

					if (PInfo[i]->ToPABuffer)
					{
						if (singleToPASystem)
							ImpCall(MmFreeContiguousMemory, PInfo[i]->ToPABuffer);
						else
							ImpCall(ExFreePool, PInfo[i]->ToPABuffer);
						PInfo[i]->ToPABuffer = NULL;
					}

					if (PInfo[i]->ToPABuffer2MDL)
					{
						ImpCall(IoFreeMdl, PInfo[i]->ToPABuffer2MDL);
						PInfo[i]->ToPABufferMDL = NULL;
					}

					if (PInfo[i]->ToPABuffer2)
					{
						if (singleToPASystem)
							ImpCall(MmFreeContiguousMemory, PInfo[i]->ToPABuffer2);
						else
							ImpCall(ExFreePool, PInfo[i]->ToPABuffer2);

						PInfo[i]->ToPABuffer2 = NULL;
					}

					if (PInfo[i]->ToPAHeader)
					{
						ImpCall(ExFreePool, PInfo[i]->ToPAHeader);
						PInfo[i]->ToPAHeader = NULL;
					}

					if (PInfo[i]->ToPAHeader2)
					{
						ImpCall(ExFreePool, PInfo[i]->ToPAHeader2);
						PInfo[i]->ToPAHeader2 = NULL;
					}

					while (li = (PToPA_LOOKUP)ImpCall(RtlGetElementGenericTable, PInfo[i]->ToPALookupTable, 0))
						ImpCall(RtlDeleteElementGenericTable, PInfo[i]->ToPALookupTable, li);

					ImpCall(ExFreePool, PInfo[i]->ToPALookupTable);
					PInfo[i]->ToPALookupTable = NULL;

					while (li = (PToPA_LOOKUP)ImpCall(RtlGetElementGenericTable, PInfo[i]->ToPALookupTable2, 0))
						ImpCall(RtlDeleteElementGenericTable, PInfo[i]->ToPALookupTable2, li);

					ImpCall(ExFreePool, PInfo[i]->ToPALookupTable2);
					PInfo[i]->ToPALookupTable2 = NULL;


					ImpCall(ExFreePool, PInfo[i]);
					PInfo[i] = NULL;
				}
			}

			ImpCall(ExFreePool, (PVOID)PInfo);
			ImpCall(ExFreePool, Ultimap2_DataReady);

			PInfo = NULL;

			LOG_DEBUG("Finished terminating ultimap2\n");
		}

		if (Ultimap2Ranges)
		{
			ImpCall(ExFreePool,Ultimap2Ranges);
			Ultimap2Ranges = NULL;

			Ultimap2RangeCount = 0;
		}

		LOG_DEBUG("-------------------->DisableUltimap2:Finish<------------------\n");
	}
}