#include "WeDebug.pch.h"
#include "WeDebug.Symbolic.h"
#include "SymbolicAccess.ModuleExtenderFactory.h"

//SRV* C:\Symbols* https://msdl.microsoft.com/download/symbols

//符号服务器地址
//14393
//http://msdl.microsoft.com/download/symbols/ntkrnlmp.pdb/4DAC3B582A9147ECAED2644CB165222B1/ntkrnlmp.pdb
//http://msdl.microsoft.com/download/symbols/win32kbase.pdb/DB082FF11D914CA1BA49E137CC66F5E11/win32kbase.pdb
//http://msdl.microsoft.com/download/symbols/win32kfull.pdb/A792B492035540D397D66128F30037B01/win32kfull.pdb

#define GET_FUNC_POINTER(Name) \
	Global::Name = (fn##Name)moduleExtender->GetPointer<fn##Name>(#Name); \
	if (Global::Name == NULL) {LOG_ERROR("[-] 系统版本[%d]函数[%s]地址为空\r\n", g_CurrentWindowsBuildNumber, #Name); }\
	else {  LOG_DEBUG("[+] %s:0x%p\n", #Name, Global::Name);}
		

#define CHECK_FUNC_PTR(Ptr) \
    if ((Global::Ptr) == nullptr) { \
		LOG_ERROR("[-] 函数指针为空[%s]\r\n", #Ptr); \
    }

BOOLEAN InitNtoskrnlSymbolsTable()
{
	symbolic_access::ModuleExtenderFactory extenderFactory{};

	const auto& moduleExtender = extenderFactory.Create(oxorany(L"ntoskrnl.exe"));

	if (!moduleExtender.has_value())
	{
		LOG_DEBUG("[-] ntoskrnl.exe 符号初始化失败..\n");

		return FALSE;
	}

	LOG_DEBUG("[+] 成功初始化 ntoskrnl.exe 符号表\n");

	//内核的全局变量

	Global::PspLoaderInitRoutine = moduleExtender->GetPointer<PVOID>(oxorany("PspLoaderInitRoutine"));
	LOG_DEBUG("[+] PspLoaderInitRoutine:0x%p\n", Global::PspLoaderInitRoutine);

	Global::DbgkDebugObjectType = moduleExtender->GetPointer<POBJECT_TYPE>(oxorany("DbgkDebugObjectType"));
	LOG_DEBUG("[+] DbgkDebugObjectType:0x%p\n", Global::DbgkDebugObjectType);

	Global::PspNotifyEnableMask = (PULONG)moduleExtender->GetPointer<PULONG>(oxorany("PspNotifyEnableMask"));
	LOG_DEBUG("[+] PspNotifyEnableMask:0x%p\n", Global::PspNotifyEnableMask);

	Global::PerfGlobalGroupMask = (PULONG)moduleExtender->GetPointer<PULONG>(oxorany("PerfGlobalGroupMask"));
	LOG_DEBUG("[+] PerfGlobalGroupMask:0x%p\n", Global::PerfGlobalGroupMask);

	Global::PspActiveProcessLock = (PEX_PUSH_LOCK)moduleExtender->GetPointer<PEX_PUSH_LOCK>(oxorany("PspActiveProcessLock"));
	LOG_DEBUG("[+] PspActiveProcessLock:0x%p\n", Global::PspActiveProcessLock);

	Global::PspProcessSequenceNumber = (PULONG_PTR)moduleExtender->GetPointer<PULONG_PTR>(oxorany("PspProcessSequenceNumber"));
	LOG_DEBUG("[+] PspProcessSequenceNumber:0x%p\n", Global::PspProcessSequenceNumber);

	Global::PsActiveProcessHead = (PLIST_ENTRY)moduleExtender->GetPointer<PLIST_ENTRY>(oxorany("PsActiveProcessHead"));
	LOG_DEBUG("[+] PsActiveProcessHead:0x%p\n", Global::PsActiveProcessHead);

	//------------------
	//内核函数指针
	GET_FUNC_POINTER(PsFreezeProcess);
	GET_FUNC_POINTER(PsThawProcess);
	GET_FUNC_POINTER(NtCreateDebugObject);
	GET_FUNC_POINTER(DbgkpSuspendProcess);
	GET_FUNC_POINTER(DbgkCreateThread);
	GET_FUNC_POINTER(DbgkpQueueMessage);
	GET_FUNC_POINTER(DbgkpSectionToFileHandle);
	GET_FUNC_POINTER(DbgkpSendApiMessage);
	GET_FUNC_POINTER(DbgkMapViewOfSection);
	GET_FUNC_POINTER(DbgkUnMapViewOfSection);
	GET_FUNC_POINTER(DbgkpSuppressDbgMsg);
	GET_FUNC_POINTER(DbgkExitThread);
	GET_FUNC_POINTER(PsSetProcessFaultInformation);
	GET_FUNC_POINTER(PsCaptureExceptionPort);
	GET_FUNC_POINTER(DbgkpSendApiMessageLpc);
	GET_FUNC_POINTER(DbgkpSendErrorMessage);
	GET_FUNC_POINTER(DbgkForwardException);
	//Global::DbgkpSetProcessDebugObject = (fnDbgkpSetProcessDebugObject)moduleExtender->GetPointer<fnDbgkpSetProcessDebugObject>("DbgkpSetProcessDebugObject");
	GET_FUNC_POINTER(DbgkpPostFakeProcessCreateMessages);
	GET_FUNC_POINTER(NtDebugActiveProcess);
	GET_FUNC_POINTER(DbgkExitProcess);
	GET_FUNC_POINTER(PspExitThread);
	GET_FUNC_POINTER(DbgkpWakeTarget);
	GET_FUNC_POINTER(NtDebugContinue);
	GET_FUNC_POINTER(NtWaitForDebugEvent);
	GET_FUNC_POINTER(ObDuplicateObject);
	GET_FUNC_POINTER(DbgkClearProcessDebugObject);
	GET_FUNC_POINTER(NtRemoveProcessDebug);
	GET_FUNC_POINTER(PsGetNextProcess);
	GET_FUNC_POINTER(DbgkpMarkProcessPeb);
	GET_FUNC_POINTER(PsTerminateProcess);
	GET_FUNC_POINTER(ObCreateObjectType);
	GET_FUNC_POINTER(PsGetNextProcessThread);
	GET_FUNC_POINTER(DbgkpPostFakeThreadMessages);
	GET_FUNC_POINTER(KiStackAttachProcess);
	GET_FUNC_POINTER(KiUnstackDetachProcess);
	GET_FUNC_POINTER(NtReadVirtualMemory);
	GET_FUNC_POINTER(NtWriteVirtualMemory);
	GET_FUNC_POINTER(ZwProtectVirtualMemory);
	GET_FUNC_POINTER(NtProtectVirtualMemory);
	GET_FUNC_POINTER(PspCreateThread);
	GET_FUNC_POINTER(NtCreateThreadEx);
	GET_FUNC_POINTER(NtOpenProcess);
	GET_FUNC_POINTER(DbgkpConvertKernelToUserStateChange);
	GET_FUNC_POINTER(DbgkpOpenHandles);
	GET_FUNC_POINTER(KeCopyExceptionRecord);
	GET_FUNC_POINTER(ObReferenceObjectByHandleWithTag);
	GET_FUNC_POINTER(ObReferenceObjectByHandle);
	GET_FUNC_POINTER(ObfDereferenceObjectWithTag);
	GET_FUNC_POINTER(ObfDereferenceObject);
	GET_FUNC_POINTER(KiCheckForKernelApcDelivery);
	GET_FUNC_POINTER(KeEnterCriticalRegionThread);
	GET_FUNC_POINTER(KeLeaveCriticalRegionThread);
	GET_FUNC_POINTER(MmCopyVirtualMemory);
	GET_FUNC_POINTER(PspCreateUserContext);
	GET_FUNC_POINTER(PspCallThreadNotifyRoutines);
	GET_FUNC_POINTER(PspAllocateThread);
	GET_FUNC_POINTER(ObpReferenceObjectByHandleWithTag);
	GET_FUNC_POINTER(MiObtainReferencedVadEx);
	GET_FUNC_POINTER(MmProtectVirtualMemory);
	GET_FUNC_POINTER(NtGetContextThread);
	GET_FUNC_POINTER(NtSetContextThread);
	GET_FUNC_POINTER(ZwGetContextThread);
	GET_FUNC_POINTER(PspGetContextThreadInternal);
	GET_FUNC_POINTER(KiDispatchException);
	GET_FUNC_POINTER(KeStackAttachProcess);
	GET_FUNC_POINTER(KiStackAttachProcess);
	GET_FUNC_POINTER(NtSetInformationDebugObject);
	GET_FUNC_POINTER(NtTerminateProcess);
	GET_FUNC_POINTER(NtSuspendThread);
	GET_FUNC_POINTER(NtResumeThread);
	GET_FUNC_POINTER(NtQueryInformationThread);
	GET_FUNC_POINTER(PsGetCurrentProcessByThread);
	GET_FUNC_POINTER(PsQuerySystemDllInfo);
	GET_FUNC_POINTER(PsWow64GetProcessNtdllType);
	GET_FUNC_POINTER(PspReferenceSystemDll);
	GET_FUNC_POINTER(MiSectionControlArea);
	GET_FUNC_POINTER(MiReferenceControlAreaFile);
	GET_FUNC_POINTER(ObFastDereferenceObject);
	GET_FUNC_POINTER(DbgkpPostModuleMessages);
	GET_FUNC_POINTER(PsCallImageNotifyRoutines);
	GET_FUNC_POINTER(PsReferenceProcessFilePointer);
	GET_FUNC_POINTER(SeAuditingWithTokenForSubcategory);
	GET_FUNC_POINTER(SeAuditProcessCreation);
	GET_FUNC_POINTER(PspImplicitAssignProcessToJob);
	GET_FUNC_POINTER(PspUnlockProcessListExclusive);
	GET_FUNC_POINTER(DbgkCopyProcessDebugPort);
	GET_FUNC_POINTER(SeCreateAccessStateEx);
	GET_FUNC_POINTER(ObInsertObjectEx);
	GET_FUNC_POINTER(ObCheckRefTraceProcess);
	GET_FUNC_POINTER(PspValidateJobAffinityState);
	GET_FUNC_POINTER(SepDeleteAccessState);
	GET_FUNC_POINTER(PspInsertProcess);
	GET_FUNC_POINTER(DbgkSendSystemDllMessages);
	GET_FUNC_POINTER(DbgkOpenProcessDebugPort);

	Global::PspInheritSyscallProvider = (fnPspInheritSyscallProvider)moduleExtender->GetPointer<fnPspInheritSyscallProvider>(oxorany("PspInheritSyscallProvider"));
	if (Global::PspInheritSyscallProvider)
	{
		LOG_DEBUG("[+] PspInheritSyscallProvider:0x%p (Win11+)\n", Global::PspInheritSyscallProvider);
	}
	else
	{
		LOG_DEBUG("[*] PspInheritSyscallProvider 未找到，当前为 Win10 将跳过\n");
	}

	Global::MiReferenceControlAreaFileWithTag = (fnMiReferenceControlAreaFileWithTag)moduleExtender->GetPointer<fnMiReferenceControlAreaFileWithTag>(oxorany("MiReferenceControlAreaFileWithTag"));
	if (Global::MiReferenceControlAreaFileWithTag)
	{
		LOG_DEBUG("[+] MiReferenceControlAreaFileWithTag:0x%p\n", Global::MiReferenceControlAreaFileWithTag);
	}

	Global::ObTypeIndexTable = moduleExtender->GetPointer<POBJECT_TYPE>(oxorany("ObTypeIndexTable"));
	LOG_DEBUG("[+] ObTypeIndexTable:0x%p\n", Global::ObTypeIndexTable);

	//-----------------------------------------
	Offset::EProcess::Pcb = moduleExtender->GetOffset(oxorany("_EPROCESS"), oxorany("Pcb")).value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::Pcb:0x%X\n", Offset::EProcess::Pcb);

	Offset::EProcess::DebugPort = moduleExtender->GetOffset("_EPROCESS", "DebugPort").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::DebugPort:0x%X\n", Offset::EProcess::DebugPort);

	Offset::EProcess::ImageFileName = moduleExtender->GetOffset("_EPROCESS", "ImageFileName").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::ImageFileName:0x%X\n", Offset::EProcess::ImageFileName);

	Offset::EProcess::WoW64Process = moduleExtender->GetOffset("_EPROCESS", "WoW64Process").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::WoW64Process:0x%X\n", Offset::EProcess::WoW64Process);

	Offset::EProcess::RundownProtect = moduleExtender->GetOffset("_EPROCESS", "RundownProtect").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::RundownProtect:0x%X\n", Offset::EProcess::RundownProtect);

	Offset::EProcess::ExitTime = moduleExtender->GetOffset("_EPROCESS", "ExitTime").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::ExitTime:0x%X\n", Offset::EProcess::ExitTime);

	Offset::EProcess::Flags = moduleExtender->GetOffset("_EPROCESS", "Flags").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::Flags:0x%X\n", Offset::EProcess::Flags);

	Offset::EProcess::SectionBaseAddress = moduleExtender->GetOffset("_EPROCESS", "SectionBaseAddress").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::SectionBaseAddress:0x%X\n", Offset::EProcess::SectionBaseAddress);

	Offset::EProcess::SectionObject = moduleExtender->GetOffset("_EPROCESS", "SectionObject").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::SectionObject:0x%X\n", Offset::EProcess::SectionObject);

	Offset::EProcess::ObjectTable = moduleExtender->GetOffset("_EPROCESS", "ObjectTable").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::ObjectTable:0x%X\n", Offset::EProcess::ObjectTable);

	Offset::EProcess::UniqueProcessId = moduleExtender->GetOffset("_EPROCESS", "UniqueProcessId").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::UniqueProcessId:0x%X\n", Offset::EProcess::UniqueProcessId);

	Offset::EProcess::Job = moduleExtender->GetOffset("_EPROCESS", "Job").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::Job:0x%X\n", Offset::EProcess::Job);

	Offset::EProcess::ActiveProcessLinks = moduleExtender->GetOffset("_EPROCESS", "ActiveProcessLinks").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::ActiveProcessLinks:0x%X\n", Offset::EProcess::ActiveProcessLinks);

	Offset::EProcess::SequenceNumber = moduleExtender->GetOffset("_EPROCESS", "SequenceNumber").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::SequenceNumber:0x%X\n", Offset::EProcess::SequenceNumber);

	Offset::EProcess::ProcessLock = moduleExtender->GetOffset("_EPROCESS", "ProcessLock").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::ProcessLock:0x%X\n", Offset::EProcess::ProcessLock);

	Offset::EProcess::Machine = moduleExtender->GetOffset("_EPROCESS", "Machine").value_or(0);
	LOG_DEBUG("[+] Offset::EProcess::Machine:0x%X\n", Offset::EProcess::Machine);

	Offset::KProcess::DirectoryTableBase = moduleExtender->GetOffset("_KPROCESS", "DirectoryTableBase").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EProcess::DirectoryTableBase:0x%X\n", Offset::KProcess::DirectoryTableBase);

	//-----------------------------------------
	Offset::EThread::Tcb = moduleExtender->GetOffset("_ETHREAD", "Tcb").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EThread::Tcb:0x%X\n", Offset::EThread::Tcb);

	Offset::EThread::CrossThreadFlags = moduleExtender->GetOffset("_ETHREAD", "CrossThreadFlags").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EThread::CrossThreadFlags:0x%X\n", Offset::EThread::CrossThreadFlags);

	Offset::EThread::Cid = moduleExtender->GetOffset("_ETHREAD", "Cid").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EThread::Cid:0x%X\n", Offset::EThread::Cid);

	Offset::EThread::RundownProtect = moduleExtender->GetOffset("_ETHREAD", "RundownProtect").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EThread::RundownProtect:0x%X\n", Offset::EThread::RundownProtect);

	Offset::EThread::Win32StartAddress = moduleExtender->GetOffset("_ETHREAD", "Win32StartAddress").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EThread::Win32StartAddress:0x%X\n", Offset::EThread::Win32StartAddress);

	Offset::EThread::SameThreadPassiveFlags = moduleExtender->GetOffset("_ETHREAD", "SameThreadPassiveFlags").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EThread::SameThreadPassiveFlags:0x%X\n", Offset::EThread::SameThreadPassiveFlags);

	Offset::KThread::ApcState = moduleExtender->GetOffset("_KTHREAD", "ApcState").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::ApcState:0x%X\n", Offset::KThread::ApcState);

	Offset::KThread::PreviousMode = moduleExtender->GetOffset("_KTHREAD", "PreviousMode").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::PreviousMode:0x%X\n", Offset::KThread::PreviousMode);

	Offset::KThread::Teb = moduleExtender->GetOffset("_KTHREAD", "Teb").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::Teb:0x%X\n", Offset::KThread::Teb);

	Offset::KThread::Process = moduleExtender->GetOffset("_KTHREAD", "Process").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::Process:0x%X\n", Offset::KThread::Process);

	Offset::KThread::KernelApcDisable = moduleExtender->GetOffset("_KTHREAD", "KernelApcDisable").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::KernelApcDisable:0x%X\n", Offset::KThread::KernelApcDisable);

	Offset::KThread::MiscFlags = moduleExtender->GetOffset("_KTHREAD", "MiscFlags").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::MiscFlags:0x%X\n", Offset::KThread::MiscFlags);

	Offset::KThread::TrapFrame = moduleExtender->GetOffset("_KTHREAD", "TrapFrame").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::TrapFrame:0x%X\n", Offset::KThread::TrapFrame);

	Offset::KThread::SuspendCount = moduleExtender->GetOffset("_KTHREAD", "SuspendCount").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::SuspendCount:0x%X\n", Offset::KThread::SuspendCount);

	Offset::KThread::SpecialApcDisable = moduleExtender->GetOffset("_KTHREAD", "SpecialApcDisable").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KThread::SpecialApcDisable:0x%X\n", Offset::KThread::SpecialApcDisable);

	Offset::KApcState::Process = moduleExtender->GetOffset("_KAPC_STATE", "Process").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::KApcState::Process:0x%X\n", Offset::KApcState::Process);

	Offset::ImageNtHeaders64::FileHeader = moduleExtender->GetOffset("_IMAGE_NT_HEADERS64", "FileHeader").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ImageNtHeaders64::FileHeader:0x%X\n", Offset::ImageNtHeaders64::FileHeader);

	Offset::ImageNtHeaders64::OptionalHeader = moduleExtender->GetOffset("_IMAGE_NT_HEADERS64", "OptionalHeader").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ImageNtHeaders64::OptionalHeader:0x%X\n", Offset::ImageNtHeaders64::OptionalHeader);

	Offset::ImageNtHeaders64::Signature = moduleExtender->GetOffset("_IMAGE_NT_HEADERS64", "Signature").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ImageNtHeaders64::Signature:0x%X\n", Offset::ImageNtHeaders64::Signature);

	Offset::ImageFileHeader::PointerToSymbolTable = moduleExtender->GetOffset("_IMAGE_FILE_HEADER", "PointerToSymbolTable").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ImageFileHeader::PointerToSymbolTable:0x%X\n", Offset::ImageFileHeader::PointerToSymbolTable);

	Offset::ImageFileHeader::NumberOfSymbols = moduleExtender->GetOffset("_IMAGE_FILE_HEADER", "NumberOfSymbols").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ImageFileHeader::NumberOfSymbols:0x%X\n", Offset::ImageFileHeader::NumberOfSymbols);

	Offset::EWow64Process::Machine = moduleExtender->GetOffset("_EWOW64PROCESS", "Machine").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::EWow64Process::Machine:0x%X\n", Offset::EWow64Process::Machine);

	Offset::Mmvad::Core = moduleExtender->GetOffset("_MMVAD", "Core").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::Mmvad::Core:0x%X\n", Offset::Mmvad::Core);

	Offset::MmvadShort::LongFlags = moduleExtender->GetOffset("_MMVAD_SHORT", "u").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::MmvadShort::LongFlags:0x%X\n", Offset::MmvadShort::LongFlags);

	Offset::HandleTable::UniqueProcessId = moduleExtender->GetOffset("_HANDLE_TABLE", "UniqueProcessId").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::HandleTable::UniqueProcessId:0x%X\n", Offset::HandleTable::UniqueProcessId);

	Offset::ObjectType::TypeInfo = moduleExtender->GetOffset("_OBJECT_TYPE", "TypeInfo").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ObjectType::TypeInfo:0x%X\n", Offset::ObjectType::TypeInfo);

	Offset::ObjectTypeInitializer::GenericMapping = moduleExtender->GetOffset("_OBJECT_TYPE_INITIALIZER", "GenericMapping").value_or(0xFFFFFFFF);
	LOG_DEBUG("[+] Offset::ObjectTypeInitializer::GenericMapping:0x%X\n", Offset::ObjectTypeInitializer::GenericMapping);
	//-----------------------------------------

	return TRUE;
}

BOOLEAN InitWin32kBaseSymbolsTable()
{
	//当前线程必须是gui线程才能访问win32k的内存
	symbolic_access::ModuleExtenderFactory extenderFactory{};
	const auto& moduleExtender = extenderFactory.Create(oxorany(L"win32kbase.sys"));
	if (!moduleExtender.has_value())
	{
		LOG_DEBUG("[-] win32kbase.sys 符号初始化失败..\n");
		return FALSE;
	}

	LOG_DEBUG("[+] 成功初始化 win32kbase.sys 符号表\n");

	GET_FUNC_POINTER(ValidateHwnd);

	return TRUE;
}

BOOLEAN InitWin32kFullSymbolsTable()
{
	//当前线程必须是gui线程才能访问win32k的内存
	symbolic_access::ModuleExtenderFactory extenderFactory{};
	const auto& moduleExtender = extenderFactory.Create(oxorany(L"win32kfull.sys"));
	if (!moduleExtender.has_value())
	{
		LOG_DEBUG("[-] win32kfull.sys 符号初始化失败..\n");
		return FALSE;
	}

	LOG_DEBUG("[+] 成功初始化 win32kfull.sys 符号表\n");

	GET_FUNC_POINTER(NtUserFindWindowEx);
	GET_FUNC_POINTER(NtUserWindowFromPoint);

	return TRUE;
}

void CheckFunctionPointers()
{
	//ntos

	CHECK_FUNC_PTR(PspLoaderInitRoutine);
	CHECK_FUNC_PTR(DbgkDebugObjectType);
	CHECK_FUNC_PTR(PspNotifyEnableMask);
	CHECK_FUNC_PTR(PerfGlobalGroupMask);
	CHECK_FUNC_PTR(PspActiveProcessLock);
	CHECK_FUNC_PTR(PspProcessSequenceNumber);
	CHECK_FUNC_PTR(PsActiveProcessHead);

	CHECK_FUNC_PTR(PsFreezeProcess);
	CHECK_FUNC_PTR(PsThawProcess);
	CHECK_FUNC_PTR(NtCreateDebugObject);
	CHECK_FUNC_PTR(DbgkpSuspendProcess);
	CHECK_FUNC_PTR(DbgkCreateThread);
	CHECK_FUNC_PTR(DbgkpQueueMessage);
	CHECK_FUNC_PTR(DbgkpSectionToFileHandle);
	CHECK_FUNC_PTR(DbgkpSendApiMessage);
	CHECK_FUNC_PTR(DbgkMapViewOfSection);
	CHECK_FUNC_PTR(DbgkUnMapViewOfSection);
	CHECK_FUNC_PTR(DbgkpSuppressDbgMsg);
	CHECK_FUNC_PTR(DbgkExitThread);
	CHECK_FUNC_PTR(PsSetProcessFaultInformation);
	CHECK_FUNC_PTR(PsCaptureExceptionPort);
	CHECK_FUNC_PTR(DbgkpSendApiMessageLpc);
	CHECK_FUNC_PTR(DbgkpSendErrorMessage);
	CHECK_FUNC_PTR(DbgkForwardException);
	CHECK_FUNC_PTR(DbgkpPostFakeProcessCreateMessages);
	CHECK_FUNC_PTR(NtDebugActiveProcess);
	CHECK_FUNC_PTR(DbgkExitProcess);
	CHECK_FUNC_PTR(PspExitThread);
	CHECK_FUNC_PTR(DbgkpWakeTarget);
	CHECK_FUNC_PTR(NtDebugContinue);
	CHECK_FUNC_PTR(NtWaitForDebugEvent);
	CHECK_FUNC_PTR(ObDuplicateObject);
	CHECK_FUNC_PTR(DbgkClearProcessDebugObject);
	CHECK_FUNC_PTR(NtRemoveProcessDebug);
	CHECK_FUNC_PTR(PsGetNextProcess);
	CHECK_FUNC_PTR(DbgkpMarkProcessPeb);
	CHECK_FUNC_PTR(PsTerminateProcess);
	CHECK_FUNC_PTR(ObCreateObjectType);
	CHECK_FUNC_PTR(PsGetNextProcessThread);
	CHECK_FUNC_PTR(DbgkpPostFakeThreadMessages);
	CHECK_FUNC_PTR(KiStackAttachProcess);
	CHECK_FUNC_PTR(KiUnstackDetachProcess);
	CHECK_FUNC_PTR(NtReadVirtualMemory);
	CHECK_FUNC_PTR(NtWriteVirtualMemory);
	CHECK_FUNC_PTR(ZwProtectVirtualMemory);
	CHECK_FUNC_PTR(NtProtectVirtualMemory);
	CHECK_FUNC_PTR(PspCreateThread);
	CHECK_FUNC_PTR(NtCreateThreadEx);
	CHECK_FUNC_PTR(NtOpenProcess);
	CHECK_FUNC_PTR(DbgkpConvertKernelToUserStateChange);
	CHECK_FUNC_PTR(DbgkpOpenHandles);
	CHECK_FUNC_PTR(KeCopyExceptionRecord);
	CHECK_FUNC_PTR(ObReferenceObjectByHandleWithTag);
	CHECK_FUNC_PTR(ObReferenceObjectByHandle);
	CHECK_FUNC_PTR(ObfDereferenceObjectWithTag);
	CHECK_FUNC_PTR(ObfDereferenceObject);
	CHECK_FUNC_PTR(KiCheckForKernelApcDelivery);
	CHECK_FUNC_PTR(KeEnterCriticalRegionThread);
	CHECK_FUNC_PTR(KeLeaveCriticalRegionThread);
	CHECK_FUNC_PTR(MmCopyVirtualMemory);
	CHECK_FUNC_PTR(PspCreateUserContext);

	CHECK_FUNC_PTR(PspCallThreadNotifyRoutines);
	CHECK_FUNC_PTR(PspAllocateThread);
	CHECK_FUNC_PTR(ObpReferenceObjectByHandleWithTag);

	CHECK_FUNC_PTR(MiObtainReferencedVadEx);

	CHECK_FUNC_PTR(MmProtectVirtualMemory);

	
	CHECK_FUNC_PTR(NtGetContextThread);
	CHECK_FUNC_PTR(NtSetContextThread);
	CHECK_FUNC_PTR(ZwGetContextThread);
	CHECK_FUNC_PTR(PspGetContextThreadInternal);
	CHECK_FUNC_PTR(KiDispatchException);
	CHECK_FUNC_PTR(KeStackAttachProcess);
	CHECK_FUNC_PTR(KiStackAttachProcess);
	CHECK_FUNC_PTR(NtSetInformationDebugObject);
	CHECK_FUNC_PTR(NtTerminateProcess);
	CHECK_FUNC_PTR(NtSuspendThread);
	CHECK_FUNC_PTR(NtResumeThread);
	CHECK_FUNC_PTR(NtQueryInformationThread);


	CHECK_FUNC_PTR(PsGetCurrentProcessByThread);

	CHECK_FUNC_PTR(PsQuerySystemDllInfo);
	CHECK_FUNC_PTR(PsWow64GetProcessNtdllType);

	CHECK_FUNC_PTR(PspReferenceSystemDll);
	CHECK_FUNC_PTR(MiSectionControlArea);
	CHECK_FUNC_PTR(MiReferenceControlAreaFile);
	CHECK_FUNC_PTR(ObFastDereferenceObject);
	CHECK_FUNC_PTR(DbgkpPostModuleMessages);
	CHECK_FUNC_PTR(PsCallImageNotifyRoutines);
	CHECK_FUNC_PTR(PsReferenceProcessFilePointer);
	CHECK_FUNC_PTR(SeAuditingWithTokenForSubcategory);
	CHECK_FUNC_PTR(SeAuditProcessCreation);
	CHECK_FUNC_PTR(PspImplicitAssignProcessToJob);
	CHECK_FUNC_PTR(PspUnlockProcessListExclusive);
	CHECK_FUNC_PTR(DbgkCopyProcessDebugPort);
	CHECK_FUNC_PTR(SeCreateAccessStateEx);
	CHECK_FUNC_PTR(ObInsertObjectEx);
	CHECK_FUNC_PTR(ObCheckRefTraceProcess);
	CHECK_FUNC_PTR(PspValidateJobAffinityState);
	CHECK_FUNC_PTR(SepDeleteAccessState);
	CHECK_FUNC_PTR(PspInsertProcess);
	CHECK_FUNC_PTR(DbgkSendSystemDllMessages);
	CHECK_FUNC_PTR(DbgkOpenProcessDebugPort);

	//win32kbase
	CHECK_FUNC_PTR(ValidateHwnd);

	//win32kfull
	CHECK_FUNC_PTR(NtUserFindWindowEx);
	CHECK_FUNC_PTR(NtUserWindowFromPoint);
}

bool DispatchOffsetToHost()
{
	if (!Global::g_IsInitSymbols)
	{
		return false;
	}

	WINDOWS_STRUCT vmcallinfo = { 0 };
	vmcallinfo.ethread_offset_Cid = Offset::EThread::Cid;
	vmcallinfo.command = VT_VMCALL_INIT_OFFSET;
	if (hvgt::vmcall(&vmcallinfo))
	{
		LOG_DEBUG("[+] DispatchOffsetToHost 成功\r\n");
		return true;
	}

	LOG_DEBUG("[-] DispatchOffsetToHost 失败，无法把 offset 发给 WeVt Host\r\n");
	return false;
}