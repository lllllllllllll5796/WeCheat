#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "../../Shared/SharedStruct.h"
#include "WeVt.vmcall_handler.h"
#include "WeVt.vmcall_reason.h"
#include "WeVt.interrupt.h"
#include "WeVt.invalid_ept.h"
#include "WeVt.msr.h"
#include "WeVt.ept.h"
#include "WeVt.AsmCallset.h"
#include "WeVt.hypervisor_routines.h"
#include "WeVt.vmexit_handler.h"
#include "WeVt.vmcs.h"

namespace hv
{
	bool get_breakpoint_detected(__vcpu* vcpu, PBREAKPOINT_DETECTED vmcallinfo);
}

void restore_segment_registers()
{
	__writemsr(IA32_FS_BASE, hv::vmread(GUEST_FS_BASE));
	__writemsr(IA32_GS_BASE, hv::vmread(GUEST_GS_BASE));
	__reload_gdtr(hv::vmread(GUEST_GDTR_BASE), hv::vmread(GUEST_GDTR_LIMIT));
	__reload_idtr(hv::vmread(GUEST_IDTR_BASE), hv::vmread(GUEST_IDTR_LIMIT));
}

void call_vmxoff(__vcpu* vcpu)
{
	vcpu->vmx_off_state.vmx_off_executed = true;
}

static bool InitOffset(PWINDOWS_STRUCT vmcallinfo)
{
	WINDOWS_STRUCT tmp_vmcallinfo = { 0 };

	if (sizeof(WINDOWS_STRUCT) != hv::read_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(WINDOWS_STRUCT)))
	{
		return false;
	}

	hv::ghv.kpcr_pcrb_offset = 0x180;
	hv::ghv.kprcb_current_thread_offset = 0x8;
	Global::ethread_offset::Cid = tmp_vmcallinfo.ethread_offset_Cid;
	return true;
}

static bool SetBreakpoint(PVT_BREAK_POINT vmcallinfo, unsigned __int64 Type)
{
	int errorCode = 0;
	VT_BREAK_POINT tmp_vmcallinfo = { 0 };

	if (sizeof(VT_BREAK_POINT) != hv::read_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(VT_BREAK_POINT)))
	{
		return false;
	}

	int outID = -1;
	if (ept::ept_watch_activate(tmp_vmcallinfo, Type, &outID, errorCode))
	{
		tmp_vmcallinfo.watchid = outID;
		if (sizeof(VT_BREAK_POINT) != hv::write_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(VT_BREAK_POINT)))
		{
			return false;
		}
		return true;
	}

	tmp_vmcallinfo.errorCode = errorCode;
	hv::write_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(VT_BREAK_POINT));
	return false;
}

static bool RemoveBreakpoint(PVT_BREAK_POINT vmcallinfo)
{
	VT_BREAK_POINT tmp_vmcallinfo = { 0 };

	if (sizeof(VT_BREAK_POINT) != hv::read_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(VT_BREAK_POINT)))
	{
		return false;
	}

	return ept::ept_watch_deactivate(tmp_vmcallinfo, tmp_vmcallinfo.watchid) == 0;
}

void vmexit_vmcall_handler(__vcpu* vcpu)
{
	bool status = true;
	unsigned __int64 vmcall_reason = 0;
	unsigned __int64 vmcall_parameter1 = 0;
	unsigned __int64 vmcall_parameter2 = 0;
	unsigned __int64 vmcall_parameter3 = 0;
	unsigned __int64 vmcall_parameter4 = 0;

	if ((vcpu->vmexit_info.guest_registers->rax != VMCALL_IDENTIFIER) &&
		(vcpu->vmexit_info.guest_registers->eax != VMCALL_IDENTIFIER2))
	{
		if (ept::handler_vmcall_rip(*vcpu->ept_state))
		{
			return;
		}

		hv::inject_interruption(EXCEPTION_VECTOR_UNDEFINED_OPCODE, INTERRUPT_TYPE_HARDWARE_EXCEPTION, 0, false);
		return;
	}

	if (vcpu->vmexit_info.guest_registers->eax == VMCALL_IDENTIFIER2)
	{
		vmcall_reason = vcpu->vmexit_info.guest_registers->ecx;
		vmcall_parameter1 = vcpu->vmexit_info.guest_registers->edx;
	}
	else
	{
		vmcall_reason = vcpu->vmexit_info.guest_registers->rcx;
		vmcall_parameter1 = vcpu->vmexit_info.guest_registers->rdx;
		vmcall_parameter2 = vcpu->vmexit_info.guest_registers->r8;
		vmcall_parameter3 = vcpu->vmexit_info.guest_registers->r9;
		vmcall_parameter4 = vcpu->vmexit_info.guest_registers->r10;
	}

	switch (vmcall_reason)
	{
	case VT_VMCALL_TEST:
	{
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_VMXOFF:
	{
		call_vmxoff(vcpu);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_INVEPT_CONTEXT:
	{
		if (vmcall_parameter1 != 0)
		{
			invept_all_contexts_func();
		}
		else
		{
			invept_single_context_func((*vcpu->ept_state).ept_pointer->all);
		}

		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_DUMP_POOL_MANAGER:
	{
		pool_manager::dump_pools_info();
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_DUMP_VMCS_STATE:
	{
		hv::dump_vmcs();
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_HIDE_HV_PRESENCE:
	{
		g_vmm_context.hv_presence = false;
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_UNHIDE_HV_PRESENCE:
	{
		g_vmm_context.hv_presence = true;
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_EPT_CC_HOOK:
	{
		status = ept::cc_hook_function(
			*vcpu->ept_state,
			(void*)vmcall_parameter1,
			(void*)vmcall_parameter2,
			(void**)vmcall_parameter3);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_EPT_INT1_HOOK:
	{
		status = ept::int1_hook_function(
			*vcpu->ept_state,
			(void*)vmcall_parameter1,
			(void*)vmcall_parameter2,
			(void**)vmcall_parameter3);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_EPT_RIP_HOOK:
	{
		status = ept::vmcall_hook_function(
			*vcpu->ept_state,
			(void*)vmcall_parameter1,
			(void*)vmcall_parameter2,
			(void**)vmcall_parameter3,
			vmcall_parameter4);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_EPT_HOOK_FUNCTION:
	{
		status = ept::hook_function(
			*vcpu->ept_state,
			(void*)vmcall_parameter1,
			(void*)vmcall_parameter2,
			(void**)vmcall_parameter3);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_EPT_UNHOOK_FUNCTION:
	{
		if (vmcall_parameter1 != 0)
		{
			ept::unhook_all_functions(*vcpu->ept_state);
		}
		else
		{
			status = ept::unhook_function(*vcpu->ept_state, vmcall_parameter2);
		}

		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_HIDE_SOFTWARE_BREAKPOINT:
	{
		status = ept::set_hide_software_breakpoint((PVT_BREAK_POINT)vmcall_parameter1);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_READ_SOFTWARE_BREAKPOINT:
	{
		status = ept::get_hide_software_breakpoint(*vcpu->ept_state, (PVT_BREAK_POINT)vmcall_parameter1);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_READ_EPT_FAKE_PAGE_MEMORY:
	{
		status = false;
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_WATCH_WRITES:
	{
		status = SetBreakpoint((PVT_BREAK_POINT)vmcall_parameter1, EPTW_WRITE);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_WATCH_READS:
	{
		status = SetBreakpoint((PVT_BREAK_POINT)vmcall_parameter1, EPTW_READWRITE);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_WATCH_EXECUTES:
	{
		status = SetBreakpoint((PVT_BREAK_POINT)vmcall_parameter1, EPTW_EXECUTE);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_WATCH_DELETE:
	{
		status = RemoveBreakpoint((PVT_BREAK_POINT)vmcall_parameter1);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_GET_BREAKPOINT:
	{
		status = hv::get_breakpoint_detected(vcpu, (PBREAKPOINT_DETECTED)vmcall_parameter1);
		adjust_rip(vcpu);
		break;
	}
	case VT_VMCALL_INIT_OFFSET:
	{
		status = InitOffset((PWINDOWS_STRUCT)vmcall_parameter1);
		adjust_rip(vcpu);
		break;
	}
	default:
	{
		status = false;
		adjust_rip(vcpu);
		break;
	}
	}

	vcpu->vmexit_info.guest_registers->rax = status;
}
