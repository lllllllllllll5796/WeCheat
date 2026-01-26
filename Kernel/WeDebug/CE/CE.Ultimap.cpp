#include "WeVt.pch.h"
#include "CE.Ultimap.h"

extern volatile VT_Apic::PAPIC DBVM_APIC_BASE;

namespace Ultimap
{
	using namespace VT_InterruptHook;
	using namespace VT_Apic;
	using namespace DBKUtil;
	using namespace VTUltimap;
	using namespace VmxHelper;

	JUMPBACK PerfmonJumpBackLocation;

	PDS_AREA_MANAGEMENT DS_AREA[256]; //used to store the addresses. (reading the msr that holds the DS_AREA is impossible with dbvm active)
	int DS_AREA_SIZE;

	/*
	#ifdef AMD64
	volatile PAPIC DBVM_APIC_BASE=0; //(PAPIC)0xfffffffffffe0000;
	#else
	volatile PAPIC DBVM_APIC_BASE=0; //(PAPIC)0xfffe0000;
	#endif
	*/

	BOOL SaveToFile; //If set it will save the results to a file instead of sending a message to the usermode app that is watching the data
	HANDLE FileHandle;

	int MaxDataBlocks = 1;

	KSEMAPHORE DataBlockSemaphore; //governs how many events can be active at a time
	FAST_MUTEX DataBlockMutex; //when a thread passes the semaphore this is used to pick a DataBlock

	typedef struct
	{
		BOOL Available;
		PBTS Data;
		int DataSize;
		int CpuID;
		KEVENT DataReady;
	} _DataBlock;

	_DataBlock* DataBlock;
	PVOID* DataReadyPointerList;

	extern "C" int perfmon_interrupt_centry(void);

	/* use apic.* now
	#define MSR_IA32_APICBASE               0x0000001b

	void setup_APIC_BASE(void)
	{
		PHYSICAL_ADDRESS Physical_APIC_BASE;
		LOG_DEBUG("Fetching the APIC base\n");

		Physical_APIC_BASE.QuadPart=readMSR(MSR_IA32_APICBASE) & 0xFFFFFFFFFFFFF000ULL;


		LOG_DEBUG("Physical_APIC_BASE=%p\n", Physical_APIC_BASE.QuadPart);

		APIC_BASE = (PAPIC)MmMapIoSpace(Physical_APIC_BASE, sizeof(APIC), MmNonCached);


		LOG_DEBUG("APIC_BASE at %p\n", APIC_BASE);

	}

	void clean_APIC_BASE(void)
	{
		if (APIC_BASE)
			MmUnmapIoSpace((PVOID)APIC_BASE, sizeof(APIC));
	}*/
	
	void ultimap_flushBuffers_all(UINT_PTR param)
	{
		LOG_DEBUG("Calling Perfmon_Interrupt_Centry() manually\n");
		if (DS_AREA[cpunr()]) //don't call if ultimap has been disabled
		{
			perfmon_interrupt_centry();
			enableInterrupts(); //the handler disables it on exit so re-enable it
		}
	}

	void ultimap_flushBuffers(void)
	{
		//call this when the buffer of the current cpu needs to be flushed and handled
		int i;
		int count;

		LOG_DEBUG("ultimap_flushBuffers\n");

		//what it does:
		//for each cpu emulate a "buffer filled" event.
		//the handler then copies all the current data to a temporary buffer and signals the worker thread to deal with it. If there is no available worker thread it waits
		forEachCpuPassive(ultimap_flushBuffers_all, 0);

		LOG_DEBUG("ultimap_flushBuffers_all has returned\n");
		//it returned and all worker thread are currently working on this data (it only returns when it has send a worker to work)


		//now wait for all workers to finish
		//do this by aquiring all semaphore slots and waiting for them to return again
		//forEachCpuPassive(ultimap_flushBuffers_all,0);
		//LOG_DEBUG("ultimap_flushBuffers_all has returned a second time\n"); //this means that the previous blocks have been dealt with


		//actually... no, this is no guarantee. Now that the buffers are empty handling is so fast that while block 2,3,4,5 and 6 are still being handled block 1 can become available multiple times

	}

	NTSTATUS ultimap_continue(PULTIMAPDATAEVENT data)
		/*
		Called from usermode to signal that the data has been handled
		*/
	{
		LOG_DEBUG("ultimap_continue\n");
		ImpCall(MmUnmapLockedPages, (PVOID)(UINT_PTR)data->Address, (PMDL)(UINT_PTR)data->Mdl);
		ImpCall(IoFreeMdl, (PMDL)(UINT_PTR)data->Mdl);

		ImpCall(ExFreePool, (PVOID)(UINT_PTR)data->KernelAddress); //this memory is not needed anymore


		if (DataBlock)
			DataBlock[data->Block].Available = TRUE;


		ImpCall(KeReleaseSemaphore, &DataBlockSemaphore, 1, 1, FALSE); //Let the next block go through
		LOG_DEBUG("Released semaphore\n");
		return STATUS_SUCCESS;
	}

	NTSTATUS ultimap_waitForData(ULONG timeout, PULTIMAPDATAEVENT data)
		/*
		Called from usermode to wait for data
		*/
	{
		NTSTATUS r;
		LARGE_INTEGER wait;

		PKWAIT_BLOCK waitblock;

		if (DataBlock)
		{
			waitblock = (PKWAIT_BLOCK)ImpCall(ExAllocatePool, NonPagedPool, MaxDataBlocks * sizeof(KWAIT_BLOCK));


			wait.QuadPart = -10000LL * timeout;

			//Wait for the events in the list
			//If an event is triggered find out which one is triggered, then map that block into the usermode space and return the address and block
			//That block will be needed to continue

			if (timeout == 0xffffffff) //infinite wait
				r = ImpCall(KeWaitForMultipleObjects, MaxDataBlocks, DataReadyPointerList, WaitAny, UserRequest, UserMode, TRUE, NULL, waitblock);
			else
				r = ImpCall(KeWaitForMultipleObjects, MaxDataBlocks, DataReadyPointerList, WaitAny, UserRequest, UserMode, TRUE, &wait, waitblock);

			ImpCall(ExFreePool, waitblock);

			data->Block = r - STATUS_WAIT_0;

			if (data->Block <= MaxDataBlocks)
			{
				//Map this block to usermode


				ImpCall(ExAcquireFastMutex, &DataBlockMutex);
				if (DataBlock)
				{
					data->KernelAddress = (UINT64)DataBlock[data->Block].Data;
					data->Mdl = (UINT64)ImpCall(IoAllocateMdl, DataBlock[data->Block].Data, DataBlock[data->Block].DataSize, FALSE, FALSE, NULL);
					if (data->Mdl)
					{
						ImpCall(MmBuildMdlForNonPagedPool,(PMDL)(UINT_PTR)data->Mdl);

						data->Address = (UINT_PTR)ImpCall(MmMapLockedPagesSpecifyCache, (PMDL)(UINT_PTR)data->Mdl, UserMode, MmCached, NULL, FALSE, NormalPagePriority);
						if (data->Address)
						{
							data->Size = DataBlock[data->Block].DataSize;
							data->CpuID = DataBlock[data->Block].CpuID;
							r = STATUS_SUCCESS;
						}
						else
							r = STATUS_UNSUCCESSFUL;
					}
					else
						r = STATUS_UNSUCCESSFUL;
				}
				else
					r = STATUS_UNSUCCESSFUL;

				ImpCall(ExReleaseFastMutex, &DataBlockMutex);

				return r;
			}
			else
				return STATUS_UNSUCCESSFUL;

		}
		else
			return STATUS_UNSUCCESSFUL;



	}

	/*
	void apic_clearPerfmon()
	{
		APIC_BASE->LVT_Performance_Monitor.a = APIC_BASE->LVT_Performance_Monitor.a & 0xff;
		APIC_BASE->EOI.a = 0;
	}
	*/
	void ultimap_cleanstate()
	{
		apic_clearPerfmon();
	}

	extern "C" int perfmon_interrupt_centry(void)
	{

		KIRQL old = PASSIVE_LEVEL;
		int changedIRQL = 0;

		void* temp;
		int causedbyme = (DS_AREA[cpunr()]->BTS_IndexBaseAddress >= DS_AREA[cpunr()]->BTS_InterruptThresholdAddress);
		UINT_PTR blocksize;

		LOG_DEBUG("perfmon_interrupt_centry\n", cpunr());


		if (causedbyme)
			ultimap_cleanstate();

		blocksize = (UINT_PTR)(DS_AREA[cpunr()]->BTS_IndexBaseAddress - DS_AREA[cpunr()]->BTS_BufferBaseAddress);

		{
			if (ImpCall(KeGetCurrentIrql) < DISPATCH_LEVEL)
			{
				//When called by the pre-emptive caller
				changedIRQL = 1;
				old = ImpCall(KeRaiseIrqlToDpcLevel);
			}


			LOG_DEBUG("Entry cpunr=%d\n", cpunr());
			LOG_DEBUG("Entry threadid=%d\n", ImpCall(PsGetCurrentThreadId));


			temp = ImpCall(ExAllocatePool, NonPagedPool, blocksize);
			if (temp)
			{
				RtlCopyMemory(temp, (PVOID*)(UINT_PTR)DS_AREA[cpunr()]->BTS_BufferBaseAddress, blocksize);

				LOG_DEBUG("temp=%p\n", temp);


				DS_AREA[cpunr()]->BTS_IndexBaseAddress = DS_AREA[cpunr()]->BTS_BufferBaseAddress; //don't reset on alloc error	
			}
			else
			{
				LOG_DEBUG("ExAllocatePool has failed\n");
				ImpCall(KeLowerIrql, old);
				disableInterrupts();
				return causedbyme;
			}

			if (changedIRQL)
				ImpCall(KeLowerIrql,old);
			//should be passive mode, taskswitches and cpu switches will happen now (When this returns, I may not be on the same interrupt as I was when I started)


			if (SaveToFile)
			{
				IO_STATUS_BLOCK iosb;
				NTSTATUS r;

				//Instead of sending the data to a usermode app it was chosen to store the data to a file for later usage
				LOG_DEBUG("Writing buffer to disk\n");
				r = ImpCall(ZwWriteFile, FileHandle, NULL, NULL, NULL, &iosb, temp, (ULONG)blocksize, NULL, NULL);
				LOG_DEBUG("Done Writing. Result=%x\n", r);
			}
			else
			{
				LOG_DEBUG("Waiting till there is a block free\n");
				//When all workers are busy do not continue
				if ((DataBlock) && (ImpCall(KeWaitForSingleObject, &DataBlockSemaphore, Executive, KernelMode, FALSE, NULL) == STATUS_SUCCESS))
				{
					int currentblock;
					int i;

					//Enter a critical section and choose a block
					LOG_DEBUG("Acquired semaphore. Now picking a usable datablock\n");


					ImpCall(ExAcquireFastMutex, &DataBlockMutex);
					LOG_DEBUG("Acquired mutex. Looking for a Datablock that can be used\n");

					if (DataBlock)
					{
						currentblock = -1;
						for (i = 0; i < MaxDataBlocks; i++)
						{
							if (DataBlock[i].Available) //look for a block that is set as available
							{
								currentblock = i;
								DataBlock[currentblock].Available = FALSE; //not available anymore
								break;
							}
						}
					}
					else currentblock = -1;



					if (currentblock >= 0)
					{
						LOG_DEBUG("Using datablock %d\n", currentblock);
						DataBlock[currentblock].Data = (PBTS)temp;
						DataBlock[currentblock].DataSize = (int)blocksize;
						DataBlock[currentblock].CpuID = cpunr();

						LOG_DEBUG("Calling KeSetEvent/KeWaitForSingleObject\n");
						ImpCall(KeSetEvent, &DataBlock[currentblock].DataReady, 1, FALSE); //Trigger a worker thread to start working					
					}
					ImpCall(ExReleaseFastMutex, &DataBlockMutex);
					//LOG_DEBUG("Released mutex\n");
				}
				else
				{
					LOG_DEBUG("if ((DataBlock) && (KeWaitForSingleObject(&DataBlockSemaphore, Executive, KernelMode, FALSE, NULL) == STATUS_SUCCESS)) failed\n");
				}
			}

			//and return to the caller process
			disableInterrupts();
			return causedbyme;
		}
	}



#ifdef AMD64
	extern "C" void perfmon_interrupt();
#else
	_declspec(naked) void perfmon_interrupt(void)
	{
		__asm {
			cld

			push ebp
			mov ebp, esp

			//store state
			pushad
			xor eax, eax
			mov ax, ds
			push eax

			mov ax, es
			push eax

			mov ax, fs
			push eax

			mov ax, gs
			push eax

			mov ax, 0x23 //0x10 should work too, but even windows itself is using 0x23
			mov ds, ax
			mov es, ax
			mov gs, ax
			mov ax, 0x30
			mov fs, ax

			call perfmon_interrupt_centry
			cmp eax, 1	//set flag


			//restore state
			pop gs
			pop fs
			pop es
			pop ds
			popad

			pop ebp

			je skip_original_perfmon

			jmp far[perfmonJumpBackLocation]

			skip_original_perfmon:
			// commented out: I don't think a APIC interrupt has an errorcode....  add esp,4 //undo errorcode push
			iretd
		}
	}
#endif

	VOID ultimap_pause_dpc(IN struct _KDPC* Dpc, IN PVOID DeferredContext, IN PVOID SystemArgumen1, IN PVOID SystemArgument2)
	{
		vmx_ultimap_pause();
	}

	void ultimap_pause(void)
	{
		forEachCpu(ultimap_pause_dpc, NULL, NULL, NULL, NULL);
	}

	VOID ultimap_resume_dpc(IN struct _KDPC* Dpc, IN PVOID DeferredContext, IN PVOID SystemArgumen1, IN PVOID SystemArgument2)
	{
		vmx_ultimap_resume();
	}

	void ultimap_resume(void)
	{
		forEachCpu(ultimap_resume_dpc, NULL, NULL, NULL, NULL);
	}

	VOID ultimap_disable_dpc(IN struct _KDPC* Dpc, IN PVOID DeferredContext, IN PVOID SystemArgumen1, IN PVOID SystemArgument2)
	{
		int i;
		//LOG_DEBUG("ultimap_disable_dpc()\n");

		if (vmxusable)
		{
			i = vmx_ultimap_disable();

			if (DS_AREA[cpunr()])
			{
				ImpCall(ExFreePool, DS_AREA[cpunr()]);
				DS_AREA[cpunr()] = NULL;
			}
		}
	}

	void ultimap_disable(void)
	{
		void* clear = NULL;

		if (DataBlock)
		{
			int i;

			forEachCpu(ultimap_disable_dpc, NULL, NULL, NULL, NULL);

			if (SaveToFile && FileHandle)
			{
				ImpCall(ZwClose, FileHandle);
				FileHandle = NULL;
			}

			//all logging should have stopped now

			//Trigger all events waking up each thread that was waiting for the events

			ImpCall(ExAcquireFastMutex, &DataBlockMutex);

			for (i = 0; i < MaxDataBlocks; i++)
				ImpCall(KeSetEvent, &DataBlock[i].DataReady, 0, FALSE);

			ImpCall(ExFreePool,DataBlock);
			DataBlock = NULL;

			if (DataReadyPointerList)
			{
				ImpCall(ExFreePool,DataReadyPointerList);
				DataReadyPointerList = NULL;
			}
			ImpCall(ExReleaseFastMutex,&DataBlockMutex);

			HalSetSystemInformation(HalProfileSourceInterruptHandler, sizeof(PVOID*), &clear); //unhook the perfmon interrupt
		}
	}

	VOID ultimap_setup_dpc(IN struct _KDPC* Dpc, IN PVOID  DeferredContext, IN PVOID  SystemArgument1, IN PVOID  SystemArgument2)
		/*
		initializes ultimap. If the DS_AREA_SIZE is bigger than 0 it will allocate the required region (the usual option, but if not used it could be a LBR only thing)
		Call this for each processor
		*/
	{
		struct XXX
		{
			UINT64 cr3;
			UINT64 dbgctl_msr;
			int DS_AREA_SIZE;
		} *params;

		params = (XXX*)DeferredContext;

		DS_AREA_SIZE = params->DS_AREA_SIZE;
		if (DS_AREA_SIZE == 0)
		{
			LOG_DEBUG("DS_AREA_SIZE==0\n");
			return;
		}


		LOG_DEBUG("ultimap(%I64x, %I64x, %d)", (UINT64)params->cr3, (UINT64)params->dbgctl_msr, params->DS_AREA_SIZE);
		DS_AREA[cpunr()] = NULL;

		if (params->DS_AREA_SIZE)
		{
			DS_AREA[cpunr()] = (PDS_AREA_MANAGEMENT)ImpCall(ExAllocatePool, NonPagedPool, params->DS_AREA_SIZE);

			if (DS_AREA[cpunr()] == NULL)
			{
				LOG_DEBUG("ExAllocatePool failed\n");
				return;
			}

			RtlZeroMemory(DS_AREA[cpunr()], params->DS_AREA_SIZE);

			LOG_DEBUG("DS_AREA[%d]=%p\n", cpunr(), DS_AREA[cpunr()]);

			//Initialize the DS_AREA 

			DS_AREA[cpunr()]->BTS_BufferBaseAddress = (QWORD)(UINT_PTR)DS_AREA[cpunr()] + sizeof(DS_AREA_MANAGEMENT);
			DS_AREA[cpunr()]->BTS_BufferBaseAddress += sizeof(BTS);

			DS_AREA[cpunr()]->BTS_BufferBaseAddress -= DS_AREA[cpunr()]->BTS_BufferBaseAddress % sizeof(BTS);

			DS_AREA[cpunr()]->BTS_IndexBaseAddress = DS_AREA[cpunr()]->BTS_BufferBaseAddress;
			DS_AREA[cpunr()]->BTS_AbsoluteMaxAddress = (QWORD)(UINT_PTR)DS_AREA[cpunr()] + params->DS_AREA_SIZE - sizeof(BTS);
			DS_AREA[cpunr()]->BTS_AbsoluteMaxAddress -= DS_AREA[cpunr()]->BTS_AbsoluteMaxAddress % sizeof(BTS);
			DS_AREA[cpunr()]->BTS_AbsoluteMaxAddress++;

			DS_AREA[cpunr()]->BTS_InterruptThresholdAddress = (DS_AREA[cpunr()]->BTS_AbsoluteMaxAddress - 1) - 4 * sizeof(BTS);
		}


		if (params->dbgctl_msr & (1 << 8))
		{
			//hook the perfmon interrupt. First get the interrupt assigned (usually 0xfe, but let's be sure and read it from the apic)		

			int perfmonIVT = (DBVM_APIC_BASE->LVT_Performance_Monitor.a) & 0xff;

			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.a=%x\n", DBVM_APIC_BASE->LVT_Performance_Monitor.a);
			if (perfmonIVT == 0) //if not setup at all then set it up now
				perfmonIVT = 0xfe;

			DBVM_APIC_BASE->LVT_Performance_Monitor.a = perfmonIVT; //clear mask flag if it was set

			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.a=%x\n", DBVM_APIC_BASE->LVT_Performance_Monitor.a);


			/*

			if (inthook_HookInterrupt((unsigned char)perfmonIVT, getCS(), (ULONG_PTR)perfmon_interrupt, &perfmonJumpBackLocation))
				LOG_DEBUG("Interrupt hooked\n");
			else
				LOG_DEBUG("Failed to hook interrupt\n");
			*/

		}

		//and finally activate the mapping
		if (vmxusable)
		{
			vmx_ultimap((UINT_PTR)params->cr3, params->dbgctl_msr, DS_AREA[cpunr()]);
		}
		else
		{
			LOG_DEBUG("vmxusable is false. So no ultimap for you!!!\n");
		}
	}

	void ultimapapc(PKAPC Apc, PKNORMAL_ROUTINE NormalRoutine, PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		DBKUtil::EFLAGS e = getEflags();
		LOG_DEBUG("ultimapapc call for cpu %d ( IF=%d IRQL=%d)\n", KeGetCurrentProcessorNumber(), e.IF, ImpCall(KeGetCurrentIrql));
		LOG_DEBUG("SystemArgument1=%x\n", *(PULONG)SystemArgument1);
		LOG_DEBUG("tid=%x\n", ImpCall(PsGetCurrentThreadId));
		LOG_DEBUG("Apc=%p\n", Apc);
	}

	void ultimapapcnormal(PVOID arg1, PVOID arg2, PVOID arg3)
	{
		DBKUtil::EFLAGS e = getEflags();
		LOG_DEBUG("ultimapapcnormal call for cpu %d ( IF=%d IRQL=%d)\n", KeGetCurrentProcessorNumber(), e.IF, ImpCall(KeGetCurrentIrql));
		LOG_DEBUG("tid=%x\n", ImpCall(PsGetCurrentThreadId));

		ultimap_flushBuffers();

		return;
	}

	KAPC      kApc[128];
	volatile LONG apcnr = 0;

	void perfmon_hook(__in struct _KINTERRUPT* Interrupt, __in PVOID ServiceContext)
	{
		int i = InterlockedIncrement(&apcnr) % 128;

		DBKUtil::EFLAGS e = getEflags();
		LOG_DEBUG("permon_hook call for cpu %d ( IF=%d IRQL=%d)\n", KeGetCurrentProcessorNumber(), e.IF, ImpCall(KeGetCurrentIrql));

		LOG_DEBUG("kApc=%p\n", &kApc);


		//switch buffer pointers

		//call DPC for ultimap for this cpu


	    //todo: if this is buggy use a dpc instead to create the apc.  (slower)
		ImpCall(KeInitializeApc, &kApc[i],
			(PKTHREAD)PsGetCurrentThread(),
			(wdk::KAPC_ENVIRONMENT)0,
			(PKKERNEL_ROUTINE)ultimapapc,
			NULL,
			(PKNORMAL_ROUTINE)ultimapapcnormal,
			KernelMode,
			0
		);

		ImpCall(KeInsertQueueApc,&kApc[i], NULL, NULL, 0);

		LOG_DEBUG("after KeInsertQueueApc\n");

		//perfmon_interrupt_centry();
		ultimap_cleanstate();

		LOG_DEBUG("permon_return\n");
	}

	NTSTATUS ultimap(UINT64 cr3, UINT64 dbgctl_msr, int _DS_AREA_SIZE, BOOL savetofile, WCHAR* filename, int handlerCount)
	{
		struct
		{
			UINT64 cr3;
			UINT64 dbgctl_msr;
			int DS_AREA_SIZE;
		} params;
		int i;

		if (handlerCount > 64)
			return STATUS_UNSUCCESSFUL;

		static void* pperfmon_hook = (void*)perfmon_hook;

		//create file
		SaveToFile = savetofile;

		if (SaveToFile)
		{
			UNICODE_STRING usFile;
			OBJECT_ATTRIBUTES oaFile;
			IO_STATUS_BLOCK iosb;
			NTSTATUS r;

			ImpCall(RtlInitUnicodeString, &usFile, filename);
			InitializeObjectAttributes(&oaFile, &usFile, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

			LOG_DEBUG("Creating file %S", usFile.Buffer);

			FileHandle = 0;
			r = ImpCall(ZwCreateFile,&FileHandle, SYNCHRONIZE | FILE_READ_DATA | FILE_APPEND_DATA | GENERIC_ALL, &oaFile, &iosb, 0, FILE_ATTRIBUTE_NORMAL, 0, FILE_SUPERSEDE, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
			LOG_DEBUG("ZwCreateFile=%x\n", r);
		}

		MaxDataBlocks = handlerCount;
		ImpCall(KeInitializeSemaphore, &DataBlockSemaphore, MaxDataBlocks, MaxDataBlocks);
		KernelCommon::Utils::NewExInitializeFastMutex(&DataBlockMutex);

		//Datablock inits

		DataBlock = (_DataBlock*)ImpCall(ExAllocatePool, NonPagedPool, sizeof(_DataBlock) * MaxDataBlocks);
		DataReadyPointerList = (PVOID*)ImpCall(ExAllocatePool, NonPagedPool, sizeof(PVOID) * MaxDataBlocks);

		RtlZeroMemory(DataBlock, sizeof(_DataBlock) * MaxDataBlocks);
		RtlZeroMemory(DataReadyPointerList, sizeof(PVOID) * MaxDataBlocks);


		if ((DataBlock) && (DataReadyPointerList))
		{
			NTSTATUS r;

			for (i = 0; i < MaxDataBlocks; i++)
			{
				//DataBlock[i]->
				DataBlock[i].Data = NULL;

				ImpCall(KeInitializeEvent, &DataBlock[i].DataReady, SynchronizationEvent, FALSE);

				DataBlock[i].Available = TRUE;

				DataReadyPointerList[i] = &DataBlock[i].DataReady;
			}


			params.cr3 = cr3;
			params.dbgctl_msr = dbgctl_msr;
			params.DS_AREA_SIZE = _DS_AREA_SIZE;

			r = HalSetSystemInformation(HalProfileSourceInterruptHandler, sizeof(PVOID*), &pperfmon_hook); //hook the perfmon interrupt

			LOG_DEBUG("HalSetSystemInformation returned %x\n", r);

			forEachCpu(ultimap_setup_dpc, &params, NULL, NULL, NULL);
			return STATUS_SUCCESS;
		}
		else
		{
			LOG_DEBUG("Failure allocating DataBlock and DataReadyPointerList\n");
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
	}
}