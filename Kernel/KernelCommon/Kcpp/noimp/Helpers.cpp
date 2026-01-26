#include "KernelCommon.pch.h"
#include "Helpers.h"

PVOID FindSection(PVOID ModBase, const char* Name, PULONG SectSize /*= 0*/)
{
	//get & enum sections
	PIMAGE_NT_HEADERS NT_Header = NT_HEADER(ModBase);
	PIMAGE_SECTION_HEADER Sect = IMAGE_FIRST_SECTION(NT_Header);
	//sp("123");
	////sp(Sect->Name);
	//hp(ModBase);
	//hp(NT_Header);


	for (PIMAGE_SECTION_HEADER pSect = Sect; pSect < Sect + NT_Header->FileHeader.NumberOfSections; pSect++)
	{
		//copy section name
		char SectName[9]; SectName[8] = 0;
		*(ULONG64*)&SectName[0] = *(ULONG64*)&pSect->Name[0];

		//sp(pSect->Name);
		//check name
		if (StrICmp(Name, SectName, true))
		{
			//save size
			if (SectSize)
			{
				ULONG SSize = SizeAlign(_max(pSect->Misc.VirtualSize, pSect->SizeOfRawData));
				*SectSize = SSize;
			}

			//ret full sect ptr
			return (PVOID)((ULONG64)ModBase + pSect->VirtualAddress);
		}
	}

	//no section
	return nullptr;
}

bool MakeValid(PVOID addr)
{
	auto VirtualQuery = [](PVOID Addr, wdk::MEMORY_BASIC_INFORMATION* MemInfo) {
		size_t outSz;

		auto ret = !ImpCall(ZwQueryVirtualMemory, (HANDLE)-1, Addr,
			(::MEMORY_INFORMATION_CLASS)0,
			MemInfo, sizeof(wdk::MEMORY_BASIC_INFORMATION), &outSz);

		return ret;
	};

	if (!ImpCall(MmIsAddressValid, addr)) {
		//SIZE_T RetSize;
		wdk::MEMORY_BASIC_INFORMATION MemInfo{};
		if (VirtualQuery((PVOID)addr, &MemInfo))
		{
			if ((MemInfo.State == MEM_COMMIT) &&
				(MemInfo.Protect & PAGE_EXECUTE_READ))
			{

				//*(volatile char*)addr;
				//sp("ok!");

				//return 0;
				return ImpCall(MmIsAddressValid, addr);
			}

			else
				return false;
		}
		else
			return false;
	}

	return true;
}

bool FixUserMemory(PVOID ModBase, ULONG RegSize /*= 0*/)
{
#define LOCK_VM_IN_RAM 2
#define LOCK_VM_IN_WORKING_SET 1

	//get modsize
	if (!RegSize) {
		PIMAGE_NT_HEADERS NT_Header = NT_HEADER(ModBase);
		PIMAGE_SECTION_HEADER Sect = IMAGE_FIRST_SECTION(NT_Header);
		RegSize = NT_Header->OptionalHeader.SizeOfImage;
	}

	//fix workset size
	QUOTA_LIMITS_EX ql{};
	ql.MinimumWorkingSetSize = 0x200000000; //8GB
	ql.MaximumWorkingSetSize = 0x200000000; //8GB

	QUOTA_LIMITS ql2{};
	ImpCall(ZwQueryInformationProcess, (HANDLE)-1i64, ProcessQuotaLimits, &ql2, sizeof(ql2), NULL);

	//hp(ql2.MinimumWorkingSetSize);
	//hp(ql2.MaximumWorkingSetSize);


	//ZwSetInformationProcess((HANDLE)-1i64, ProcessQuotaLimits, &ql, sizeof(ql));

	//fix mod pages
	for (size_t i = 0; i < RegSize; i += PAGE_SIZE) {
		PVOID v4 = (PVOID)((ULONG64)ModBase + i); size_t v5 = PAGE_SIZE;
		if (!NT_SUCCESS(ImpCall(ZwLockVirtualMemory, (HANDLE)-1i64, &v4, &v5, LOCK_VM_IN_WORKING_SET | LOCK_VM_IN_RAM)))
			return false;
	}
	//sp("ok!");
	//ok!!!
	return true;
}

PUCHAR FindPatternSect2(PVOID ModBase, const char* SectName, const char* Pattern)
{
	//find pattern utils
#define InRange(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (InRange(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
#define GetByte(x) ((UCHAR)(GetBits(x[0]) << 4 | GetBits(x[1])))

//get sect range
	ULONG SectSize;
	PUCHAR ModuleStart = (PUCHAR)FindSection(ModBase, SectName, &SectSize);
	PUCHAR ModuleEnd = ModuleStart + SectSize;

	//scan pattern main
	PUCHAR FirstMatch = nullptr;
	const char* CurPatt = Pattern;
	for (; ModuleStart < ModuleEnd; ++ModuleStart) {
		bool SkipByte = (*CurPatt == '\?');
		if (SkipByte || *ModuleStart == GetByte(CurPatt)) {
			if (!FirstMatch) FirstMatch = ModuleStart;
			SkipByte ? CurPatt += 2 : CurPatt += 3;
			if (CurPatt[-1] == 0) return FirstMatch;
		}

		else if (FirstMatch) {
			ModuleStart = FirstMatch;
			FirstMatch = nullptr;
			CurPatt = Pattern;
		}
	}

	//fail
	return nullptr;
}

bool readByte(PVOID addr, UCHAR* ret)
{
	if (!MakeValid(addr))
	{
		return false;
	}

	*ret = *(volatile char*)addr;
	return true;
}

PUCHAR FindPatternSect(PVOID ModBase, const char* SectName, const char* Pattern)
{
	//find pattern utils
#define InRange(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (InRange(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
#define GetByte(x) ((UCHAR)(GetBits(x[0]) << 4 | GetBits(x[1])))

//get sect range
	ULONG SectSize;
	PUCHAR ModuleStart = (PUCHAR)FindSection(ModBase, SectName, &SectSize);
	PUCHAR ModuleEnd = ModuleStart + SectSize;

	//scan pattern main
	PUCHAR FirstMatch = nullptr;
	const char* CurPatt = Pattern;
	for (; ModuleStart < ModuleEnd; ++ModuleStart)
	{
		bool SkipByte = (*CurPatt == '\?');

		//hp(ModuleStart);
		UCHAR byte1;
		if (!readByte(ModuleStart, &byte1))
		{
			auto addr2 = (u64)ModuleStart;
			addr2 &= 0xFFFFFFFFFFFFF000;
			addr2 += 0xFFF;
			ModuleStart = (PUCHAR)addr2;
			//sp("123");
			goto Skip;
		}

		if (SkipByte || byte1 == GetByte(CurPatt)) {
			if (!FirstMatch) FirstMatch = ModuleStart;
			SkipByte ? CurPatt += 2 : CurPatt += 3;
			if (CurPatt[-1] == 0) return FirstMatch;
		}

		else if (FirstMatch) {
			ModuleStart = FirstMatch;
		Skip:
			FirstMatch = nullptr;
			CurPatt = Pattern;
		}
	}

	//failed
	return nullptr;
}

PUCHAR FindPatternRange(PVOID Start, u32 size, const char* Pattern)
{
	//find pattern utils
#define InRange(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (InRange(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
#define GetByte(x) ((UCHAR)(GetBits(x[0]) << 4 | GetBits(x[1])))

//get sect range
	ULONG SectSize;
	PUCHAR ModuleStart = (PUCHAR)Start;
	PUCHAR ModuleEnd = ModuleStart + size;

	//scan pattern main
	PUCHAR FirstMatch = nullptr;
	const char* CurPatt = Pattern;
	for (; ModuleStart < ModuleEnd; ++ModuleStart)
	{
		bool SkipByte = (*CurPatt == '\?');

		//hp(ModuleStart);
		UCHAR byte1;
		if (!readByte(ModuleStart, &byte1)) {
			auto addr2 = (u64)ModuleStart;
			addr2 &= 0xFFFFFFFFFFFFF000;
			addr2 += 0xFFF;
			ModuleStart = (PUCHAR)addr2;
			//sp("123");
			goto Skip;
		}

		if (SkipByte || byte1 == GetByte(CurPatt)) {
			if (!FirstMatch) FirstMatch = ModuleStart;
			SkipByte ? CurPatt += 2 : CurPatt += 3;
			if (CurPatt[-1] == 0) return FirstMatch;
		}

		else if (FirstMatch) {
			ModuleStart = FirstMatch;
		Skip:
			FirstMatch = nullptr;
			CurPatt = Pattern;
		}
	}

	//failed
	return nullptr;
}

PVOID NQSI(SYSTEM_INFORMATION_CLASS Class)
{
	ULONG ret_size = 1024 * 1024;
	PVOID pInfo = KAlloc(ret_size);
	ImpCall(ZwQuerySystemInformation, Class, pInfo, ret_size, &ret_size);
	return pInfo;
}

NOINLINE PVOID GetProcAdress(PVOID ModBase, const char* Name)
{
	
	//parse headers
	PIMAGE_NT_HEADERS NT_Head = NT_HEADER(ModBase);
	PIMAGE_EXPORT_DIRECTORY ExportDir = (PIMAGE_EXPORT_DIRECTORY)((ULONG64)ModBase + NT_Head->OptionalHeader.DataDirectory[0].VirtualAddress);

	//process records
	for (ULONG i = 0; i < ExportDir->NumberOfNames; i++)
	{
		//get ordinal & name
		USHORT Ordinal = ((USHORT*)((ULONG64)ModBase + ExportDir->AddressOfNameOrdinals))[i];
		const char* ExpName = (const char*)ModBase + ((ULONG*)((ULONG64)ModBase + ExportDir->AddressOfNames))[i];

		//check export name
		if (StrICmp(Name, ExpName, true))
		{
			PVOID Address = (PVOID)((ULONG64)ModBase + ((ULONG*)((ULONG64)ModBase + ExportDir->AddressOfFunctions))[Ordinal]);
			// DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("[+] 0x%llX\r\n"), Address);
			return Address;
		}
			
	}

	//no export
	return nullptr;
}

PVOID GetUserModuleBase(PEPROCESS Process, const char* ModName, ULONG* ModSize /*= 0*/, bool force64 /*= 0*/)
{
	PPEB32 pPeb32 = (PPEB32)ImpCall(PsGetProcessWow64Process, Process);

	if (pPeb32 && !force64)
	{
		if (!pPeb32->Ldr)
			return nullptr;

		for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)(((PPEB_LDR_DATA32)(pPeb32->Ldr)))->InLoadOrderModuleList.Flink;
			pListEntry != (PLIST_ENTRY32)(&(((PPEB_LDR_DATA32)(pPeb32->Ldr)))->InLoadOrderModuleList);
			pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
		{
			PLDR_DATA_TABLE_ENTRY32 pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

			if (StrICmp(ModName, (PWCH)pEntry->BaseDllName.Buffer, false))
			{
				if (ModSize) {
					*ModSize = pEntry->SizeOfImage;
				}

				return (PVOID)pEntry->DllBase;
			}
		}
	}

	else
	{
		PPEB64 PEB = (PPEB64)ImpCall(PsGetProcessPeb, Process);
		if (!PEB->Ldr)
			return nullptr;

		for (PLIST_ENTRY64 pListEntry = (PLIST_ENTRY64)(((PPEB_LDR_DATA64)(PEB->Ldr)))->InLoadOrderModuleList.Flink;
			pListEntry != (PLIST_ENTRY64)(&(((PPEB_LDR_DATA64)(PEB->Ldr)))->InLoadOrderModuleList);
			pListEntry = (PLIST_ENTRY64)pListEntry->Flink)
		{
			PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

			if (StrICmp(ModName, (const char*)pEntry->BaseDllName.Buffer, false))
			{
				if (ModSize) {
					*ModSize = pEntry->SizeOfImage;
				}

				return (PVOID)pEntry->DllBase;
			}
		}
	}

	return nullptr;
}

PVOID GetUserModuleBase1(PEPROCESS Process, const char* ModName)
{
	//get peb & ldr
	PPEB64 PEB = (PPEB64)ImpCall(PsGetProcessPeb, Process);

	if (!PEB || !PEB->Ldr) return nullptr;

	//process modules list (with peb->ldr)
	for (PLIST_ENTRY64 pListEntry = (PLIST_ENTRY64)(((PPEB_LDR_DATA64)(PEB->Ldr)))->InLoadOrderModuleList.Flink;
		pListEntry != (PLIST_ENTRY64)(&(((PPEB_LDR_DATA64)(PEB->Ldr)))->InLoadOrderModuleList);
		pListEntry = (PLIST_ENTRY64)pListEntry->Flink)
	{
		PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (StrICmp(ModName, (const char*)(pEntry->BaseDllName.Buffer), false))
			return pEntry->DllBase;
	}

	//no module
	return nullptr;
}

DWORD Protect(PVOID addr, DWORD Prot)
{
	DWORD oldProt;
	auto addr2 = addr; SIZE_T SizeUL = 1;
	ImpCall(ZwProtectVirtualMemory, (HANDLE)-1, &addr2, &SizeUL, Prot, &oldProt);
	return oldProt;
}

PEPROCESS GetProcessWModule(const char* ProcName, const char* ModName, PVOID* WaitModBase /*= nullptr*/)
{
	//get process list
	PEPROCESS EProc = nullptr;
	PSYSTEM_PROCESS_INFO pInfo = (PSYSTEM_PROCESS_INFO)NQSI(SystemProcessInformation), pInfoCur = pInfo;

	while (true)
	{
		//get process name
		const wchar_t* ProcessName = pInfoCur->ImageName.Buffer;
		if (ImpCall(MmIsAddressValid, (PVOID)ProcessName))
		{
			//check process name
			if (StrICmp(ProcName, ProcessName, true))
			{
				//attach to process
				PEPROCESS Process = AttachToProcess(pInfoCur->UniqueProcessId);
				if (Process != nullptr)
				{
					//check wait module
					PVOID ModBase = GetUserModuleBase(Process, ModName);
					if (ModBase) {
						//check save modbase
						if (WaitModBase)
							*WaitModBase = ModBase;

						EProc = Process;
						break;
					}

					//failed, no wait module
					DetachFromProcess(Process);
				}
			}
		}

		//goto next process entry
		if (!pInfoCur->NextEntryOffset) break;
		pInfoCur = (PSYSTEM_PROCESS_INFO)((ULONG64)pInfoCur + pInfoCur->NextEntryOffset);
	}

	//cleanup
	KFree(pInfo);
	return EProc;
}

PVOID GetKernelModuleBase(const char* ModName, ULONG* ModSize /*= nullptr*/)
{
	//get module list
	PSYSTEM_MODULE_INFORMATION ModuleList = (PSYSTEM_MODULE_INFORMATION)NQSI(SystemModuleInformation);

	//process module list
	PVOID ModuleBase = 0;

	for (ULONG64 i = 0; i < ModuleList->Count; i++) {
		SYSTEM_MODULE_ENTRY Module = ModuleList->Module[i];
		//if (Module.Flags & 0x20) {
		//	sp(&Module.ImageName[Module.ModuleNameOffset]);
		//}
		//

		if (StrICmp(&Module.FullPathName[Module.OffsetToFileName], ModName, true)) {
			ModuleBase = Module.ImageBase;
			if (ModSize) {
				*ModSize = Module.ImageSize;
			} break;
		}
	}

	//cleanup
	KFree(ModuleList);
	return ModuleBase;
}

PUCHAR FindPatternInProcess(ULONG ModBase, const char* Pattern)
{
	//find pattern utils
#define InRange(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (InRange(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
#define GetByte(x) ((UCHAR)(GetBits(x[0]) << 4 | GetBits(x[1])))

//get module range
	PUCHAR ModuleStart = (PUCHAR)ModBase; if (!ModuleStart) return nullptr;
	PIMAGE_NT_HEADERS NtHeader = ((PIMAGE_NT_HEADERS)(ModuleStart + ((PIMAGE_DOS_HEADER)ModuleStart)->e_lfanew));
	PUCHAR ModuleEnd = (PUCHAR)(ModuleStart + NtHeader->OptionalHeader.SizeOfImage - 0x1000); ModuleStart += 0x1000;

	//scan pattern main
	PUCHAR FirstMatch = nullptr;
	const char* CurPatt = Pattern;
	for (; ModuleStart < ModuleEnd; ++ModuleStart)
	{
		bool SkipByte = (*CurPatt == '\?');
		if (SkipByte || *ModuleStart == GetByte(CurPatt))
		{
			if (!FirstMatch)
				FirstMatch = ModuleStart;

			SkipByte ? CurPatt += 2 : CurPatt += 3;

			if (CurPatt[-1] == 0)
				return FirstMatch;
		}

		else if (FirstMatch)
		{
			ModuleStart = FirstMatch;
			FirstMatch = nullptr;
			CurPatt = Pattern;
		}
	}

	return nullptr;
}

void flush1()
{
	auto v9 = __readcr4();
	if ((v9 & 0x20080) != 0) {
		__writecr4(v9 ^ 0x80);
		__writecr4(v9);
	}

	else {
		v9 = __readcr3();
		__writecr3(v9);
	}
}

BOOLEAN MemoryMapperReadMemorySafeByPte(PVOID SourceVA, PHYSICAL_ADDRESS PaAddressToRead, SIZE_T SizeToRead, UINT64 PteVaAddress, UINT64 MappingVa)
{
	PVOID Va = (PVOID)MappingVa;
	PVOID NewAddress;
	PAGE_ENTRY PageEntry;
	PAGE_ENTRY OldPageEntry;
	PPAGE_ENTRY Pte = (PPAGE_ENTRY)PteVaAddress;

	//下面代码主要是把自己建立的pte拷贝到 PageEntry，然后接下去自己改属性
	// 然后把要读写的内存的物理地址的PFN替换到我们自己作的页表上来 进行写操作

	//
	// Copy the previous entry into the new entry
	//
	PageEntry.Flags = Pte->Flags;

	//保存下老的我们自己的pte的属性，之后还原
	OldPageEntry.Flags = Pte->Flags;

	PageEntry.Present = 1;

	//写也给1
	PageEntry.Write = 1;

	// 设置全局位，防止cr3切换的时候刷新tlb
	// Do not flush this page from the TLB on CR3 switch, by setting the
	// global bit in the PTE.
	//
	PageEntry.Global = 1;

	// 把要读写的内存的物理地址的PFN替换到我们自己作的页表上来
	// Set the PFN of this PTE to that of the provided physical address.
	//
	PageEntry.PageFrameNumber = PaAddressToRead.QuadPart >> 12;

	// 把临时的PageEntry重新写道我们自己建立的pte上去
	// Apply the page entry in a single instruction
	//
	Pte->Flags = PageEntry.Flags;

	// 给我们自己建造的pte的虚拟地址刷新下缓存
	KernelIntrin__invlpg(Va);

	//
	// Compute the address
	//
	//拿到具体的 offset=pa&0xfff
	unsigned long Offset = PAGE_4KB_OFFSET & PaAddressToRead.QuadPart;
	//计算对应的va地址=va+offset
	NewAddress = (PVOID)((UINT64)Va + Offset);

	//
	// Move the address into the buffer in a safe manner
	//
	memcpy(SourceVA, NewAddress, SizeToRead);

	//
	//  这里改成还原之前老的，不清空
	//
	Pte->Flags = OldPageEntry.Flags;

	//记得刷新缓存
	KernelIntrin__invlpg(Va);

	return TRUE;
}

BOOLEAN MemoryMapperWriteMemorySafeByPte(PVOID SourceVA, PHYSICAL_ADDRESS PaAddressToWrite, SIZE_T SizeToWrite, UINT64 PteVaAddress, UINT64 MappingVa)
{
	PVOID       Va = (PVOID)MappingVa;
	PVOID       NewAddress;
	PAGE_ENTRY  PageEntry;
	PAGE_ENTRY OldPageEntry;
	PPAGE_ENTRY Pte = (PPAGE_ENTRY)PteVaAddress;

	//
	// Copy the previous entry into the new entry
	//
	PageEntry.Flags = Pte->Flags;

	//保存下老的我们自己的pte的属性，之后还原
	OldPageEntry.Flags = Pte->Flags;

	PageEntry.Present = 1;

	//
	// Generally we want each page to be writable
	//
	PageEntry.Write = 1;

	//
	// Do not flush this page from the TLB on CR3 switch, by setting the
	// global bit in the PTE.
	//
	PageEntry.Global = 1;

	//
	// Set the PFN of this PTE to that of the provided physical address.
	//
	PageEntry.PageFrameNumber = PaAddressToWrite.QuadPart >> 12;

	//
	// Apply the page entry in a single instruction
	//
	Pte->Flags = PageEntry.Flags;

	//
	// Finally, invalidate the caches for the virtual address.
	//
	KernelIntrin__invlpg(Va);

	//
	// Compute the address
	//
	NewAddress = (PVOID)((UINT64)Va + (PAGE_4KB_OFFSET & (PaAddressToWrite.QuadPart)));

	//
	// Move the address into the buffer in a safe manner
	//
	memcpy(NewAddress, SourceVA, SizeToWrite);

	//
	//  这里改成还原之前老的，不清空
	//
	Pte->Flags = OldPageEntry.Flags;


	//记得刷新缓存
	KernelIntrin__invlpg(Va);

	return TRUE;
}

PACCESS_TOKEN GetProceesTokenAddress(ULONG_PTR Address)
{
	//
	// To get an address of a token from the Token field in EPROCESS, the lowest
	// N bits where N is size of a RefCnt field needs to be masked.
	//
	// kd> dt nt!_EX_FAST_REF
	//   + 0x000 Object : Ptr64 Void
	//   + 0x000 RefCnt : Pos 0, 4 Bits
	//   + 0x000 Value  : Uint8B
	//
	ULONG_PTR Value = 0;
	//const ULONG_PTR Value = *reinterpret_cast<ULONG_PTR *>(Address);

	if (ImpCall(MmIsAddressValid, (PVOID)Address))
	{
		SafeCopy(&Value, (PVOID)Address, sizeof(Value), NULL);

		return reinterpret_cast<PACCESS_TOKEN>(Value & (static_cast<ULONG_PTR>(~0xf)));
	}

	return NULL;
}

ULONG SearchControlPid()
{
	uint32_t MagicCode = 0;
	ULONG pid = 40;
	PEPROCESS PrivageProcess = NULL;
	while (pid < 0xFFFFFF)
	{
		if (NT_SUCCESS(ImpCall(PsLookupProcessByProcessId, (HANDLE)pid, &PrivageProcess)))
		{
			g_ControlProcess = PrivageProcess;

			PPEB64 ControlPeb = (PPEB64)ImpCall(PsGetProcessPeb, PrivageProcess);
			PVOID TargetAddress = (PVOID)((ULONG64)ControlPeb + 0x4);

			if (TargetAddress && SafeCopy(&MagicCode, TargetAddress, 4, PrivageProcess))
			{
				/*ULONG64 FunctionAddr = (ULONG64)TestFunction;
				ULONG64 ControlProcessAddress = 0;
				SafeCopy(&ControlProcessAddress, (PVOID)((ULONG64)ControlPeb + 0x6F8), 8, PrivageProcess);
				ImpCall(DbgPrint, "ControlProcessAddress: 0x%llX \r\n", ControlProcessAddress);
				SafeCopyRe((PVOID)ControlProcessAddress, &FunctionAddr, 8, PrivageProcess);*/
				if (MagicCode == (uint32_t)(0xDEADDEAD))
				{
					ULONG Buffer = 0;
					SIZE_T numberOfBytesWritten = 0;

					ImpCall(MmCopyVirtualMemory,
						ImpCall(IoGetCurrentProcess),
						&Buffer,
						PrivageProcess,
						TargetAddress,
						4,
						KernelMode,
						&numberOfBytesWritten
					);

					ImpCall(ObfDereferenceObject, PrivageProcess);
					//LOG_DEBUG("进程提权 PID:%d\r\n", pid);
					return pid;
				}
			}

			ImpCall(ObfDereferenceObject, PrivageProcess);
		}

		pid += 4;
	}

	return 0;
}

BOOL GiveControlProcessSystemToken()
{
	BOOL SuccessLoaded = FALSE;
	PEPROCESS PrivageProcess = NULL;

	ULONG ProcessID = SearchControlPid();

	if (ProcessID > 0)
	{
		if (NT_SUCCESS(ImpCall(PsLookupProcessByProcessId, (HANDLE)ProcessID, &PrivageProcess)))
		{
			g_ControlProcess = PrivageProcess;

			//LOG_DEBUG("找到通信进程 PID:%d\r\n", ProcessID);

			PACCESS_TOKEN PrivageToken = ImpCall(PsReferencePrimaryToken, PrivageProcess);
			PACCESS_TOKEN SystemToken = ImpCall(PsReferencePrimaryToken, (PEPROCESS)(/**(ULONG64*)*/PsInitialSystemProcessFn));

			if (PrivageToken && SystemToken)
			{
				ULONG uSize = sizeof(PVOID) * 0xB0;
				//(蓝屏爆炸)
				for (ULONG Offset = 0ul; Offset < uSize; Offset += sizeof(PVOID))
				{
					// Is this address stores token?
					if (PrivageProcess)
					{
						const auto TestAddress = reinterpret_cast<ULONG_PTR>(PrivageProcess) + Offset;
						if (TestAddress)
						{
							const auto ProbableToken = GetProceesTokenAddress(TestAddress);
							if (ProbableToken)
							{
								if (ProbableToken == PrivageToken)
								{
									// Found the field, replace the contents with the SYSTEM token
									auto TokenAddress = reinterpret_cast<PACCESS_TOKEN*>(TestAddress);
									//*TokenAddress = SystemToken;
									if (ImpCall(MmIsAddressValid, (PVOID)TokenAddress) && ImpCall(MmIsAddressValid,(PVOID)SystemToken))
									{
										SafeCopy((PVOID)TokenAddress, (PVOID)&SystemToken, sizeof(SystemToken), NULL);
										SuccessLoaded = TRUE;
										break;
									}
								}
							}
						}
					}
				}

				ImpCall(PsDereferencePrimaryToken, PrivageToken);
				ImpCall(PsDereferencePrimaryToken, SystemToken);
			}

			ImpCall(ObfDereferenceObject, PrivageProcess);
		}
	}

	return SuccessLoaded;
}

ULONG GetSystemBuildVersion()
{
	NTSTATUS status = STATUS_SUCCESS;
	RTL_OSVERSIONINFOW  version;

	status = ImpCall(RtlGetVersion, &version);
	if (status != STATUS_SUCCESS)
	{
		return 0;
	}
	return version.dwBuildNumber;
}

PVOID GetMmAllocateIndependentPages(ULONG64 Size)
{
	typedef __int64(__fastcall* fn_MmAllocateIndependentPages)(__int64 a1, __int64 a2);
	fn_MmAllocateIndependentPages MmAllocateIndependentPages = NULL;


	ULONG64 FindPtr = 0;
	PVOID Result = NULL;
	NtSystemVersion = GetSystemVersion();

	if (NtSystemVersion <= SystemVersion::Windows10_1803)
	{
		//ULONG64 FindPiDDBCacheTableAddr = 0;
		FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany("PAGE"), oxorany("41 8B D6 B9 00 10 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 2C BE 0C 00 00 00"));
		if ((ULONG64)FindPtr < 0x1000)
		{
			FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("41 8B D6 B9 00 10 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 2C BE 0C 00 00 00"));
			if ((ULONG64)FindPtr < 0x1000)
			{
				return NULL;
			}
		}

		FindPtr = FindPtr + 9;
		MmAllocateIndependentPages = (fn_MmAllocateIndependentPages)(FindPtr + *(PLONG)(FindPtr)+0x4);
		Result = (PVOID)MmAllocateIndependentPages(Size, 0);
		goto Exit;
	}

	if (NtSystemVersion == SystemVersion::Windows10_1809)
	{
		//ULONG64 FindPiDDBCacheTableAddr = 0;
		FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany("PAGE"), oxorany("48 89 5C 24 08 57 48 83 EC 20 48 8B 19 48 8B F9 48 85 DB 75 3C 0F B7 D2 B9 00 70 00 00 E8"));
		if ((ULONG64)FindPtr < 0x1000)
		{
			FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("48 89 5C 24 08 57 48 83 EC 20 48 8B 19 48 8B F9 48 85 DB 75 3C 0F B7 D2 B9 00 70 00 00 E8"));
			if ((ULONG64)FindPtr < 0x1000)
			{
				return NULL;
			}
		}

		FindPtr = FindPtr + 30;
		MmAllocateIndependentPages = (fn_MmAllocateIndependentPages)(FindPtr + *(PLONG)(FindPtr)+0x4);
		Result = (PVOID)MmAllocateIndependentPages(Size, 0);
		goto Exit;
	}

	if (NtSystemVersion == SystemVersion::Windows10_1903)
	{
		//ULONG64 FindPiDDBCacheTableAddr = 0;
		FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany("PAGE"), oxorany("48 89 5C 24 08 57 48 83 EC 20 48 8B 19 48 8B F9 48 85 DB 75 39 B9 00 70 00 00 E8"));
		if ((ULONG64)FindPtr < 0x1000)
		{
			FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("48 89 5C 24 08 57 48 83 EC 20 48 8B 19 48 8B F9 48 85 DB 75 39 B9 00 70 00 00 E8"));
			if ((ULONG64)FindPtr < 0x1000)
			{
				return NULL;
			}
		}

		FindPtr = FindPtr + 27;
		MmAllocateIndependentPages = (fn_MmAllocateIndependentPages)(FindPtr + *(PLONG)(FindPtr)+0x4);
		Result = (PVOID)MmAllocateIndependentPages(Size, 0);
		goto Exit;

	}

	if (NtSystemVersion == SystemVersion::Windows10_1909)
	{
		//ULONG64 FindPiDDBCacheTableAddr = 0;
		FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany("PAGE"), oxorany("48 89 5C 24 08 57 48 83 EC 20 48 8B 19 48 8B F9 48 85 DB 75 39 B9 00 70 00 00 E8"));
		if ((ULONG64)FindPtr < 0x1000)
		{
			FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("48 89 5C 24 08 57 48 83 EC 20 48 8B 19 48 8B F9 48 85 DB 75 39 B9 00 70 00 00 E8"));
			if ((ULONG64)FindPtr < 0x1000)
			{
				return NULL;
			}
		}

		FindPtr = FindPtr + 27;
		MmAllocateIndependentPages = (fn_MmAllocateIndependentPages)(FindPtr + *(PLONG)(FindPtr)+0x4);
		Result = (PVOID)MmAllocateIndependentPages(Size, 0);
		goto Exit;

	}

	if (NtSystemVersion >= SystemVersion::Windows10_2004)
	{

		//ULONG64 FindPiDDBCacheTableAddr = 0;
		FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany("PAGE"), oxorany("41 8B D6 B9 00 10 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 2C BE 0C 00 00 00"));
		if ((ULONG64)FindPtr < 0x1000)
		{
			FindPtr = (ULONG64)FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("41 8B D6 B9 00 10 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 2C BE 0C 00 00 00"));
			if ((ULONG64)FindPtr < 0x1000)
			{
				return NULL;
			}
		}

		FindPtr = FindPtr + 9;
		MmAllocateIndependentPages = (fn_MmAllocateIndependentPages)(FindPtr + *(PLONG)(FindPtr)+0x4);
		Result = (PVOID)MmAllocateIndependentPages(Size, 0);
		goto Exit;

	}

Exit:
	return Result;
}

PLDR_DATA_TABLE_ENTRY GetModuleByName(PEPROCESS process, PWCHAR moduleName)
{
	UNICODE_STRING moduleNameStr = { 0 };
	ImpCall(RtlInitUnicodeString, &moduleNameStr, moduleName);

	PPEB64 PEB = (PPEB64)ImpCall(PsGetProcessPeb, process);

	PLIST_ENTRY64 list = (PLIST_ENTRY64)(&(((PPEB_LDR_DATA64)(PEB->Ldr)))->InLoadOrderModuleList);

	for (PLIST_ENTRY64 entry = (PLIST_ENTRY64)list->Flink; entry != list; )
	{
		PLDR_DATA_TABLE_ENTRY64 module = (PLDR_DATA_TABLE_ENTRY64)CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY64, InLoadOrderLinks);

		if (ImpCall(RtlCompareUnicodeString, (PCUNICODE_STRING)(&module->BaseDllName), &moduleNameStr, TRUE) == 0)
		{
			return module;
		}


		entry = (PLIST_ENTRY64)(module->InLoadOrderLinks.Flink);
	}

	return NULL;
}

NTSTATUS RtlSuperCopyMemory(_In_ VOID UNALIGNED* Destination, _In_ VOID UNALIGNED* Source, _In_ ULONG Length)
{
	KIRQL oldIrql;
	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

	PMDL mdl = ImpCall(IoAllocateMdl, (PVOID)Destination, Length, FALSE, FALSE, nullptr);
	if (mdl == nullptr) {
		KeLowerIrql(oldIrql);
		return STATUS_NO_MEMORY;
	}

	ImpCall(MmBuildMdlForNonPagedPool, mdl);
	// Hack: prevent bugcheck from Driver Verifier and possible future version of Windows
#pragma prefast(push)
	// Disables the warnings specified in a given warning list.
#pragma prefast(disable:__WARNING_MODIFYING_MDL,"Trust me I'm a scientist")	
	CSHORT flags = mdl->MdlFlags;
	mdl->MdlFlags |= MDL_PAGES_LOCKED;
	mdl->MdlFlags &= ~MDL_SOURCE_IS_NONPAGED_POOL;

	// Map pages and do the copy
	PVOID mapped = ImpCall(MmMapLockedPagesSpecifyCache, mdl, KernelMode, MmCached, nullptr, FALSE, HighPagePriority);
	if (mapped == nullptr) {
		mdl->MdlFlags = flags;
		ImpCall(IoFreeMdl, mdl);
		KeLowerIrql(oldIrql);
		return STATUS_NONE_MAPPED;
	}

	RtlCopyMemory(mapped, (const void*)Source, Length);

	ImpCall(MmUnmapLockedPages, mapped, mdl);
	mdl->MdlFlags = flags;
#pragma prefast(pop)

	ImpCall(IoFreeMdl, mdl);
	KeLowerIrql(oldIrql);

	return STATUS_SUCCESS;
}

VOID GetSystemModuleBase(CHAR* ModuleName, ULONG64* pModuleBase, ULONG* pBufferSize)
{
	ULONG NeedSize, i, ModuleCount, BufferSize = 0x5000;
	PVOID pBuffer = nullptr;
	PCHAR pDrvName = nullptr;
	NTSTATUS v_ret_status = { STATUS_UNSUCCESSFUL };
	wdk::PRTL_PROCESS_MODULES v_modules;
	do
	{
		pBuffer = ImpCall(ExAllocatePool, PagedPool, BufferSize);
		if (pBuffer == nullptr)
			return;
		v_ret_status = ImpCall(ZwQuerySystemInformation, wdk::SYSTEM_INFORMATION_CLASS::SystemModuleInformation, pBuffer, BufferSize, &NeedSize);
		if (v_ret_status == STATUS_INFO_LENGTH_MISMATCH)
		{
			ImpCall(ExFreePool, pBuffer);
			BufferSize *= 2;
		}
		else if (!NT_SUCCESS(v_ret_status))
		{
			ImpCall(ExFreePool, pBuffer);
			return;
		}
	} while (v_ret_status == STATUS_INFO_LENGTH_MISMATCH);
	v_modules = static_cast<wdk::PRTL_PROCESS_MODULES>(pBuffer);
	ModuleCount = v_modules->NumberOfModules;
	for (i = 0; i < ModuleCount; i++)
	{
		if (reinterpret_cast<ULONG64>(v_modules->Modules[i].ImageBase) > static_cast<ULONG64>(0x8000000000000000))
		{
			pDrvName = reinterpret_cast<char*>(v_modules->Modules[i].FullPathName);
			if (strstr(pDrvName, ModuleName))
			{

				*pModuleBase = reinterpret_cast<ULONG64>(v_modules->Modules[i].ImageBase);
				*pBufferSize = v_modules->Modules[i].ImageSize;

				goto exit_sub;
			}
		}
	}
exit_sub:
	ImpCall(ExFreePool, pBuffer);
}

BOOLEAN CheckMask(PCHAR base, PCHAR pattern, PCHAR mask)
{
	for (; *mask; ++base, ++pattern, ++mask)
	{
		if (*mask == 'x' && *base != *pattern)
		{
			return FALSE;
		}
	}
	return TRUE;
}

PVOID FindPattern(PCHAR base, ULONG length, PCHAR pattern, PCHAR mask)
{
	length -= (ULONG)strlen(mask);
	for (ULONG i = 0; i <= length; ++i)
	{
		PVOID addr = &base[i];
		if (CheckMask((PCHAR)addr, pattern, mask))
		{
			return addr;
		}
	}

	return 0;
}

PVOID FindPatternImage(PCHAR base, PCHAR pattern, PCHAR mask)
{
	PVOID match = 0;

	PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
	PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);
	for (ULONG i = 0; i < headers->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER section = &sections[i];
		//LOG_DEBUG("sectionName:%s\n", section->Name);
		if (memcmp(section->Name, ".text", 5) == 0)
		{
			match = FindPattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
			if (match)
			{
				break;
			}
		}
	}

	return match;
}

PVOID GetNtOsKernelBase()
{
	PVOID pret = NULL;
	UNICODE_STRING apiname = { 0 };
	PVOID apiaddr = NULL;

	ImpCall(RtlInitUnicodeString, &apiname, oxorany(L"NtCreateFile"));

	apiaddr = ImpCall(MmGetSystemRoutineAddress, &apiname);

	ImpCall(RtlPcToFileHeader, (PVOID)apiaddr, &pret);

	return pret;
}

INT64 GetPiDDBCacheTableAddr()
{
	ULONG64 ntoskrnlBase = (ULONG64)GetNtOsKernelBase();

	if (ntoskrnlBase == 0)
	{
		return 0;
	}

	/*
	48 8D 0D E0 00 2B 00	lea     rcx, PiDDBCacheTable
	E8 EB 41 A1 FF			call    RtlNumberGenericTableElementsAvl
	3D 00 01 00 00			cmp     eax, 100h
	*/
	ULONG64 FindPiDDBCacheTableAddr = 0;

	const char* part = oxorany("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x3D\x00\x01\x00\x00");
	const char* mask = oxorany("xxx????x????xxxxx");

	FindPiDDBCacheTableAddr = (ULONG64)FindPatternImage(reinterpret_cast<PCHAR>(ntoskrnlBase), (PCHAR)(part), (PCHAR)(mask));

	if (!FindPiDDBCacheTableAddr)
	{
		return 0;
	}

	INT64 PiDDBCacheTableAddrOffset = 0;
	INT64 PiDDBCacheTableAddr = 0;
	PiDDBCacheTableAddrOffset = *(PLONG)(FindPiDDBCacheTableAddr + 3);

	PiDDBCacheTableAddr = FindPiDDBCacheTableAddr + PiDDBCacheTableAddrOffset + 7;

	return PiDDBCacheTableAddr;
}

VOID ClearPiDDBCacheTable(PDRIVER_OBJECT pDriverObject)
{
	// NTSTATUS status = STATUS_SUCCESS;

	PIMAGE_DOS_HEADER		 g_pDosHeader = NULL;
	PIMAGE_NT_HEADERS64		 g_pNTHeader = NULL;

	ULONG TimeDateStamp = 0;
	ULONG64 imgBase = 0;

	imgBase = (ULONG64)((wdk::PLDR_DATA_TABLE_ENTRY)(pDriverObject->DriverSection))->DllBase;

	g_pDosHeader = (PIMAGE_DOS_HEADER)imgBase;
	g_pNTHeader = (PIMAGE_NT_HEADERS64)((ULONG64)imgBase + g_pDosHeader->e_lfanew); // PE头
	TimeDateStamp = g_pNTHeader->FileHeader.TimeDateStamp;

	PRTL_AVL_TABLE table = (PRTL_AVL_TABLE)GetPiDDBCacheTableAddr();
	if (!table || !ImpCall(MmIsAddressValid, table) || !ImpCall(MmIsAddressValid, &table->BalancedRoot))
	{
		return;
	}

	PPiDDBCacheEntry PPiDDCacheEntryRoot = (PPiDDBCacheEntry)((ULONG64)(table->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS));
	if (!PPiDDCacheEntryRoot || !ImpCall(MmIsAddressValid, PPiDDCacheEntryRoot))
	{
		return;
	}

	for (PLIST_ENTRY link = PPiDDCacheEntryRoot->List.Flink; link != PPiDDCacheEntryRoot->List.Blink; link = link->Flink)
	{
		PPiDDBCacheEntry PPiDDCacheEntry = (PPiDDBCacheEntry)(link);

		if (!PPiDDCacheEntry || !ImpCall(MmIsAddressValid, PPiDDCacheEntry))
		{
			continue;
		}

		if (!ImpCall(MmIsAddressValid, &PPiDDCacheEntry->DriverName) || !ImpCall(MmIsAddressValid, PPiDDCacheEntry->DriverName.Buffer))
		{
			continue;
		}

		if (PPiDDCacheEntry->TimeDateStamp != TimeDateStamp)
		{
			continue;
		}

		PPiDDBCacheEntry FindTable = (PPiDDBCacheEntry)ImpCall(RtlLookupElementGenericTableAvl, table, PPiDDCacheEntry);
		if (!FindTable) return;

		RemoveEntryList(&FindTable->List);
		ImpCall(RtlDeleteElementGenericTableAvl, table, FindTable);
		//LOG_DEBUG("RtlDeleteElementGenericTableAvl %wZ\n", FindTable->DriverName);
		break;
	}
}

VOID ClearMmUnloaderDriver(PDRIVER_OBJECT pDriverObject)
{
	/*
	MiRememberUnloadedDriver中会验证这个BaseDllName的长度
	如果为0，那么根本就不会添加到MmUnloaderDriver里面去
	此为最简单的消除办法
	*/
	((wdk::PLDR_DATA_TABLE_ENTRY)(pDriverObject->DriverSection))->BaseDllName.Length = 0;
}

VOID ClearDriverInstallMark(PDRIVER_OBJECT pDriverObject)
{
	ClearPiDDBCacheTable(pDriverObject);

	ClearMmUnloaderDriver(pDriverObject);
}
