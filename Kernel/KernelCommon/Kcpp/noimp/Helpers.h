#pragma once

#define ALWAYS_INLINE __attribute__((always_inline))

//DBG Help
#define DBG1 0
#ifdef DBG1

//#define wsp(a) DbgPrintEx(0, 0, "\nFACE WSTR: %ws\n", (a))
//#define hp(a) DbgPrintEx(0, 0, "\nFACE HEX: 0x%p\n", (a))
//#define sp(a) DbgPrintEx(0, 0, "\nFACE STR: %s\n", (a))
//#define dp(a) DbgPrintEx(0, 0, "\nFACE DEC: %d\n", (a))
//#define fp(a) DbgPrintEx(0, 0, "\nFACE FP: %d.%d\n", (int)(a), abs((int)(((a) - (int)(a)) * 1000000)))
#endif

//bool KeAreInterruptsEnabled() {
//	return (__readeflags() & 0x200) != 0;
//}

//ptr utils
template <typename Type>
INLINE Type EPtr(Type Ptr) {
	return (Type)((ULONG64)Ptr);
}

template<typename Ret = void, typename... ArgT>
INLINE Ret CallPtr(PVOID Fn, ArgT... Args) {
	typedef Ret(*ShellFn)(ArgT...);
	return ((ShellFn)Fn)(Args...);
}

//kernel memory utils
INLINE PVOID KAlloc(ULONG Size, bool exec = true) {
	PVOID Buff = ExAllocatePool2(exec ? NonPagedPool : NonPagedPoolNx, Size, 'yc');
	MemZero(Buff, Size);
	return Buff;
}

INLINE void KFree(PVOID Ptr) {
	ExFreePool(Ptr);
}

//basic utils
PVOID FindSection(PVOID ModBase, const char* Name, PULONG SectSize = 0);

bool MakeValid(PVOID addr);

bool FixUserMemory(PVOID ModBase, ULONG RegSize = 0);

PUCHAR FindPatternSect2(PVOID ModBase, const char* SectName, const char* Pattern);

#include "Memory.h"

bool readByte(PVOID addr, UCHAR* ret);

//���������ģ����ը�ѵĿ���
//ALWAYS_INLINE
PUCHAR FindPatternSect(PVOID ModBase, const char* SectName, const char* Pattern);

PUCHAR FindPatternRange(PVOID Start, u32 size, const char* Pattern);

PVOID NQSI(SYSTEM_INFORMATION_CLASS Class);

NOINLINE PVOID GetProcAdress(PVOID ModBase, const char* Name);

INLINE void Sleep(LONG64 MSec) {
	LARGE_INTEGER Delay; Delay.QuadPart = -MSec * 10000;
	KeDelayExecutionThread(KernelMode, false, &Delay);
}

//process utils
INLINE PEPROCESS AttachToProcess(HANDLE PID)
{
	//get eprocess
	PEPROCESS Process = nullptr;
	if (PsLookupProcessByProcessId(PID, &Process) || !Process)
		return nullptr;

	//take process lock
	if (PsAcquireProcessExitSynchronization(Process))
	{
		//process lock failed
		ObfDereferenceObject(Process);
		return nullptr;
	}

	//attach to process
	KeAttachProcess(Process);
	return Process;
}

INLINE void DetachFromProcess(PEPROCESS Process)
{
	//check valid process
	if (Process != nullptr)
	{
		//de-attach to process
		KeDetachProcess();

		//cleanup & process unlock
		ObfDereferenceObject(Process);
		PsReleaseProcessExitSynchronization(Process);
	}
}

PVOID GetUserModuleBase(PEPROCESS Process, const char* ModName, ULONG* ModSize = 0, bool force64 = 0);

PVOID GetUserModuleBase1(PEPROCESS Process, const char* ModName);

DWORD Protect(PVOID addr, DWORD Prot);

INLINE PVOID UAlloc(ULONG Size, ULONG Protect = PAGE_READWRITE, bool load = true) {
	PVOID AllocBase = nullptr; SIZE_T SizeUL = SizeAlign(Size);
#define LOCK_VM_IN_RAM 2
#define LOCK_VM_IN_WORKING_SET 1
	if (!ZwAllocateVirtualMemory(ZwCurrentProcess(), &AllocBase, 0, &SizeUL, MEM_COMMIT, Protect)) {
		//ZwLockVirtualMemory(ZwCurrentProcess(), &AllocBase, &SizeUL, LOCK_VM_IN_WORKING_SET | LOCK_VM_IN_RAM);
		if (load)
			MemZero(AllocBase, SizeUL);
	}
	return AllocBase;
}

INLINE void UFree(PVOID Ptr) {
	SIZE_T SizeUL = 0;
	ZwFreeVirtualMemory(ZwCurrentProcess(), &Ptr, &SizeUL, MEM_RELEASE);
}

//kernel utils
PEPROCESS GetProcessWModule(const char* ProcName, const char* ModName, PVOID* WaitModBase = nullptr);

PVOID GetKernelModuleBase(const char* ModName, ULONG* ModSize = nullptr);

PUCHAR FindPatternInProcess(ULONG ModBase, const char* Pattern);

INLINE void ForceWrite(PVOID Dst, PVOID Src, ULONG Size) 
{
	//bool enable = KeAreInterruptsEnabled();
	_disable();
	auto cr0 = __readcr0();
	__writecr0(cr0 & 0xFFFEFFFF);
	memcpy(Dst, Src, Size);
	__writecr0(cr0);
	//if(enable)
		_enable();
}

//memory
template<typename ReadType>
__forceinline ReadType Read(PVOID Addr)
{
	ReadType ReadData{};
	if (Addr && MmIsAddressValid((PVOID)Addr)) {
		ReadData = *(ReadType*)Addr;
	}

	return ReadData;
}

__forceinline bool ReadArr(PVOID Addr, PVOID Buff, ULONG Size)
{
	if (MmIsAddressValid((PVOID)Addr)) {
		MemCpy(Buff, (PVOID)Addr, Size);
		return true;
	}

	return false;
}

template<typename WriteType>
__forceinline void Write(PVOID Addr, WriteType Data)
{
	if (MmIsAddressValid((PVOID)Addr)) {
		*(WriteType*)Addr = Data;
	}
}

__forceinline void WriteArr(PVOID Addr, PVOID Buff, ULONG Size){
	if (MmIsAddressValid((PVOID)Addr)) {
		MemCpy((PVOID)Addr, Buff, Size);
	}
}

template<typename WriteType>
__forceinline bool WriteProt(PVOID Addr, /*const*/ WriteType/*&*/ Data)
{
	PVOID Addr1 = (PVOID)Addr;
	SIZE_T Size1 = sizeof(WriteType);
	
	//hp(Addr1);

	ULONG oldProt;
	if (!ZwProtectVirtualMemory(ZwCurrentProcess(), &Addr1, &Size1, PAGE_EXECUTE_READWRITE, &oldProt))
	{
		auto data1 = Data;
		MemCpy((PVOID)Addr, &data1, sizeof(WriteType));

		//hp(Addr1);
		ZwProtectVirtualMemory(ZwCurrentProcess(), &Addr1, &Size1, oldProt, &oldProt);

		return true;
	}

	return false;
}

void flush1();

BOOLEAN
MemoryMapperReadMemorySafeByPte(
	PVOID SourceVA,
	PHYSICAL_ADDRESS PaAddressToRead,
	SIZE_T SizeToRead,
	UINT64 PteVaAddress,
	UINT64 MappingVa);

BOOLEAN
MemoryMapperWriteMemorySafeByPte(
	PVOID            SourceVA,
	PHYSICAL_ADDRESS PaAddressToWrite,
	SIZE_T           SizeToWrite,
	UINT64           PteVaAddress,
	UINT64           MappingVa);


// MmCopyVirtualMemory
INLINE BOOL SafeCopy(PVOID dest, PVOID src, SIZE_T size, PEPROCESS Process = NULL)
{
	if (dest == NULL || src == NULL || size == 0)
	{
		return FALSE;
	}

	if (Process == nullptr)
	{
		Process = IoGetCurrentProcess();
	}

	SIZE_T returnSize = 0;

	if (NT_SUCCESS(
		MmCopyVirtualMemory(
			Process,
			src,
			IoGetCurrentProcess(),
			dest, size, KernelMode, &returnSize)
	))
	{
		return TRUE;
	}
	return FALSE;
}

//// Returns an address of a token assuming that Address points to the Token field
PACCESS_TOKEN GetProceesTokenAddress(ULONG_PTR Address);

// MmCopyVirtualMemory
INLINE BOOL SafeCopyRe(PVOID dest, PVOID src, SIZE_T size, PEPROCESS Process)
{
	if (Process == nullptr)
	{
		Process = IoGetCurrentProcess();
	}
	SIZE_T returnSize = 0;
	if (NT_SUCCESS(
			MmCopyVirtualMemory(
				IoGetCurrentProcess(),
				src,
				Process,
				dest,  size, KernelMode, &returnSize)
		)
		&&
		returnSize == size
		)
	{
		return TRUE;
	}
	return FALSE;
}

//VOID TestFunction()
//{
//	DbgPrint(("===================== \r\n"));
//	DbgPrint(("TestFunction \r\n"));
//	DbgPrint(("===================== \r\n"));
//}

//(����ը��)
ULONG SearchControlPid();

//(������ը)
BOOL GiveControlProcessSystemToken();

ULONG GetSystemBuildVersion();

PVOID GetMmAllocateIndependentPages(ULONG64 Size);

PLDR_DATA_TABLE_ENTRY GetModuleByName(PEPROCESS process, PWCHAR moduleName);

//ALWAYS_INLINE
NTSTATUS RtlSuperCopyMemory(_In_ VOID UNALIGNED* Destination, _In_ VOID UNALIGNED* Source, _In_ ULONG Length);

VOID GetSystemModuleBase(CHAR* ModuleName, ULONG64* pModuleBase, ULONG* pBufferSize);

BOOLEAN CheckMask(PCHAR base, PCHAR pattern, PCHAR mask);

PVOID FindPattern(PCHAR base, ULONG length, PCHAR pattern, PCHAR mask);

PVOID FindPatternImage(PCHAR base, PCHAR pattern, PCHAR mask);

INT64 GetPiDDBCacheTableAddr();

//�������PiDDB����Ϣ
VOID ClearPiDDBCacheTable(PDRIVER_OBJECT pDriverObject);

//�������ж����Ϣ
VOID ClearMmUnloaderDriver(PDRIVER_OBJECT pDriverObject);

//���һЩ����������Ϣ
VOID ClearDriverInstallMark(PDRIVER_OBJECT pDriverObject);

PVOID GetNtOsKernelBase();