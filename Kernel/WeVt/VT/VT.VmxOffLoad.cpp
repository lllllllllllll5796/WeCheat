#include "WeVt.pch.h"
#include "VT.vmdisk.h"
#include "zlib/zlib.h"

extern "C"
{
#pragma pack(2) 
	struct
	{
		WORD limit;
		UINT_PTR base;
	} NewGDTDescriptor;
#pragma pack()

#pragma pack(1) 
	typedef struct _INITVARS
	{
		UINT64 loadedOS; //physical address of the loadedOS section
		UINT64 vmmstart; //physical address of virtual address 00400000 (obsoletish...)
		UINT64 pagedirlvl4; //Virtual address of the pml4 table (the virtual memory after this until the next 4MB alignment is free to use)
		UINT64 nextstack; //The virtual address of the stack for the next CPU (vmloader only sets it up when 0)
		UINT64 extramemory; //Physical address of some extra initial memory (physically contiguous)
		UINT64 extramemorysize; //the number of pages that extramemory spans
		UINT64 contiguousmemory; //Physical address of some extra initial memory (physically contiguous)
		UINT64 contiguousmemorysize; //the number of pages that extramemory spans
	} INITVARS, * PINITVARS;

	typedef struct _OriginalState
	{
		//ok, everything uint64, I hate these incompatibilities with alignment between gcc and ms c
		UINT64		cpucount;
		UINT64		originalEFER;
		UINT64		originalLME;
		UINT64		idtbase;
		UINT64		idtlimit;
		UINT64		gdtbase;
		UINT64		gdtlimit;
		UINT64		cr0;
		UINT64		cr2;
		UINT64		cr3;
		UINT64		cr4;
		UINT64		dr7;
		UINT64		rip;

		UINT64		rax;
		UINT64		rbx;
		UINT64		rcx;
		UINT64		rdx;
		UINT64		rsi;
		UINT64		rdi;
		UINT64		rbp;
		UINT64		rsp;
		UINT64		r8;
		UINT64		r9;
		UINT64		r10;
		UINT64		r11;
		UINT64		r12;
		UINT64		r13;
		UINT64		r14;
		UINT64		r15;

		UINT64		rflags;
		UINT64		cs;
		UINT64		ss;
		UINT64		ds;
		UINT64		es;
		UINT64		fs;
		UINT64		gs;
		UINT64		tr;
		UINT64		ldt;

		UINT64		cs_AccessRights;
		UINT64		ss_AccessRights;
		UINT64		ds_AccessRights;
		UINT64		es_AccessRights;
		UINT64		fs_AccessRights;
		UINT64		gs_AccessRights;

		UINT64		cs_Limit;
		UINT64		ss_Limit;
		UINT64		ds_Limit;
		UINT64		es_Limit;
		UINT64		fs_Limit;
		UINT64		gs_Limit;

		UINT64		fsbase;
		UINT64		gsbase;

	} OriginalState, * POriginalState;
#pragma pack() 

	POriginalState Originalstate; //one of the reasons why multiple cpu's don't start at exactly the same time

	UINT_PTR NewGDTDescriptorVA;
	UINT_PTR DBVMPML4PA;
	UINT_PTR OriginalstatePA;
	UINT_PTR TemporaryPagingSetupPA;
	UINT_PTR EnterVMM2PA;
	unsigned char* EnterVMM2;
	UINT_PTR vmmPA;
}


namespace VT_VmxOffLoad
{
	using namespace VT_Util;
	using namespace VT_InterruptHook;
	using namespace VmxHelper;
	unsigned char* vmm;

	BOOL InitializedVmm = FALSE;   //无模块这B玩意首次判断真假有问题
	PMDL DBVMMDL;

	PINITVARS initvars;

	PMDL EnterVMM2MDL;
	
	PMDL OriginalstateMDL;

	PVOID TemporaryPagingSetup;
	
	PMDL TemporaryPagingSetupMDL;

	
	KSPIN_LOCK LoadedOSSpinLock; //spinlock to prevent LoadedOS from being overwritten (should not be needed, but just being safe)
	//----------------------------

#ifdef AMD64
	extern "C" void EnterVMM(void); //declared in vmxoffloada.asm
	extern "C" void EnterVMMPrologue(void);
	extern "C" void EnterVMMEpilogue(void);
	extern "C" void JTAGBP(void);
#else
	_declspec(naked) void EnterVMM(void)
	{
		__asm
		{
	begin:
			xchg bx, bx //trigger bochs breakpoint

			//setup the GDT
			lgdt[ebx] //ebx is the 'virtual address' so just do that before disabling paging ok...

			//switch to identify mapped pagetable
			mov cr3, edx
			jmp short weee
	weee :
			//now jump to the physical address (identity mapped to the same virtual address)
			mov eax, secondentry
			sub eax, begin
			add eax, esi
			jmp eax

	secondentry :

			//disable paging		
			mov eax, cr0
			and eax, 0x7FFFFFFF
			mov cr0, eax
			//paging off
			jmp short weee2
	weee2 :

			//load paging for vmm (but don't apply yet, in nonpaged mode)
			mov cr3, ecx

			//enable PAE and PSE
			mov eax, 0x30
			__emit 0x0f  //-|
			__emit 0x22  //-|-mov cr4,eax  (still WTF's me that visual studio doesn't know about cr4)
			__emit 0xe0  //-|


			mov ecx, 0xc0000080 //enable efer_lme
			rdmsr
			or eax, 0x100
			wrmsr

			//mov eax,cr0		
			//or eax,0x80000020 //re-enable pg (and ne to be sure)
			//edit, who cares, fuck the original state, it's my own state now
			mov eax, 0x80000021
			mov cr0, eax

			mov eax, edi //tell dbvm it's an OS entry and a that location the start info is
			mov ebx, ebp //tell vmmPA

			__emit 0xea  //-|
			__emit 0x00  //-|
			__emit 0x00  //-|
			__emit 0x40  //-|JMP FAR 0x50:0x00400000
			__emit 0x00  //-|
			__emit 0x50  //-|
			__emit 0x00  //-|

			__emit 0xce
			__emit 0xce
			__emit 0xce
			__emit 0xce
			__emit 0xce
			__emit 0xce
			__emit 0xce
		}
	}
#endif

	PVOID InstallImage(BOOL bNeed_Uncompress, OUT ULONG64& uImageFileLength)
	{
		PVOID ImageFileBuffer = NULL;

		errno_t err; //错误变量的定义

		ULONG fileLength = sizeof(g_image_data);
		uLongf uDestBufferLen = 1 * 1024 * 1024; //此处长度需要足够大以容纳解压缩后数据

		ImageFileBuffer = (char*)ImpCall(ExAllocatePool, NonPagedPool, 1 * 1024 * 1024);
		if (ImageFileBuffer)
		{
			if (bNeed_Uncompress)
			{
				LOG_DEBUG("uncompress Before Data Length:%d bytes\r\n", fileLength);
				//解压缩buffer中的数据
				err = uncompress((Bytef*)ImageFileBuffer, (uLongf*)&uDestBufferLen, (Bytef*)g_image_data, (uLongf)fileLength);
				if (err == Z_OK)
				{
					LOG_DEBUG("uncompress After Data Length:%d bytes\r\n", uDestBufferLen);
					uImageFileLength = uDestBufferLen;
				}
				else
				{
					LOG_DEBUG("uncompress failed：%d\r\n", err);
					ImpCall(ExFreePool,ImageFileBuffer);
					ImageFileBuffer = NULL;
				}
				LOG_DEBUG("uncompress Data Address:%p\r\n", ImageFileBuffer);
			}
			else
			{
				//LOG_DEBUG("文件大小:%d\r\n", fileLength);
				RtlCopyMemory(ImageFileBuffer, g_image_data, fileLength);
				uImageFileLength = fileLength;
			}
		}

		return ImageFileBuffer;
	}

	void UnInstallImage(PVOID ImageFileBuffer)
	{
		if (ImageFileBuffer)
		{
			ImpCall(ExFreePool,ImageFileBuffer);
		}
	}

	BOOL InitializeDBVM_Internal(DWORD vmmsize, PHYSICAL_ADDRESS& LowAddress, PHYSICAL_ADDRESS& HighAddress, PHYSICAL_ADDRESS& SkipBytes)
	{
		int i = 0;
		PHYSICAL_ADDRESS maxPA;

		//basic paging setup for the vmm, will get expanded by the vmm itself
		UINT64* GDTBase;
		PPDPTE_PAE	PageMapLevel4;
		PPDPTE_PAE	PageDirPtr;
		PPDE_PAE	PageDir;
		PPTE_PAE	PageTable1, PageTable2;
		UINT_PTR	FreeVA = (((UINT_PTR)vmm + vmmsize) & 0xfffffffffffff000ULL) + 4096; //next free virtual address

		UINT64		mainstack;
		initvars = (PINITVARS)&vmm[0x10];

		mainstack = FreeVA; FreeVA += 16 * 4096;

		GDTBase = (UINT64*)FreeVA; FreeVA += 4096;
		PageDirPtr = (PPDPTE_PAE)FreeVA; FreeVA += 4096;
		PageDir = (PPDE_PAE)FreeVA;  FreeVA += 4096;
		PageTable1 = (PPTE_PAE)FreeVA;  FreeVA += 4096;
		PageTable2 = (PPTE_PAE)FreeVA;  FreeVA += 4096;
		PageMapLevel4 = (PPDPTE_PAE)FreeVA;  FreeVA += 4096; //has to be the last alloc

		DBVMPML4PA = (UINT_PTR)ImpCall(MmGetPhysicalAddress,PageMapLevel4).QuadPart;

		//blame MS for making this hard to read
		//LOG_DEBUG("Setting up initial paging table for vmm\n");

		*(PUINT64)(&PageMapLevel4[0]) = ImpCall(MmGetPhysicalAddress,PageDirPtr).QuadPart;
		PageMapLevel4[0].P = 1;
		PageMapLevel4[0].RW = 1;

		*(PUINT64)(&PageDirPtr[0]) = ImpCall(MmGetPhysicalAddress,PageDir).QuadPart;
		PageDirPtr[0].P = 1;
		PageDirPtr[0].RW = 1;

		//DBVM 11 does no longer need the map at 0 to 00400000
		*(PUINT64)(&PageDir[0]) = 0; //00000000-00200000
		PageDir[0].P = 1;
		PageDir[0].RW = 0; //map as readonly (only for the jump to 0x00400000)
		PageDir[0].PS = 1;

		*(PUINT64)(&PageDir[1]) = 0x00200000; //00200000-00400000
		PageDir[1].P = 1;
		PageDir[1].RW = 0;
		PageDir[1].PS = 1;

		{
			*(PUINT64)(&PageDir[2]) = ImpCall(MmGetPhysicalAddress,PageTable1).QuadPart;
			PageDir[2].P = 1;
			PageDir[2].RW = 1;
			PageDir[2].PS = 0; //points to a pagetable 

			*(PUINT64)(&PageDir[3]) = ImpCall(MmGetPhysicalAddress,PageTable2).QuadPart;
			PageDir[3].P = 1;
			PageDir[3].RW = 1;
			PageDir[3].PS = 0;
		}

		//fill in the pagetables
		for (i = 0; i < 1024; i++) //pagetable1 and 2 are allocated after eachother, so 1024 can be used here using pagetable1
		{
			*(PUINT64)(&PageTable1[i]) = ImpCall(MmGetPhysicalAddress,(PVOID)(((UINT_PTR)vmm) + (4096 * i))).QuadPart;
			PageTable1[i].P = 1;
			PageTable1[i].RW = 1;
		}

		i = (int)((UINT64)((mainstack - (UINT64)vmm)) >> 12);
		PageTable1[i].P = 0; //mark the first page of the stack as unreadable


		//setup GDT
		GDTBase[0] = 0;						    //0 :
		GDTBase[1] = 0x00cf92000000ffffULL;	    //8 : 32-bit data
		GDTBase[2] = 0x00cf96000000ffffULL;	    //16: test, stack, failed, unused
		GDTBase[3] = 0x00cf9b000000ffffULL;	    //24: 32-bit code
		GDTBase[4] = 0x00009a000000ffffULL;	    //32: 16-bit code
		GDTBase[5] = 0x000092000000ffffULL;	    //40: 16-bit data
		GDTBase[6] = 0x00009a030000ffffULL;	    //48: 16-bit code, starting at 0x30000
		GDTBase[7] = 0;						    //56: 32-bit task	
		GDTBase[8] = 0;						    //64: 64-bit task
		GDTBase[9] = 0;						    //72:  ^   ^   ^
		GDTBase[10] = 0x00af9b000000ffffULL;	//80: 64-bit code
		GDTBase[11] = 0;						//88:  ^   ^   ^
		GDTBase[12] = 0;						//96: 64-bit tss descriptor (2)
		GDTBase[13] = 0;						//104: ^   ^   ^


		NewGDTDescriptor.limit = 0x6f; //111
		NewGDTDescriptor.base = 0x00400000 + (UINT64)GDTBase - (UINT64)vmm;

		//LOG_DEBUG("&NewGDTDescriptor=%p, &NewGDTDescriptor.limit=%p, &NewGDTDescriptor.base=%p\n", &NewGDTDescriptor, &NewGDTDescriptor.limit, &NewGDTDescriptor.base);
		//LOG_DEBUG("NewGDTDescriptor.limit=%x\n", NewGDTDescriptor.limit);
		//LOG_DEBUG("NewGDTDescriptor.base=%p\n", NewGDTDescriptor.base);

		NewGDTDescriptorVA = (UINT_PTR)&NewGDTDescriptor;

		maxPA.QuadPart = 0x003fffffULL; //allocate 4k at the lower 4MB
		//LOG_DEBUG("Before enterVMM2 alloc: maxPA=%I64x\n", maxPA.QuadPart);

		EnterVMM2 = (unsigned char*)ImpCall(MmAllocateContiguousMemory,4096, maxPA);
		if (EnterVMM2)
		{
			unsigned char* original = (unsigned char*)EnterVMM;
			RtlZeroMemory(EnterVMM2, 4096);

			EnterVMM2MDL = ImpCall(IoAllocateMdl,EnterVMM2, 4096, FALSE, FALSE, NULL);
			ImpCall(MmProbeAndLockPages, EnterVMM2MDL, KernelMode, IoReadAccess);

			//LOG_DEBUG("enterVMM is located at %p (%I64x)\n", enterVMM, MmGetPhysicalAddress(enterVMM).QuadPart);
			//LOG_DEBUG("enterVMM2 is located at %p (%I64x)\n", enterVMM2, MmGetPhysicalAddress(enterVMM2).QuadPart);


			//LOG_DEBUG("Copying function till end\n");
			//copy memory

			i = 0;
			while ((i < 4096) && ((original[i] != 0xce) || (original[i + 1] != 0xce) || (original[i + 2] != 0xce) || (original[i + 3] != 0xce) || (original[i + 4] != 0xce)))
				i++;

			//LOG_DEBUG("Size is %d", i);

			RtlCopyMemory(EnterVMM2, original, i);
			//LOG_DEBUG("Copy done\n");
		}
		else
		{
			LOG_DEBUG("Failure allocating enterVMM2\n");
			return FALSE;
		}

		//now create a paging setup where enterVMM2 is identity mapped AND mapped at the current virtual address, needed to be able to go down to nonpaged mode
		//easiest way, make every page point to enterVMM2

		//allocate 4 pages
		//LOG_DEBUG("Allocating memory for the temp pagedir\n");
		TemporaryPagingSetup = ImpCall(ExAllocatePool,PagedPool, 4 * 4096);
		if (TemporaryPagingSetup == NULL)
		{
			LOG_DEBUG("TemporaryPagingSetup==NULL!!!\n");
			return FALSE;
		}

		TemporaryPagingSetupMDL = ImpCall(IoAllocateMdl, TemporaryPagingSetup, 4 * 4096, FALSE, FALSE, NULL);
		ImpCall(MmProbeAndLockPages, TemporaryPagingSetupMDL, KernelMode, IoReadAccess);

		RtlZeroMemory(TemporaryPagingSetup, 4096 * 4);
		//LOG_DEBUG("TemporaryPagingSetup is located at %p (%I64x)\n", TemporaryPagingSetup, MmGetPhysicalAddress(TemporaryPagingSetup).QuadPart);


		TemporaryPagingSetupPA = MmGetMdlPfnArray(TemporaryPagingSetupMDL)[0] << 12; // (UINT_PTR)MmGetPhysicalAddress(TemporaryPagingSetup).QuadPart;

		EnterVMM2PA = MmGetMdlPfnArray(EnterVMM2MDL)[0] << 12;
		//LOG_DEBUG("TemporaryPagingSetupPA = (%I64x) (Should be %I64x)\n", (UINT64)TemporaryPagingSetupPA, (UINT64)MmGetPhysicalAddress(TemporaryPagingSetup).QuadPart);
#ifdef AMD64			
					//LOG_DEBUG("Setting up temporary paging setup for x64\n");

		{
			PUINT64 PML4Table = (PUINT64)TemporaryPagingSetup;
			PUINT64	PageDirPtr = (PUINT64)((UINT_PTR)TemporaryPagingSetup + 4096);
			PUINT64	PageDir = (PUINT64)((UINT_PTR)TemporaryPagingSetup + 2 * 4096);
			PUINT64	PageTable = (PUINT64)((UINT_PTR)TemporaryPagingSetup + 3 * 4096);

			//LOG_DEBUG("PAE paging\n");
			for (i = 0; i < 512; i++)
			{
				PML4Table[i] = ImpCall(MmGetPhysicalAddress,PageDirPtr).QuadPart;
				((PPDPTE_PAE)(&PML4Table[i]))->P = 1;

				PageDirPtr[i] = ImpCall(MmGetPhysicalAddress,PageDir).QuadPart;
				((PPDPTE_PAE)(&PageDirPtr[i]))->P = 1;

				PageDir[i] = ImpCall(MmGetPhysicalAddress,PageTable).QuadPart;
				((PPDE_PAE)(&PageDir[i]))->P = 1;
				((PPDE_PAE)(&PageDir[i]))->PS = 0; //4KB

				PageTable[i] = EnterVMM2PA;
				((PPTE_PAE)(&PageTable[i]))->P = 1;
			}
		}

#else
		LOG_DEBUG("Setting up temporary paging setup\n");
		if (PTESize == 8) //PAE paging
		{
			PUINT64	PageDirPtr = (PUINT64)TemporaryPagingSetup;
			PUINT64	PageDir = (PUINT64)((UINT_PTR)TemporaryPagingSetup + 4096);
			PUINT64	PageTable = (PUINT64)((UINT_PTR)TemporaryPagingSetup + 2 * 4096);

			LOG_DEBUG("PAE paging\n");
			for (i = 0; i < 512; i++)
			{
				PageDirPtr[i] = MmGetPhysicalAddress(PageDir).QuadPart;
				((PPDPTE_PAE)(&PageDirPtr[i]))->P = 1;
				//((PPDPTE_PAE)(&PageDirPtr[i]))->RW=1;


				PageDir[i] = MmGetPhysicalAddress(PageTable).QuadPart;
				((PPDE_PAE)(&PageDir[i]))->P = 1;
				//((PPDE_PAE)(&PageDir[i]))->RW=1;							
				((PPDE_PAE)(&PageDir[i]))->PS = 0; //4KB

				PageTable[i] = MmGetPhysicalAddress(EnterVMM2).QuadPart;
				((PPTE_PAE)(&PageTable[i]))->P = 1;
				//((PPTE_PAE)(&PageTable[i]))->RW=1;					

			}

		}
		else
		{
			//normal(old) 4 byte page entries
			PDWORD PageDir = (PDWORD)TemporaryPagingSetup;
			PDWORD PageTable = (PDWORD)((DWORD)TemporaryPagingSetup + 4096);
			LOG_DEBUG("Normal paging\n");
			for (i = 0; i < 1024; i++)
			{
				PageDir[i] = MmGetPhysicalAddress(PageTable).LowPart;
				((PPDE)(&PageDir[i]))->P = 1;
				((PPDE)(&PageDir[i]))->RW = 1;
				((PPDE)(&PageDir[i]))->PS = 0; //4KB

				PageTable[i] = MmGetPhysicalAddress(EnterVMM2).LowPart;
				((PPTE)(&PageTable[i]))->P = 1;
				((PPTE)(&PageTable[i]))->RW = 1;
			}

		}
#endif

		//LOG_DEBUG("Temp paging has been setup\n");

		//EnterVMM2PA = (UINT_PTR)MmGetPhysicalAddress(enterVMM2).QuadPart;

		Originalstate = (POriginalState)ImpCall(ExAllocatePool, PagedPool, 4096);
		OriginalstateMDL = ImpCall(IoAllocateMdl, Originalstate, 4096, FALSE, FALSE, NULL);
		ImpCall(MmProbeAndLockPages, OriginalstateMDL, KernelMode, IoReadAccess);

		RtlZeroMemory(Originalstate, 4096);
		OriginalstatePA = MmGetMdlPfnArray(OriginalstateMDL)[0] << 12; //(UINT_PTR)MmGetPhysicalAddress(Originalstate).QuadPart;					

		//LOG_DEBUG("EnterVMM2PA = 0x%llx\n", EnterVMM2PA);
		//LOG_DEBUG("OriginalstatePA = 0x%llx\n", OriginalstatePA);
		//LOG_DEBUG("OriginalstatePA = 0x%llx\n", (UINT_PTR)MmGetPhysicalAddress(Originalstate).QuadPart);

		//setup init vars	
		initvars->loadedOS = OriginalstatePA;
		initvars->vmmstart = vmmPA;
		initvars->pagedirlvl4 = 0x00400000 + ((UINT64)PageMapLevel4 - (UINT64)vmm);
		initvars->nextstack = 0x00400000 + ((UINT64)mainstack - (UINT64)vmm) + (16 * 4096) - 0x40;
		initvars->contiguousmemory = 0;

		PMDL contiguousMDL = ImpCall(MmAllocatePagesForMdlEx, LowAddress, HighAddress, SkipBytes, 8 * 4096, MmCached, MM_ALLOCATE_REQUIRE_CONTIGUOUS_CHUNKS | MM_ALLOCATE_FULLY_REQUIRED);
		if (contiguousMDL)
		{
			initvars->contiguousmemory = MmGetMdlPfnArray(contiguousMDL)[0] << 12;
			//LOG_DEBUG("contiguous PA = 0x%llx\n", initvars->contiguousmemory);
			initvars->contiguousmemorysize = 8;
			ImpCall(ExFreePool,contiguousMDL);
		}
		else
			LOG_DEBUG("Failed allocating 32KB of contiguous memory\n");

		ImpCall(KeInitializeSpinLock, &LoadedOSSpinLock);

		return TRUE;
	}

	BOOL InitializeDBVM(PCWSTR DbvmImgPath)
	/*
	Runs at passive mode
	*/
	{
		if (InitializedVmm)
			return FALSE; //already initialized

		//LOG_DEBUG("First time run. Initializing vmm section\r\n");

		PHYSICAL_ADDRESS LowAddress, HighAddress, SkipBytes;
		LowAddress.QuadPart = 0;
		HighAddress.QuadPart = -1;
		SkipBytes.QuadPart = 0;

		DBVMMDL = ImpCall(MmAllocatePagesForMdlEx, LowAddress, HighAddress, SkipBytes, 4 * 1024 * 1024, MmCached, MM_ALLOCATE_REQUIRE_CONTIGUOUS_CHUNKS | MM_ALLOCATE_FULLY_REQUIRED);
		if (!DBVMMDL)
		{
			LOG_DEBUG("Failure allocating the required 4MB\r\n");
			return FALSE;
		}

		vmm = (unsigned char*)ImpCall(MmMapLockedPagesSpecifyCache, DBVMMDL, KernelMode, MmCached, NULL, FALSE, 0);

		//default password when dbvm is just loaded (needed for adding extra ram)
		VmxHelper::vmx_password1 = PASSWORD1;
		VmxHelper::vmx_password2 = PASSWORD2;
		VmxHelper::vmx_password3 = PASSWORD3;

		if (vmm)
		{
			do 
			{
				int i;
				PHYSICAL_ADDRESS maxPA;
				HANDLE dbvmimghandle;
				UNICODE_STRING filename;
				IO_STATUS_BLOCK statusblock;
				OBJECT_ATTRIBUTES oa;
				NTSTATUS OpenedFile;

				vmmPA = (UINT_PTR)ImpCall(MmGetPhysicalAddress,vmm).QuadPart;

				//LOG_DEBUG("Allocated memory at virtual address %p (physical address %I64x)\n", vmm, MmGetPhysicalAddress(vmm));
				vmmPA = MmGetMdlPfnArray(DBVMMDL)[0] << 12;
				//LOG_DEBUG("(physical address %I64x)\n", vmmPA);

				RtlZeroMemory(vmm, 4 * 1024 * 1024); //initialize
				//LOG_DEBUG("vmm:%p\r\n", vmm);
				//LOG_DEBUG("DbvmImgPath:%S\r\n", DbvmImgPath);
				ImpCall(RtlInitUnicodeString, &filename, DbvmImgPath);

				//Load the .img file
				InitializeObjectAttributes(&oa, &filename, 0, NULL, NULL);
				OpenedFile = ImpCall(ZwCreateFile, &dbvmimghandle, SYNCHRONIZE | STANDARD_RIGHTS_READ, &oa, &statusblock, NULL, FILE_SYNCHRONOUS_IO_NONALERT | FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, 0, NULL, 0);

				if (OpenedFile == STATUS_SUCCESS)
				{
					WORD startsector;
					LARGE_INTEGER byteoffset;
					FILE_STANDARD_INFORMATION fsi;
					NTSTATUS ReadFile;

					//Getting filesize
					ImpCall(ZwQueryInformationFile, dbvmimghandle, &statusblock, &fsi, sizeof(fsi), FileStandardInformation);

					//fsi.EndOfFile contains the filesize

					if (fsi.EndOfFile.QuadPart > 4 * 1024 * 1024)
					{
						LOG_DEBUG("File bigger than 4MB. Big retard detected\n");
						break;
					}

					byteoffset.QuadPart = 0x8; //offset containing sectornumber of the vmm location 这个位置记录着需要跳过的扇区数量
					ReadFile = ImpCall(ZwReadFile, dbvmimghandle, NULL, NULL, NULL, &statusblock, &startsector, 2, &byteoffset, NULL);

					if (ReadFile == STATUS_PENDING)
					{
						if (ImpCall(ZwWaitForSingleObject, dbvmimghandle, FALSE, NULL) != STATUS_SUCCESS)
						{
							LOG_DEBUG("Read failure\n");
							break;
						}
					}

					if (statusblock.Status == STATUS_SUCCESS)
					{
						DWORD vmmsize = fsi.EndOfFile.LowPart;// -(startsector * 512);

						//now read the vmdisk into the allocated memory
						//LOG_DEBUG("The startsector = %d (that's offset %d)\n", startsector, startsector * 512);  //The startsector = 35 (that's offset 17920)

						byteoffset.QuadPart = startsector * 512;
						ReadFile = ImpCall(ZwReadFile, dbvmimghandle, NULL, NULL, NULL, &statusblock, vmm, vmmsize, &byteoffset, NULL);
						if (ReadFile == STATUS_PENDING)
							ImpCall(ZwWaitForSingleObject, dbvmimghandle, FALSE, NULL);

						vmmsize = (vmmsize + 4096) & 0xfffffffffffff000ULL; //adjust the size internally to a page boundary (sure, there's some mem loss, but it's predicted, dbvm assumes first 10 pages are scratch pages)

						//LOG_DEBUG("vmmsize = 0x%x\n", vmmsize);

						if (statusblock.Status == STATUS_SUCCESS)
						{
							if (InitializeDBVM_Internal(vmmsize, LowAddress, HighAddress, SkipBytes))
							{
								InitializedVmm = TRUE;
							}
						}
					}
					ImpCall(ZwClose, dbvmimghandle);

					//LOG_DEBUG("Opened and processed: %S\n", filename.Buffer);
				}
				else
				{
					LOG_DEBUG("Failure opening the file. Status=%x  (filename=%S)\n", OpenedFile, filename.Buffer);
				}
			} while (FALSE);
			
			//fill in some specific memory regions
			ImpCall(MmUnmapLockedPages, vmm, DBVMMDL);
		}
		else
		{
			LOG_DEBUG("Failure allocating the required 4MB\n");
		}
		ImpCall(ExFreePool, DBVMMDL);

		return InitializedVmm;
	}

	BOOL InitializeDBVM_4NoModule(BOOL Compress)
	{
		InitializedVmm = FALSE;

		LOG_DEBUG("InitializeDBVM_4NoModule\r\n");
		LOG_DEBUG("First time run. Initializing vmm section\r\n");

		CHAR* ImageFileBuffer = NULL;
		ULONG64 uImageFileLength = 0;

		PHYSICAL_ADDRESS LowAddress, HighAddress, SkipBytes;
		LowAddress.QuadPart = 0;
		HighAddress.QuadPart = -1;
		SkipBytes.QuadPart = 0;

		DBVMMDL = ImpCall(MmAllocatePagesForMdlEx, LowAddress, HighAddress, SkipBytes, 4 * 1024 * 1024, MmCached, MM_ALLOCATE_REQUIRE_CONTIGUOUS_CHUNKS | MM_ALLOCATE_FULLY_REQUIRED);
		if (!DBVMMDL)
		{
			LOG_DEBUG("Failure allocating the required 4MB\r\n");
			return FALSE;
		}

		vmm = (unsigned char*)ImpCall(MmMapLockedPagesSpecifyCache, DBVMMDL, KernelMode, MmCached, NULL, FALSE, 0);

		if (vmm)
		{
			do 
			{
				vmmPA = (UINT_PTR)ImpCall(MmGetPhysicalAddress, vmm).QuadPart;

				//4M内容清空
				RtlZeroMemory(vmm, 4 * 1024 * 1024); //initialize

				//Compress为TRUE,则需要解压
				ImageFileBuffer = (CHAR*)InstallImage(Compress, uImageFileLength);

				if (ImageFileBuffer && uImageFileLength)
				{
					WORD startsector;
					LARGE_INTEGER byteoffset;

					//fsi.EndOfFile contains the filesize

					if (uImageFileLength > 4 * 1024 * 1024)
					{
						LOG_DEBUG("File bigger than 4MB. Big retard detected\n");
						break;
					}

					byteoffset.QuadPart = 0x8; //offset containing sectornumber of the vmm location
					startsector = *((WORD*)((char*)ImageFileBuffer + byteoffset.QuadPart));

					//LOG_DEBUG("startsector:%d\r\n", startsector);

					if (startsector != 0)
					{
						DWORD vmmsize = (DWORD)uImageFileLength - (startsector * 512);

						//now read the vmdisk into the allocated memory
						//LOG_DEBUG("The startsector=%d (that's offset %d)\n", startsector, startsector * 512);

						byteoffset.QuadPart = startsector * 512;

						//偏移过31个扇区,把后面的内容全部读出来
						memcpy(vmm, ((char*)ImageFileBuffer + byteoffset.QuadPart), vmmsize);

						//LOG_DEBUG("vmmsize_remain:%d\n", vmmsize);
						//往后蹿4096字节然后0x1000对齐粒度
						vmmsize = (vmmsize + 4096) & 0xfffffffffffff000ULL; //adjust the size internally to a page boundary (sure, there's some mem loss, but it's predicted, dbvm assumes first 10 pages are scratch pages)

						LOG_DEBUG("vmmsize=%x\n", vmmsize);

						if (InitializeDBVM_Internal(vmmsize, LowAddress, HighAddress, SkipBytes))
						{
							InitializedVmm = TRUE;
						}
					}

					UnInstallImage(ImageFileBuffer);
				}
			} while (FALSE);

			//fill in some specific memory regions
			ImpCall(MmUnmapLockedPages, vmm, DBVMMDL);
		}
		else
		{
			LOG_DEBUG("Failure allocating the required 4MB\n");
		}

		ImpCall(ExFreePool, DBVMMDL);

		return InitializedVmm;
	}

	void CleanupDBVM()
	{
		if (!InitializedVmm)
			return;

		if (EnterVMM2MDL) 
		{
			ImpCall(MmUnlockPages,EnterVMM2MDL);
			ImpCall(IoFreeMdl,EnterVMM2MDL);
			EnterVMM2MDL = 0;
		}

		if (EnterVMM2) 
		{
			RtlZeroMemory(EnterVMM2, 4096);
			ImpCall(MmFreeContiguousMemory,EnterVMM2);
			EnterVMM2 = 0;
		}

		if (TemporaryPagingSetupMDL)
		{
			ImpCall(MmUnlockPages,TemporaryPagingSetupMDL);
			ImpCall(IoFreeMdl,TemporaryPagingSetupMDL);
			TemporaryPagingSetupMDL = 0;
		}

		if (TemporaryPagingSetup)
		{
			RtlZeroMemory(TemporaryPagingSetup, 4096 * 4);
			ImpCall(ExFreePool,TemporaryPagingSetup);
			TemporaryPagingSetup = 0;
		}

		if (OriginalstateMDL)
		{
			ImpCall(MmUnlockPages,OriginalstateMDL);
			ImpCall(IoFreeMdl,OriginalstateMDL);
			OriginalstateMDL = 0;
		}

		if (Originalstate)
		{
			RtlZeroMemory(Originalstate, 4096);
			ImpCall(ExFreePool,Originalstate);
			Originalstate = 0;
		}

		InitializedVmm = FALSE;
	}

	void VmxOffLoad_Override(CCHAR cpunr, PKDEFERRED_ROUTINE Dpc, PVOID DeferredContext, PVOID* SystemArgument1, PVOID* SystemArgument2)
	{
		//LOG_DEBUG("[2] VmxOffLoad_Override-Enter\r\n");
		//runs at passive (in any unrelated cpu)

		//allocate 64KB of extra memory for this(and every other) cpu's DBVM
		PHYSICAL_ADDRESS LowAddress, HighAddress, SkipBytes;
		PMDL mdl;
		//LOG_DEBUG("VmxOffLoad_Override\n");
		LowAddress.QuadPart = 0;
		HighAddress.QuadPart = 0xffffffffffffffffI64;
		SkipBytes.QuadPart = 0;
		mdl = ImpCall(MmAllocatePagesForMdlEx, LowAddress, HighAddress, SkipBytes, 64 * 1024, MmCached, MM_ALLOCATE_REQUIRE_CONTIGUOUS_CHUNKS | MM_ALLOCATE_FULLY_REQUIRED); //do not free this, EVER

		if (mdl)
		{
			//convert the pfnlist to a list DBVM understands
			PDBVMOffloadMemInfo mi = (PDBVMOffloadMemInfo)ImpCall(ExAllocatePool, NonPagedPool, sizeof(DBVMOffloadMemInfo));
			int i;
			PFN_NUMBER* pfnlist;

			//LOG_DEBUG("VmxOffLoad_Override: mi=%p\n", mi);

			mi->List = (UINT64*)ImpCall(ExAllocatePool, NonPagedPool, sizeof(UINT64) * 16);

			//LOG_DEBUG("VmxOffLoad_Override: mi->list=%p\n", mi->List);

			pfnlist = MmGetMdlPfnArray(mdl);

			for (i = 0; i < 16; i++)
				mi->List[i] = pfnlist[i] << 12;

			mi->Count = 16;

			ImpCall(ExFreePool,mdl);

			*SystemArgument1 = mi;
		}

		//LOG_DEBUG("VmxOffLoad_Override-Leave\r\n");
	}

	/*__drv_functionClass(KDEFERRED_ROUTINE)
		__drv_maxIRQL(DISPATCH_LEVEL)
		__drv_minIRQL(DISPATCH_LEVEL)
		__drv_requiresIRQL(DISPATCH_LEVEL)
		__drv_sameIRQL*/
	VOID
	VmxOffLoad_Dpc(
		__in struct _KDPC* Dpc,
		__in_opt PVOID DeferredContext,
		__in_opt PVOID SystemArgument1,
		__in_opt PVOID SystemArgument2
	)
	{
		//LOG_DEBUG("[3] VmxOffLoad_Dpc-Enter\r\n");
		int c = cpunr();
		//LOG_DEBUG("VmxOffLoad_Dpc: CPU %d\n", c);
		ImpCall(KeAcquireSpinLockAtDpcLevel,&LoadedOSSpinLock);

		VT_VmxOffLoad::VmxOffLoad();  //启动虚拟化

		//仍然在这里，所以DBVM很可能已加载
		if (SystemArgument1)
		{
			int x = 0;
			PDBVMOffloadMemInfo mi = (PDBVMOffloadMemInfo)SystemArgument1;
			//LOG_DEBUG("mi->List=%p mi->Count=%d\n", mi->List, mi->Count);

			x = vmx_add_memory(mi->List, mi->Count);  //虚拟化得启动成功才能调用
			//LOG_DEBUG("Vmx_Add_Memory returned %x\n", x);

			if (mi->List)
				ImpCall(ExFreePool,mi->List);

			ImpCall(ExFreePool,mi);
		}
		else
			LOG_DEBUG("Error: SystemArgument1=NULL\n");
		ImpCall(KeReleaseSpinLockFromDpcLevel,&LoadedOSSpinLock);

		//LOG_DEBUG("VmxOffLoad_Dpc-Leave\r\n");
	}

	void VmxOffLoad(void)
	{
		//save entry state for easy exit in ReturnFromvmxoffload
		VT_Util::EFLAGS eflags;

		PHYSICAL_ADDRESS minPA, maxPA, boundary;
		GDT gdt;
		IDT idt;

		/*
		__try
		{
			DbgBreakPoint();
		}
		__except (1)
		{
			DbgPrint("No debugger\n");
		}*/

		//allocate 8MB of contigues physical memory
		minPA.QuadPart = 0;
		maxPA.QuadPart = 0xffffffffff000000ULL;
		boundary.QuadPart = 0x00800000ULL; //8 mb boundaries

		//LOG_DEBUG("VmxOffLoad\n");

		if (InitializedVmm)
		{
			//LOG_DEBUG("Cpunr=%d\n", VT_Util::Cpunr());

			//LOG_DEBUG("Storing original state\n");
			Originalstate->cpucount = getCpuCount();
			//LOG_DEBUG("originalstate->cpucount=%d\n", originalstate->cpucount);


			Originalstate->originalEFER = readMSR(0xc0000080); //amd prefers this over an LME

			Originalstate->originalLME = (int)(((DWORD)(readMSR(0xc0000080)) >> 8) & 1);
			//LOG_DEBUG("originalstate->originalLME=%d\n", originalstate->originalLME);


			Originalstate->cr0 = (UINT_PTR)getCR0();


			//LOG_DEBUG("originalstate->cr0=%I64x\n", originalstate->cr0);

			/*
			{
				int xxx;
				unsigned char *x;
				x=&originalstate->cr0;
				for (xxx=0; xxx<8; xxx++)
				{
					DbgPrint("%x ",x[xxx]);
				}
			}
			*/

			Originalstate->cr2 = (UINT_PTR)getCR2();
			//LOG_DEBUG("originalstate->cr2=%I64x", originalstate->cr2);
			/*
			{
				int xxx;
				unsigned char *x;
				x=&originalstate->cr2;
				for (xxx=0; xxx<8; xxx++)
				{
					DbgPrint("%x ",x[xxx]);
				}
			}*/

			Originalstate->cr3 = (UINT_PTR)getCR3();
			//LOG_DEBUG("originalstate->cr3=%I64x",originalstate->cr3);

			Originalstate->cr4 = (UINT_PTR)getCR4();
			//LOG_DEBUG("originalstate->cr4=%I64x",originalstate->cr4);

			Originalstate->ss = getSS();
			Originalstate->ss_AccessRights = getAccessRightsAsm(Originalstate->ss);
			Originalstate->ss_Limit = getSegmentLimitAsm(Originalstate->ss);

			//LOG_DEBUG("originalstate->ss=%I64x",originalstate->ss);
			Originalstate->cs = getCS();
			Originalstate->cs_AccessRights = getAccessRightsAsm(Originalstate->cs);
			Originalstate->cs_Limit = getSegmentLimitAsm(Originalstate->cs);
			//LOG_DEBUG("originalstate->cs=%I64x",originalstate->cs);
			Originalstate->ds = getDS();
			Originalstate->ds_AccessRights = getAccessRightsAsm(Originalstate->ds);
			Originalstate->ds_Limit = getSegmentLimitAsm(Originalstate->ds);
			//LOG_DEBUG("originalstate->ds=%I64x",originalstate->ds);
			Originalstate->es = getES();
			Originalstate->es_AccessRights = getAccessRightsAsm(Originalstate->es);
			Originalstate->es_Limit = getSegmentLimitAsm(Originalstate->es);
			//LOG_DEBUG("originalstate->es=%I64x",originalstate->es);
			Originalstate->fs = getFS();
			Originalstate->fs_AccessRights = getAccessRightsAsm(Originalstate->fs);
			Originalstate->fs_Limit = getSegmentLimitAsm(Originalstate->fs);
			//LOG_DEBUG("originalstate->fs=%I64x",originalstate->fs);
			Originalstate->gs = getGS();
			Originalstate->gs_AccessRights = getAccessRightsAsm(Originalstate->gs);
			Originalstate->gs_Limit = getSegmentLimitAsm(Originalstate->gs);
			//LOG_DEBUG("originalstate->gs=%I64x",originalstate->gs);
			Originalstate->ldt = getLDT();
			//LOG_DEBUG("originalstate->ldt=%I64x",originalstate->ldt);
			Originalstate->tr = getTR();
			//LOG_DEBUG("originalstate->tr=%I64x",originalstate->tr);	


			Originalstate->fsbase = readMSR(0xc0000100);
			Originalstate->gsbase = readMSR(0xc0000101);

			//LOG_DEBUG("originalstate->fsbase=%I64x originalstate->gsbase=%I64x\n", originalstate->fsbase, originalstate->gsbase);


			Originalstate->dr7 = getDR7();


			gdt.vector = 0;
			gdt.wLimit = 0;
			GetGDT(&gdt);
			Originalstate->gdtbase = (UINT64)gdt.vector;
			Originalstate->gdtlimit = gdt.wLimit;

			//LOG_DEBUG("originalstate->gdtbase=%I64x",originalstate->gdtbase);
		    //LOG_DEBUG("originalstate->gdtlimit=%I64x",originalstate->gdtlimit);

			GetIDT(&idt);
			Originalstate->idtbase = (UINT64)idt.vector;
			Originalstate->idtlimit = idt.wLimit;

			//LOG_DEBUG("originalstate->idtbase=%I64x",originalstate->idtbase);
			//LOG_DEBUG("originalstate->idtlimit=%I64x",originalstate->idtlimit);


			eflags = getEflags();
			eflags.IF = 0;
			Originalstate->rflags = *(PUINT_PTR)&eflags;

			Originalstate->rsp = getRSP();
			//LOG_DEBUG("originalstate->rsp=%I64x",originalstate->rsp);
			Originalstate->rbp = getRBP();
			//LOG_DEBUG("originalstate->rbp=%I64x",originalstate->rbp);

			Originalstate->rax = getRAX();
			//LOG_DEBUG("originalstate->rax=%I64x",originalstate->rax);
			Originalstate->rbx = getRBX();
			//LOG_DEBUG("originalstate->rbx=%I64x",originalstate->rbx);
			Originalstate->rcx = getRCX();
			//LOG_DEBUG("originalstate->rcx=%I64x",originalstate->rcx);
			Originalstate->rdx = getRDX();
			//LOG_DEBUG("originalstate->rdx=%I64x",originalstate->rdx);
			Originalstate->rsi = getRSI();
			//LOG_DEBUG("originalstate->rsi=%I64x",originalstate->rsi);
			Originalstate->rdi = getRDI();
			//LOG_DEBUG("originalstate->rdi=%I64x",originalstate->rdi);
#ifdef AMD64
			Originalstate->r8 = getR8();
			//LOG_DEBUG("originalstate->r8=%I64x",originalstate->r8);
			Originalstate->r9 = getR9();
			//LOG_DEBUG("originalstate->r9=%I64x",originalstate->r9);
			Originalstate->r10 = getR10();
			//LOG_DEBUG("originalstate->r10=%I64x",originalstate->r10);
			Originalstate->r11 = getR11();
			//LOG_DEBUG("originalstate->r11=%I64x",originalstate->r11);
			Originalstate->r12 = getR12();
			//LOG_DEBUG("originalstate->r12=%I64x",originalstate->r12);
			Originalstate->r13 = getR13();
			//LOG_DEBUG("originalstate->r13=%I64x",originalstate->r13);
			Originalstate->r14 = getR14();
			//LOG_DEBUG("originalstate->r14=%I64x",originalstate->r14);
			Originalstate->r15 = getR15();
			//LOG_DEBUG("originalstate->r15=%I64x",originalstate->r15);
#endif

#ifdef AMD64

			Originalstate->rsp -= 8; //adjust rsp for the "call entervmmprologue"
			Originalstate->rip = (UINT_PTR)EnterVMMEpilogue; //enterVMMEpilogue is an address inside the entervmmprologue function

			//LOG_DEBUG("originalstate->rip=%llx",originalstate->rip);

			//LOG_DEBUG("Calling entervmm2. (Originalstate=%p (%llx))\n",originalstate,originalstatePA);




			//call to entervmmprologue, pushes the return value on the stack
			EnterVMMPrologue();


			enableInterrupts();

			//LOG_DEBUG("Returned from EnterVMMPrologue\n");

			//LOG_DEBUG("Cpunr=%d\n",VT_Util::Cpunr());



		   //KeLowerIrql(oldirql);



		   //LOG_DEBUG("cpunr=%d\n",cpunr());
#else


			{
				ULONG vmmentryeip;

				__asm
				{
					lea eax, [EnterVMMEpilogue]
					mov vmmentryeip, eax
				}
				Originalstate->rip = (UINT64)vmmentryeip;
			}


			__asm 
			{
				cli //goodbye interrupts						
				xchg bx, bx


				mov ebx, vmmPA
				__emit 0x8b
				__emit 0xeb //mov ebp,ebx


				lea ebx, NewGDTDescriptor
				mov ecx, DBVMPML4PA;
				mov edx, TemporaryPagingSetupPA //for the mov cr3,ecx

				mov esi, EnterVMM2PA
				mov edi, OriginalstatePA

				call[EnterVMM2]

			//Will never get here. NEVER
			FUUUUU:
				xchg bx, bx
				jmp FUUUUU

			EnterVMMEpilogue:
				//cli //test
				nop
				nop
				xchg bx, bx //bochs bp
				nop
				nop
				sti
				nop
				nop
				nop
				nop
				nop
				nop
			}
			//KeLowerIrql(oldirql);

#endif
		   //LOG_DEBUG("Returning\n");

			return;


		}

	}

}


