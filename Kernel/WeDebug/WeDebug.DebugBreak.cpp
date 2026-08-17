#include "WeDebug.pch.h"
#include "WeDebug.Control.h"
#include "WeDebug.AsmVmcall.h"
#include "WeDebug.vmcall_reason.h"
#include "WeDebug.DebugBreak.h"

static bool vmcall_internal(PVOID vmcallinfo)
{
	unsigned long ecode = 0;
	bool boSuccess = false;
	__try
	{
		boSuccess = __vm_call(((PVMCALLINFO)vmcallinfo)->command, (unsigned __int64)vmcallinfo, 0, 0);
	}
	__except (ecode = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_DEBUG("[-] VMCALL failed (error: 0x%X)\r\n", ecode);
	}
	return boSuccess;
}

static bool current_vmcall(PVOID vmcallinfo)
{
	return vmcall_internal(vmcallinfo);
}

static bool AddHardwareBreakpoint(PBREAKPOINT_RECORD Breakpoint)
{
	bool boSuccess = false;
	VT_BREAK_POINT vmcallinfo = { 0 };
	vmcallinfo.cr3 = Breakpoint->cr3;
	vmcallinfo.VirtualAddress = (unsigned __int64)Breakpoint->Address;
	vmcallinfo.Size = Breakpoint->length;
	vmcallinfo.command = Breakpoint->command;
	vmcallinfo.CPUCount = Breakpoint->CPUCount;
	vmcallinfo.LoopUserMode = Breakpoint->LoopUserMode;
	vmcallinfo.watchid = -1;

	::KAPC_STATE ApcState;
	PEPROCESS Process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)Breakpoint->pid, &Process);
	if (NT_SUCCESS(status))
	{
		__try
		{
			KeStackAttachProcess(Process, &ApcState);
			*(volatile BYTE*)Breakpoint->Address;
			WeDebug_cli();
			vmcallinfo.cr3 = WeDebug_readcr3();
			WeDebug_sti();
			boSuccess = current_vmcall(&vmcallinfo);
			KeUnstackDetachProcess(&ApcState);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_DEBUG("[-] Set hardware breakpoint crashed\r\n");
		}

		Breakpoint->watchid = vmcallinfo.watchid;

		if (!boSuccess)
		{
			LOG_DEBUG("[-] current_vmcall failed\r\n");
		}
		else
		{
			LOG_DEBUG("[+] current_vmcall ok, errorCode:%d\r\n", vmcallinfo.errorCode);
		}
		ObDereferenceObject(Process);
	}
	return boSuccess;
}

void SetHardwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp)
{
	USER_DATA user = Control::GetUserData(userData);

	PBREAKPOINT_RECORD output = (PBREAKPOINT_RECORD)pIrp->AssociatedIrp.SystemBuffer;
	RtlZeroMemory(output, sizeof(BREAKPOINT_RECORD));

	BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
	DecryptData((PVOID)user.pUserData, aucPlainText);

	size_t numElements = user.uSize / sizeof(BREAKPOINT_RECORD);
	for (size_t i = 0; i < numElements; i++)
	{
		PBREAKPOINT_RECORD pInfo = reinterpret_cast<PBREAKPOINT_RECORD>(aucPlainText + i * sizeof(BREAKPOINT_RECORD));
		if (AddHardwareBreakpoint(pInfo))
		{
			*output = *pInfo;
		}
		break;
	}
	free_pool(aucPlainText);
}

static bool DeleteHardwareBreakpoint(PBREAKPOINT_RECORD Breakpoint)
{
	bool boSuccess = false;
	VT_BREAK_POINT vmcallinfo = { 0 };
	vmcallinfo.cr3 = Breakpoint->cr3;
	vmcallinfo.VirtualAddress = (unsigned __int64)Breakpoint->Address;
	vmcallinfo.Size = Breakpoint->length;
	vmcallinfo.command = VT_VMCALL_WATCH_DELETE;
	vmcallinfo.LoopUserMode = Breakpoint->LoopUserMode;
	vmcallinfo.watchid = Breakpoint->watchid;
	vmcallinfo.CPUCount = Breakpoint->CPUCount;

	::KAPC_STATE ApcState;
	PEPROCESS Process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)Breakpoint->pid, &Process);
	if (NT_SUCCESS(status))
	{
		__try
		{
			KeStackAttachProcess(Process, &ApcState);
			*(volatile BYTE*)Breakpoint->Address;
			WeDebug_cli();
			vmcallinfo.cr3 = WeDebug_readcr3();
			WeDebug_sti();
			boSuccess = current_vmcall(&vmcallinfo);
			KeUnstackDetachProcess(&ApcState);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_DEBUG("[-] Delete hardware breakpoint crashed\r\n");
		}
		ObDereferenceObject(Process);
	}
	return boSuccess;
}

void RemoveHardwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp)
{
	USER_DATA user = Control::GetUserData(userData);

	DWORD* output = (DWORD*)pIrp->AssociatedIrp.SystemBuffer;
	RtlZeroMemory(output, sizeof(DWORD));

	BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
	DecryptData((PVOID)user.pUserData, aucPlainText);

	size_t numElements = user.uSize / sizeof(BREAKPOINT_RECORD);
	for (size_t i = 0; i < numElements; i++)
	{
		PBREAKPOINT_RECORD pInfo = reinterpret_cast<PBREAKPOINT_RECORD>(aucPlainText + i * sizeof(BREAKPOINT_RECORD));
		*output = DeleteHardwareBreakpoint(pInfo) ? 1998 : 520;
		break;
	}
	free_pool(aucPlainText);
}

static bool AddSoftwareBreakpoint(PVT_BREAK_POINT vmcallinfo)
{
	bool boSuccess = false;
	::KAPC_STATE ApcState;
	PEPROCESS Process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)vmcallinfo->pid, &Process);
	if (NT_SUCCESS(status))
	{
		__try
		{
			KeStackAttachProcess(Process, &ApcState);
			*(volatile BYTE*)vmcallinfo->VirtualAddress;
			WeDebug_cli();
			vmcallinfo->cr3 = WeDebug_readcr3();
			WeDebug_sti();
			boSuccess = current_vmcall(vmcallinfo);
			KeUnstackDetachProcess(&ApcState);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_DEBUG("[-] Set software breakpoint crashed\r\n");
		}
		ObDereferenceObject(Process);
	}
	return boSuccess;
}

void SetSoftwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp)
{
	USER_DATA user = Control::GetUserData(userData);

	PVT_BREAK_POINT output = (PVT_BREAK_POINT)pIrp->AssociatedIrp.SystemBuffer;
	RtlZeroMemory(output, sizeof(VT_BREAK_POINT));

	BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
	DecryptData((PVOID)user.pUserData, aucPlainText);

	size_t numElements = user.uSize / sizeof(VT_BREAK_POINT);
	for (size_t i = 0; i < numElements; i++)
	{
		PVT_BREAK_POINT pInfo = reinterpret_cast<PVT_BREAK_POINT>(aucPlainText + i * sizeof(VT_BREAK_POINT));
		if (AddSoftwareBreakpoint(pInfo))
		{
			*output = *pInfo;
		}
		break;
	}
	free_pool(aucPlainText);
}

static bool DeleteSoftwareBreakpoint(PVT_BREAK_POINT vmcallinfo)
{
	bool boSuccess = false;
	::KAPC_STATE ApcState;
	PEPROCESS Process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)vmcallinfo->pid, &Process);
	if (NT_SUCCESS(status))
	{
		__try
		{
			KeStackAttachProcess(Process, &ApcState);
			*(volatile BYTE*)vmcallinfo->VirtualAddress;
			WeDebug_cli();
			vmcallinfo->cr3 = WeDebug_readcr3();
			WeDebug_sti();
			boSuccess = current_vmcall(vmcallinfo);
			KeUnstackDetachProcess(&ApcState);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_DEBUG("[-] Delete software breakpoint crashed\r\n");
		}
		ObDereferenceObject(Process);
	}
	return boSuccess;
}

void RemoveSoftwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp)
{
	USER_DATA user = Control::GetUserData(userData);

	DWORD* output = (DWORD*)pIrp->AssociatedIrp.SystemBuffer;
	RtlZeroMemory(output, sizeof(DWORD));

	BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
	DecryptData((PVOID)user.pUserData, aucPlainText);

	size_t numElements = user.uSize / sizeof(VT_BREAK_POINT);
	for (size_t i = 0; i < numElements; i++)
	{
		PVT_BREAK_POINT pInfo = reinterpret_cast<PVT_BREAK_POINT>(aucPlainText + i * sizeof(VT_BREAK_POINT));
		*output = DeleteSoftwareBreakpoint(pInfo) ? 1998 : 520;
		break;
	}
	free_pool(aucPlainText);
}

static bool GetSoftwareBreakpoint(PVT_BREAK_POINT vmcallinfo)
{
	bool boSuccess = false;
	::KAPC_STATE ApcState;
	PEPROCESS Process = NULL;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)vmcallinfo->pid, &Process);
	if (NT_SUCCESS(status))
	{
		__try
		{
			KeStackAttachProcess(Process, &ApcState);
			*(volatile BYTE*)vmcallinfo->VirtualAddress;
			WeDebug_cli();
			vmcallinfo->cr3 = WeDebug_readcr3();
			WeDebug_sti();
			boSuccess = current_vmcall(vmcallinfo);
			KeUnstackDetachProcess(&ApcState);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_DEBUG("[-] Read software breakpoint crashed\r\n");
		}
		ObDereferenceObject(Process);
	}
	if (boSuccess)
	{
		vmcallinfo->errorCode = 1998;
	}
	return boSuccess;
}

void ReadSoftwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp)
{
	USER_DATA user = Control::GetUserData(userData);

	PVT_BREAK_POINT output = (PVT_BREAK_POINT)pIrp->AssociatedIrp.SystemBuffer;
	RtlZeroMemory(output, sizeof(VT_BREAK_POINT));

	BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
	DecryptData((PVOID)user.pUserData, aucPlainText);

	size_t numElements = user.uSize / sizeof(VT_BREAK_POINT);
	for (size_t i = 0; i < numElements; i++)
	{
		PVT_BREAK_POINT pInfo = reinterpret_cast<PVT_BREAK_POINT>(aucPlainText + i * sizeof(VT_BREAK_POINT));
		if (GetSoftwareBreakpoint(pInfo))
		{
			*output = *pInfo;
		}
		break;
	}
	free_pool(aucPlainText);
}
