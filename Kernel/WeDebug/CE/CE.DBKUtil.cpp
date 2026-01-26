#include "WeVt.pch.h"
#include "CE.ProcessList.h"
#include "CE.DeepKernel.h"

#pragma warning(disable:4334)

namespace DBKUtil
{
	BOOL loadedbydbvm;
	int PTESize;
	UINT_PTR PAGE_SIZE_LARGE;
	UINT_PTR MAX_PDE_POS;
	UINT_PTR MAX_PTE_POS;

	int Cpu_Stepping;
	int Cpu_Model;
	int Cpu_FamilyID;
	int Cpu_Type;
	int Cpu_Ext_ModelID;
	int Cpu_Ext_FamilyID;

	int KernelCodeStepping;
	int KernelWritesIgnoreWP;

#if (NTDDI_VERSION >= NTDDI_VISTA)
	PVOID DRMHandle = NULL;
	PEPROCESS DRMProcess = NULL;
	PEPROCESS DRMProcess2 = NULL;
#endif

	CPU_VENDOR Cpu_Vendor;

	typedef NTSTATUS(*PSRCTNR)(__in PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);
	PSRCTNR PsRemoveCreateThreadNotifyRoutine2;

	typedef NTSTATUS(*PSRLINR)(__in PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
	PSRLINR PsRemoveLoadImageNotifyRoutine2;

	BOOL VmxHardSuported()
	{
		CPUID data = { 0 };
		IA32_FEATURE_CONTROL_MSR Control = { 0 };

		// VMX bit   这里动手脚
		KernelIntrin__cpuid((int*)&data, 1);
		if ((data.ecx & (1 << 5)) == 0)
		{
			LOG_DEBUG("VMX bit failed\n");
			return FALSE;
		}

		DWORD a = data.eax;

		Cpu_Stepping = a & 0xf;
		Cpu_Model = (a >> 4) & 0xf;
		Cpu_FamilyID = (a >> 8) & 0xf;
		Cpu_Type = (a >> 12) & 0x3;
		Cpu_Ext_ModelID = (a >> 16) & 0xf;
		Cpu_Ext_FamilyID = (a >> 20) & 0xff;

		Cpu_Model = Cpu_Model + (Cpu_Ext_ModelID << 4);
		Cpu_FamilyID = Cpu_FamilyID + (Cpu_Ext_FamilyID << 4);

		Cpu_Model = Cpu_Model + (Cpu_Ext_ModelID << 4);
		Cpu_FamilyID = Cpu_FamilyID + (Cpu_Ext_FamilyID << 4);

		//LOG_DEBUG("cpu_model:0x%X cpu_familyID:0x%X\r\n", cpu_model, cpu_familyID);

		Control.All = KernelIntrin__readmsr(0x3A);

		// BIOS lock check
		if (Control.Fields.Lock == 0)
		{
			Control.Fields.Lock = TRUE;
			Control.Fields.EnableVmxon = TRUE;
			KernelIntrin__writemsr(0x3A, Control.All);
		}
		else if (Control.Fields.EnableVmxon == false)
		{
			LOG_DEBUG("Control.Fields.EnableVmxon == false\n");
			return FALSE;
		}

		return TRUE;
	}

	CPU_VENDOR UtilCPUVendor()
	{
		CPUID data = { 0 };
		char vendor[0x20] = { 0 };
		KernelIntrin__cpuid((int*)&data, 0);

		*(int*)(vendor) = data.ebx;
		*(int*)(vendor + 4) = data.edx;
		*(int*)(vendor + 8) = data.ecx;
		//LOG_DEBUG("vendor=%s\n", vendor);
		if (memcmp(vendor, oxorany("GenuineIntel"), 12) == 0)  //0x756e6547
		{
			return CPU_Intel;
		}
			
		if (memcmp(vendor, oxorany("AuthenticAMD"), 12) == 0)  //0x68747541
			return CPU_AMD;

		return CPU_Other;
	}

	BOOL HvmIsSuported()
	{
		Cpu_Vendor = UtilCPUVendor();
		if (Cpu_Vendor == CPU_Intel)
		{
			return VmxHardSuported();
		}
		else
		{
			LOG_DEBUG("Not an Intel CPU\r\n");
			if (Cpu_Vendor == CPU_AMD)
			{
				LOG_DEBUG("This is an AMD\n");
				return TRUE;
			}
		}
		return FALSE;
	}

	void DBK_Init()
	{
		WORD this_cs, this_ss, this_ds, this_es, this_fs, this_gs;
		ULONG cr4reg;
		UNICODE_STRING uFuncName;

		//criticalSection csTest;  //没用到
		//HANDLE Ultimap2Handle;   //没用到

		KernelCodeStepping = 0;
		KernelWritesIgnoreWP = 0;

		//没用到
		this_cs = getCS();
		this_ss = getSS();
		this_ds = getDS();
		this_es = getES();
		this_fs = getFS();
		this_gs = getGS();


		//Processlist init
#ifndef CETC
		ProcessList::ProcessEventCount = 0;
		ImpCall(ExInitializeResourceLite,&(ProcessList::ProcesslistR));
#endif

		ProcessList::CreateProcessNotifyRoutineEnabled = FALSE;

		//threadlist init
		ProcessList::ThreadEventCount = 0;

		//processlist init
		ProcessList::processlist = NULL;
		//--------------------------
#ifndef AMD64
		//determine if PAE is used
		cr4reg = (ULONG)getCR4();

		if ((cr4reg & 0x20) == 0x20)
		{
			PTESize = 8; //pae
			PAGE_SIZE_LARGE = 0x200000;
			MAX_PDE_POS = 0xC0604000;
			MAX_PTE_POS = 0xC07FFFF8;
		}
		else
		{
			PTESize = 4;
			PAGE_SIZE_LARGE = 0x400000;
			MAX_PDE_POS = 0xC0301000;
			MAX_PTE_POS = 0xC03FFFFC;
		}
#else
		PTESize = 8; //pae
		PAGE_SIZE_LARGE = 0x200000;
		//base was 0xfffff68000000000ULL

		//to 
		MAX_PTE_POS = 0xFFFFF6FFFFFFFFF8ULL; // base + 0x7FFFFFFFF8
		MAX_PDE_POS = 0xFFFFF6FB7FFFFFF8ULL; // base + 0x7B7FFFFFF8
#endif

#ifdef CETC
		LOG_DEBUG("Going to initialice CETC\n");
		InitializeCETC();
#endif
		//LOG_DEBUG("[+] Initializing debugger\n");
		VT_Debugger::debugger_initialize();

// 		int r[4];
// 		DWORD a;
// 
// 		__cpuid(r, 0);
// 
// 		//LOG_DEBUG("[+] cpuid.0: r[1]=%x\n", r[1]);
// 
// 		if (r[1] == 0x756e6547) //GenuineIntel
// 		{
// 			__cpuid(r, 1);
// 
// 			a = r[0];
// 
// 			Cpu_Stepping = a & 0xf;
// 			Cpu_Model = (a >> 4) & 0xf;
// 			Cpu_FamilyID = (a >> 8) & 0xf;
// 			Cpu_Type = (a >> 12) & 0x3;
// 			Cpu_Ext_ModelID = (a >> 16) & 0xf;
// 			Cpu_Ext_FamilyID = (a >> 20) & 0xff;
// 
// 			Cpu_Model = Cpu_Model + (Cpu_Ext_ModelID << 4);
// 			Cpu_FamilyID = Cpu_FamilyID + (Cpu_Ext_FamilyID << 4);
// 
// 			VmxHelper::vmx_Init_DoVmcall(1);
// 			VT_Apic::setup_APIC_BASE();      //for ultimap
// 		}
// 		else
// 		{
// 			LOG_DEBUG("[?] Not an intel cpu\n");
// 			if (r[1] == 0x68747541)
// 			{
// 				LOG_DEBUG("[+] This is an AMD\n");
// 				VmxHelper::vmx_Init_DoVmcall(0);
// 			}
// 		}

		ImpCall(RtlInitUnicodeString, &uFuncName, L"PsSuspendProcess");
		Ultimap2::PsSuspendProcess = (fnPsSuspendProcess)ImpCall(MmGetSystemRoutineAddress, &uFuncName);
		
		ImpCall(RtlInitUnicodeString, &uFuncName, L"PsResumeProcess");
		Ultimap2::PsResumeProcess = (fnPsResumeProcess)ImpCall(MmGetSystemRoutineAddress, &uFuncName);
	}

	void DBK_UnInit()
	{
		VmxOffLoad::CleanupDBVM();

 		if (!VT_Debugger::debugger_stopDebugging())
 		{
 			LOG_DEBUG("Can not unload the driver because of debugger\n");
 		}
 
 		VT_Debugger::debugger_shutdown();

		Ultimap::ultimap_disable();
		Ultimap2::DisableUltimap2();
		Ultimap2::UnregisterUltimapPMI();

		VT_Apic::clean_APIC_BASE();

		NoExceptions::NoExceptions_Cleanup();

		if ((ProcessList::CreateProcessNotifyRoutineEnabled) || (DeepKernel::ImageNotifyRoutineLoaded))
		{
			PVOID x;
			UNICODE_STRING temp;

			ImpCall(RtlInitUnicodeString, &temp, L"PsRemoveCreateThreadNotifyRoutine");
			PsRemoveCreateThreadNotifyRoutine2 = (PSRCTNR)MmGetSystemRoutineAddress(&temp);

			RtlInitUnicodeString(&temp, L"PsRemoveCreateThreadNotifyRoutine");
			PsRemoveLoadImageNotifyRoutine2 = (PSRLINR)MmGetSystemRoutineAddress(&temp);

			RtlInitUnicodeString(&temp, L"ObOpenObjectByName");
			x = MmGetSystemRoutineAddress(&temp);

			LOG_DEBUG("ObOpenObjectByName=%p\n", x);


			if ((PsRemoveCreateThreadNotifyRoutine2) && (PsRemoveLoadImageNotifyRoutine2))
			{
				LOG_DEBUG("Stopping processwatch\n");

				if (ProcessList::CreateProcessNotifyRoutineEnabled)
				{
					LOG_DEBUG("Removing process watch");
#if (NTDDI_VERSION >= NTDDI_VISTASP1)
					PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)(ProcessList::CreateProcessNotifyRoutineEx), TRUE);
#else
					PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE);
#endif


					LOG_DEBUG("Removing thread watch");
					PsRemoveCreateThreadNotifyRoutine2(ProcessList::CreateThreadNotifyRoutine);
				}

				if (DeepKernel::ImageNotifyRoutineLoaded)
					PsRemoveLoadImageNotifyRoutine2(DeepKernel::LoadImageNotifyRoutine);
			}
			else return;  //leave now!!!!!
		}

#ifdef CETC
	#ifndef CETC_RELEASE
		UnloadCETC(); //not possible in the final build
	#endif
#endif

		ProcessList::CleanProcessList();

		ImpCall(ExDeleteResourceLite, &(ProcessList::ProcesslistR));

		RtlZeroMemory(&(ProcessList::ProcesslistR), sizeof(ProcessList::ProcesslistR));

#if (NTDDI_VERSION >= NTDDI_VISTA)
		if (DRMHandle)
		{
			LOG_DEBUG("Unregistering DRM handle\n");
			ObUnRegisterCallbacks(DRMHandle);
			DRMHandle = NULL;
		}
#endif
	}

	void forEachCpuPassive(PF f, UINT_PTR param)
		/*
		calls a specific function for each cpu that runs in passive mode
		*/
	{
		CCHAR cpunr;
		KAFFINITY cpus, original;
		ULONG cpucount;

	
		//KeIpiGenericCall is not present in xp

		//count cpus first KeQueryActiveProcessorCount is not present in xp)
		cpucount = 0;
		cpus = ImpCall(KeQueryActiveProcessors);
		original = cpus;
		while (cpus)
		{
			if (cpus % 2)
				cpucount++;

			cpus = cpus / 2;
		}

		cpus = ImpCall(KeQueryActiveProcessors);
		cpunr = 0;
		while (cpus)
		{
			if (cpus % 2)
			{
				//bit is set
#if (NTDDI_VERSION >= NTDDI_VISTA)
				KAFFINITY oldaffinity;
#endif
				KAFFINITY newaffinity;



				//LOG_DEBUG("Calling passive function for cpunr %d\n", cpunr);
				//set affinity

				newaffinity = (KAFFINITY)(1 << cpunr);

#if (NTDDI_VERSION >= NTDDI_VISTA)
				oldaffinity = ImpCall(KeSetSystemAffinityThreadEx,newaffinity);
#else
				//XP and earlier (this routine is not called often, only when the user asks explicitly
				{
					LARGE_INTEGER delay;
					delay.QuadPart = -50; //short wait just to be sure... (the docs do not say that a switch happens imeadiatly for the no Ex version)

					KeSetSystemAffinityThread(newaffinity);
					KeDelayExecutionThread(UserMode, FALSE, &delay);
				}
#endif

				//call function
				f(param);

#if (NTDDI_VERSION >= NTDDI_VISTA)
				ImpCall(KeRevertToUserAffinityThreadEx,oldaffinity);
#endif
			}

			cpus = cpus / 2;
			cpunr++;
		}

#if (NTDDI_VERSION < NTDDI_VISTA)
		KeSetSystemAffinityThread(original);
#endif
	}

	void forOneCpu(CCHAR cpunr, PKDEFERRED_ROUTINE dpcfunction, PVOID DeferredContext, PVOID  SystemArgument1, PVOID  SystemArgument2, OPTIONAL PPREDPC_CALLBACK preDPCCallback)
	{
		PKDPC dpc = NULL;

		if (preDPCCallback) //if preDPCCallback is set call it which may change the system arguments
			preDPCCallback(cpunr, dpcfunction, DeferredContext, &SystemArgument1, &SystemArgument2);
	

		dpc = (PKDPC)ImpCall(ExAllocatePool, NonPagedPool, sizeof(KDPC));
		ImpCall(KeInitializeDpc, dpc, dpcfunction, DeferredContext);
		ImpCall(KeSetTargetProcessorDpc, dpc, cpunr);
		ImpCall(KeInsertQueueDpc,dpc, SystemArgument1, SystemArgument2);
		ImpCall(KeFlushQueuedDpcs);

		ImpCall(ExFreePool,dpc);
	}

	void forEachCpu(PKDEFERRED_ROUTINE dpcfunction, PVOID DeferredContext, PVOID  SystemArgument1, PVOID  SystemArgument2, OPTIONAL PPREDPC_CALLBACK preDPCCallback)
	/*
	calls a specified dpcfunction for each cpu on the system
	*/
	{
		//LOG_DEBUG("[1] ForEachCpu-Enter\r\n");
		CCHAR cpunr;
		KAFFINITY cpus;
		ULONG cpucount;
		PKDPC dpc;
		int dpcnr;


		//KeIpiGenericCall is not present in xp

		//count cpus first KeQueryActiveProcessorCount is not present in xp)
		cpucount = 0;
		cpus = ImpCall(KeQueryActiveProcessors);
		while (cpus)
		{
			if (cpus % 2)
				cpucount++;

			cpus = cpus / 2;
		}

		dpc = (PKDPC)ImpCall(ExAllocatePool, NonPagedPool, sizeof(KDPC) * cpucount);

		

		cpus = ImpCall(KeQueryActiveProcessors);
		cpunr = 0;
		dpcnr = 0;
		while (cpus)
		{
			if (cpus % 2)
			{
				//bit is set

				//LOG_DEBUG("Calling dpc routine for cpunr %d (dpc=%p)\n", cpunr, &dpc[dpcnr]);

				if (preDPCCallback)
					preDPCCallback(cpunr, dpcfunction, DeferredContext, &SystemArgument1, &SystemArgument2);

				ImpCall(KeInitializeDpc, &dpc[dpcnr], dpcfunction, DeferredContext);
				ImpCall(KeSetTargetProcessorDpc, &dpc[dpcnr], cpunr);
				ImpCall(KeInsertQueueDpc, &dpc[dpcnr], SystemArgument1, SystemArgument2);
				ImpCall(KeFlushQueuedDpcs);
				dpcnr++;
			}

			cpus = cpus / 2;
			cpunr++;
		}


		ImpCall(ExFreePool,dpc);
		//LOG_DEBUG("ForEachCpu-Leave\r\n");
	}


	void forEachCpuAsync(PKDEFERRED_ROUTINE dpcfunction, PVOID DeferredContext, PVOID  SystemArgument1, PVOID  SystemArgument2, OPTIONAL PPREDPC_CALLBACK preDPCCallback)
	/*
	calls a specified dpcfunction for each cpu on the system
	*/
	{
		CCHAR cpunr;
		KAFFINITY cpus;
		ULONG cpucount;
		PKDPC dpc;
		int dpcnr;



		//KeIpiGenericCall is not present in xp

		//count cpus first KeQueryActiveProcessorCount is not present in xp)
		cpucount = 0;
		cpus = ImpCall(KeQueryActiveProcessors);
		while (cpus)
		{
			if (cpus % 2)
				cpucount++;

			cpus = cpus / 2;
		}

		dpc = (PKDPC)ImpCall(ExAllocatePool,NonPagedPool, sizeof(KDPC) * cpucount);

		cpus = ImpCall(KeQueryActiveProcessors);
		cpunr = 0;
		dpcnr = 0;
		while (cpus)
		{
			if (cpus % 2)
			{
				//bit is set

				//LOG_DEBUG("Calling dpc routine for cpunr %d\n", cpunr);
				if (preDPCCallback) //if preDPCCallback is set call it which may change the system arguments
					preDPCCallback(cpunr, dpcfunction, DeferredContext, &SystemArgument1, &SystemArgument2);

				ImpCall(KeInitializeDpc,&dpc[dpcnr], dpcfunction, DeferredContext);
				ImpCall(KeSetTargetProcessorDpc,&dpc[dpcnr], cpunr);
				ImpCall(KeInsertQueueDpc,&dpc[dpcnr], SystemArgument1, SystemArgument2);
				dpcnr++;
			}

			cpus = cpus / 2;
			cpunr++;
		}

		ImpCall(KeFlushQueuedDpcs);


		ImpCall(ExFreePool,dpc);
	}

	//own critical section implementation for use when the os is pretty much useless (dbvm tech)
	void spinlock(volatile LONG* lockvar)
	{
		while (1)
		{

			//it was 0, let's see if we can set it to 1
			//race who can set it to 1:
			if (_InterlockedExchange((volatile LONG*)lockvar, 1) == 0)
				return; //lock aquired, else continue loop

			_mm_pause();

		}

	}

	void csEnter(PcriticalSection CS)
	{
		EFLAGS oldstate = getEflags();

		if ((CS->locked) && (CS->cpunr == cpunr()))
		{
			//already locked but the locker is this cpu, so allow, just increase lockcount
			CS->lockcount++;
			return;
		}

		disableInterrupts(); //disable interrupts to prevent taskswitch in same cpu

		spinlock(&(CS->locked)); //sets CS->locked to 1

		//here so the lock is aquired and locked is 1
		CS->lockcount = 1;
		CS->cpunr = cpunr();
		CS->oldIFstate = oldstate.IF;
	}

	void csLeave(PcriticalSection CS)
	{
		if ((CS->locked) && (CS->cpunr == cpunr()))
		{
			CS->lockcount--;
			if (CS->lockcount == 0)
			{
				//unlock    
				if (CS->oldIFstate)
					enableInterrupts();

				CS->cpunr = -1; //set to an cpunr
				CS->locked = 0;
			}
		}

	}


	int getCpuCount(void)
	{
		KAFFINITY CpuAffinity = ImpCall(KeQueryActiveProcessors);
		int ulNumberOfActiveCpu = 0;

		// 		for (ULONG i = 0; i < 32; ++i)
		// 		{
		// 			if ((CpuAffinity >> i) & 1)
		// 			{
		// 				++ulNumberOfActiveCpu;
		// 			}	
		// 		}

		while (CpuAffinity > 0)
		{
			if (CpuAffinity % 2)
				ulNumberOfActiveCpu++;

			CpuAffinity = CpuAffinity / 2;
		}

		return ulNumberOfActiveCpu;
	}


	int IsPrefix(unsigned char b)
	{
		switch (b)
		{
		case 0x26:
		case 0x2e:
		case 0x36:
		case 0x3e:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0xf0: //lock
		case 0xf2: //repne
		case 0xf3: //rep
			return 1;

		default:
			return 0;

		}
	}

	UINT64 getDR7(void)
	{
		return __readdr(7);
	}

	int cpunr(void)
	{
		DWORD x[4];
		KernelIntrin__cpuid((int*)&x[0], 1);

		return (x[1] >> 24) + 1;

	}

	EFLAGS getEflags(void)
	{
		UINT64 x = KernelIntrin__getcallerseflags();
		PEFLAGS y = (PEFLAGS)&x;
		return *y;
	}

	UINT64 readMSR(DWORD msr)
	{
		return KernelIntrin__readmsr(msr);
	}

	void setCR0(UINT64 newcr0)
	{
		__writecr0(newcr0);
	}

	UINT64 getCR0(void)
	{
		return __readcr0();
	}

	UINT64 getCR2(void)
	{
		return __readcr2();
	}

	void setCR3(UINT64 newCR3)
	{
		__writecr3((UINT_PTR)newCR3);
	}

	UINT64 getCR3(void)
	{
		return __readcr3();
	}



	void setCR4(UINT64 newcr4)
	{
		__writecr4(newcr4);
	}

	UINT64 getCR4(void)
	{
		return __readcr4();
	}

	void GetIDT(PIDT pIdt)
	{
		KernelIntrin__sidt(pIdt);
	}

	void enableInterrupts(void)
	{
#ifdef AMD64
		_enable();
#else
		__asm {sti};
#endif
	}

	void disableInterrupts(void)
	{
#ifdef AMD64
		_disable();
#else
		__asm {cli};
#endif
	}

	UINT64 getTSC(void)
	{
		return __rdtsc();
	}

#ifndef AMD64
	//function declarations that can be done inline without needing an .asm file
	_declspec(naked) WORD getSS(void)
	{
		__asm
		{
			mov ax, ss
			ret
		}
	}

	_declspec(naked) WORD getCS(void)
	{
		__asm
		{
			mov ax, cs
			ret
		}
	}

	_declspec(naked) WORD getDS(void)
	{
		__asm
		{
			mov ax, ds
			ret
		}
	}

	_declspec(naked) WORD getES(void)
	{
		__asm
		{
			mov ax, es
			ret
		}
	}

	_declspec(naked) WORD getFS(void)
	{
		__asm
		{
			mov ax, fs
			ret
		}
	}

	_declspec(naked) WORD getGS(void)
	{
		__asm
		{
			mov ax, gs
			ret
		}
	}


	_declspec(naked) ULONG getRSP(void) //...
	{
		__asm
		{
			mov eax, esp
			add eax, 4 //don't add this call
			ret
		}
	}

	_declspec(naked) ULONG getRBP(void)
	{
		__asm
		{
			mov eax, ebp
			ret
		}
	}

	_declspec(naked) ULONG getRAX(void)
	{
		__asm
		{
			mov eax, eax
			ret
		}
	}
	_declspec(naked) ULONG getRBX(void)
	{
		__asm
		{
			mov eax, ebx
			ret
		}
	}
	_declspec(naked) ULONG getRCX(void)
	{
		__asm
		{
			mov eax, ecx
			ret
		}
	}
	_declspec(naked) ULONG getRDX(void)
	{
		__asm
		{
			mov eax, edx
			ret
		}
	}
	_declspec(naked) ULONG getRSI(void)
	{
		__asm
		{
			mov eax, esi
			ret
		}
	}
	_declspec(naked) ULONG getRDI(void)
	{
		__asm
		{
			mov eax, edi
			ret
		}
	}

	_declspec(naked) unsigned short getTR(void)
	{
		__asm {
			STR AX
			ret
		}
	}


	void GetGDT(PGDT pGdt)
	{
		__asm
		{
			MOV EAX, [pGdt]
			SGDT[EAX]
		}
	}

	_declspec(naked)WORD getLDT()
	{
		__asm
		{
			SLDT ax
			ret
		}
	}

#endif

	NTSTATUS UtilForEachProcessor(NTSTATUS(*callback_routine)(void*), void* context)
	{
		const auto number_of_processors =
			ImpCall(KeQueryActiveProcessorCountEx,ALL_PROCESSOR_GROUPS);
		for (ULONG processor_index = 0; processor_index < number_of_processors;
			processor_index++)
		{
			PROCESSOR_NUMBER processor_number = {};
			auto status = ImpCall(KeGetProcessorNumberFromIndex, processor_index, &processor_number);
			if (!NT_SUCCESS(status)) 
			{
				return status;
			}

			// Switch the current processor
			GROUP_AFFINITY affinity = {};
			affinity.Group = processor_number.Group;
			affinity.Mask = 1ull << processor_number.Number;
			GROUP_AFFINITY previous_affinity = {};
			ImpCall(KeSetSystemGroupAffinityThread, &affinity, &previous_affinity);

			// Execute callback
			status = callback_routine(context);

			ImpCall(KeRevertToUserGroupAffinityThread,&previous_affinity);
			if (!NT_SUCCESS(status)) 
			{
				return status;
			}
		}
		return STATUS_SUCCESS;
	}

	NTSTATUS UtilForEachProcessorDpc(PKDEFERRED_ROUTINE deferred_routine, void* context) 
	{
		const auto number_of_processors = ImpCall(KeQueryActiveProcessorCountEx,ALL_PROCESSOR_GROUPS);
		for (ULONG processor_index = 0; processor_index < number_of_processors;
			processor_index++) 
		{
			PROCESSOR_NUMBER processor_number = {};
			auto status = ImpCall(KeGetProcessorNumberFromIndex,processor_index, &processor_number);
			if (!NT_SUCCESS(status)) 
			{
				return status;
			}

			const auto dpc = reinterpret_cast<PRKDPC>(ImpCall(ExAllocatePool, NonPagedPool, sizeof(KDPC)));
			if (!dpc) 
			{
				return STATUS_MEMORY_NOT_ALLOCATED;
			}
			ImpCall(KeInitializeDpc, dpc, deferred_routine, context);
			ImpCall(KeSetImportanceDpc, dpc, HighImportance);
			status = ImpCall(KeSetTargetProcessorDpcEx, dpc, &processor_number);
			if (!NT_SUCCESS(status)) 
			{
				ImpCall(ExFreePool,dpc);
				return status;
			}
			ImpCall(KeInsertQueueDpc,dpc, nullptr, nullptr);
		}
		return STATUS_SUCCESS;
	}

	int GenerateCRC(unsigned char* ptr, int size)
	{
		int i = 0;
		unsigned int cval = 0;
		while (i < size)
		{
			cval = (ptr[i] + cval) % 65536;
			i++;
		}

		return cval;
	}

	void Nothing(PVOID arg1, PVOID arg2, PVOID arg3)
	{
		return;
	}

	void KApcRoutine2(PKAPC Apc, PKNORMAL_ROUTINE NormalRoutine, PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		QWORD iswow64;
		ImpCall(ExFreePool, Apc);

		LOG_DEBUG("My second kernelmode apc!!!!\n");
		LOG_DEBUG("SystemArgument1=%x\n", *(PULONG)SystemArgument1);
		LOG_DEBUG("SystemArgument2=%x\n", *(PULONG)SystemArgument2);

		if (ImpCall(ZwQueryInformationProcess, ZwCurrentProcess(), ProcessWow64Information, &iswow64, sizeof(iswow64), NULL) == STATUS_SUCCESS)
		{
#if (NTDDI_VERSION >= NTDDI_VISTA)	
			if (iswow64)
			{
				LOG_DEBUG("WOW64 apc\n");
				ImpCall(PsWrapApcWow64Thread, (PVOID*)NormalContext, (PVOID*)NormalRoutine);
			}
#endif
		}
	}

	void KApcRoutine(PKAPC Apc, PKNORMAL_ROUTINE NormalRoutine, PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		//kernelmode apc, always gets executed
		PKAPC      kApc;
		LARGE_INTEGER Timeout;

		kApc = (PKAPC)ImpCall(ExAllocatePool, NonPagedPool, sizeof(KAPC));

		ImpCall(ExFreePool, Apc);

		LOG_DEBUG("My kernelmode apc!!!!(irql=%d)\n", ImpCall(KeGetCurrentIrql));

		LOG_DEBUG("NormalRoutine=%p\n", *(PUINT_PTR)NormalRoutine);
		LOG_DEBUG("NormalContext=%p\n", *(PUINT_PTR)NormalContext);
		LOG_DEBUG("SystemArgument1=%p\n", *(PUINT_PTR)SystemArgument1);
		LOG_DEBUG("SystemArgument2=%p\n", *(PUINT_PTR)SystemArgument2);

		ImpCall(KeInitializeApc, kApc,
			(PKTHREAD)ImpCall(PsGetCurrentThread),
			wdk::KAPC_ENVIRONMENT::OriginalApcEnvironment,
			(PKKERNEL_ROUTINE)KApcRoutine2,
			NULL,
			(PKNORMAL_ROUTINE)(*(PUINT_PTR)SystemArgument1),
			UserMode,
			(PVOID) * (PUINT_PTR)NormalContext
		);

		ImpCall(KeInsertQueueApc, kApc, (PVOID) * (PUINT_PTR)SystemArgument1, (PVOID)(*(PUINT_PTR)SystemArgument2), 0);

		//wait in usermode (so interruptable by a usermode apc)
		Timeout.QuadPart = 0;
		ImpCall(KeDelayExecutionThread, UserMode, TRUE, &Timeout);

		return;
	}

	void CreateRemoteAPC(ULONG ThreadId, PVOID AddressToExecute)
	{
		PKTHREAD   kThread;
		PKAPC      kApc;

		kApc = (PKAPC)ImpCall(ExAllocatePool, NonPagedPool, sizeof(KAPC));
		kThread = (PKTHREAD)MemScan::getPEThread(ThreadId);

		LOG_DEBUG("KThread=%p\n", kThread);
		LOG_DEBUG("AddressToExecute=%p\n", AddressToExecute);

		ImpCall(KeInitializeApc, kApc,
			kThread,
			wdk::KAPC_ENVIRONMENT::OriginalApcEnvironment,
			(PKKERNEL_ROUTINE)KApcRoutine,
			NULL,
			(PKNORMAL_ROUTINE)Nothing,
			KernelMode,
			0
		);

		ImpCall(KeInsertQueueApc, kApc, AddressToExecute, AddressToExecute, 0);
	}

}
