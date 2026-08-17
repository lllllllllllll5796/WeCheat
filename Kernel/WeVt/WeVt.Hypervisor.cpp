#include "WeVt.pch.h"
#include "WeVt.Hypervisor.h"
#include "WeVt.AsmCallset.h"
#include "WeVt.vmcall_reason.h"
#include "WeVt.ntapi.h"

namespace hvgt
{
	struct HookFunctionArgs
	{
		void* target_address;
		void* proxy_function;
		void** origin_function;
		unsigned __int64 current_cr3;
		volatile SHORT statuses;
	};

	struct UnHookFunctionArgs
	{
		bool unhook_all_functions;
		void* function_to_unhook;
		unsigned __int64 current_cr3;
		volatile SHORT statuses;
	};

	void broadcast_vmoff(KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		UNREFERENCED_PARAMETER(Dpc);
		UNREFERENCED_PARAMETER(DeferredContext);

		__vm_call(VT_VMCALL_VMXOFF, 0, 0, 0);
		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_invept_all_contexts(KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		UNREFERENCED_PARAMETER(Dpc);
		UNREFERENCED_PARAMETER(DeferredContext);

		__vm_call(VT_VMCALL_INVEPT_CONTEXT, true, 0, 0);
		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_invept_single_context(KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		UNREFERENCED_PARAMETER(Dpc);
		UNREFERENCED_PARAMETER(DeferredContext);

		__vm_call(VT_VMCALL_INVEPT_CONTEXT, false, 0, 0);
		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_unhook_function(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<UnHookFunctionArgs*>(DeferredContext);

		if (__vm_call(VT_VMCALL_EPT_UNHOOK_FUNCTION, args->unhook_all_functions,
			(unsigned __int64)args->function_to_unhook, args->current_cr3))
		{
			InterlockedIncrement16(&args->statuses);
		}

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_hook_function(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<HookFunctionArgs*>(DeferredContext);

		if (__vm_call_ex(VT_VMCALL_EPT_RIP_HOOK, (unsigned __int64)args->target_address,
			(unsigned __int64)args->proxy_function, (unsigned __int64)args->origin_function, args->current_cr3, 0, 0, 0, 0, 0))
		{
			InterlockedIncrement16(&args->statuses);
		}

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_get_hide_software_breakpoint(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<HookFunctionArgs*>(DeferredContext);

		if (__vm_call_ex(VT_VMCALL_READ_SOFTWARE_BREAKPOINT, (unsigned __int64)args->target_address,
			(unsigned __int64)args->proxy_function, (unsigned __int64)args->origin_function, args->current_cr3, 0, 0, 0, 0, 0))
		{
			InterlockedIncrement16(&args->statuses);
		}

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_set_hide_software_breakpoint(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<HookFunctionArgs*>(DeferredContext);

		if (__vm_call_ex(VT_VMCALL_HIDE_SOFTWARE_BREAKPOINT, (unsigned __int64)args->target_address,
			(unsigned __int64)args->proxy_function, (unsigned __int64)args->origin_function, args->current_cr3, 0, 0, 0, 0, 0))
		{
			InterlockedIncrement16(&args->statuses);
		}

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_vmcall(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<HookFunctionArgs*>(DeferredContext);
		unsigned __int64 command = *reinterpret_cast<unsigned __int64*>(args->target_address);

		if (__vm_call(command, (unsigned __int64)args->target_address, 0, 0))
		{
			InterlockedIncrement16(&args->statuses);
		}

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_test_vmcall(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto statuses = reinterpret_cast<volatile SHORT*>(DeferredContext);

		if (__vm_call(VT_VMCALL_TEST, 0, 0, 0))
		{
			InterlockedIncrement16(statuses);
		}

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void vmoff(unsigned __int32 processor_count)
	{
		NT_ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

		for (unsigned int iter = 0; iter < processor_count; iter++)
		{
			auto const orig_affinity = KeSetSystemAffinityThreadEx(1ull << iter);
			__try
			{
				__vm_call(VT_VMCALL_VMXOFF, 0, 0, 0);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
			KeRevertToUserAffinityThreadEx(orig_affinity);
		}
	}

	void invept(bool invept_all)
	{
		if (invept_all == true)
		{
			KeGenericCallDpc(broadcast_invept_all_contexts, NULL);
		}
		else
		{
			KeGenericCallDpc(broadcast_invept_single_context, NULL);
		}
	}

	void hypervisor_visible(bool value)
	{
		if (value == true)
		{
			__vm_call(VT_VMCALL_UNHIDE_HV_PRESENCE, 0, 0, 0);
		}
		else
		{
			__vm_call(VT_VMCALL_HIDE_HV_PRESENCE, 0, 0, 0);
		}
	}

	bool ept_unhook()
	{
		UnHookFunctionArgs args{ true, nullptr, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_unhook_function, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool ept_unhook(void* function_address)
	{
		UnHookFunctionArgs args{ false, function_address, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_unhook_function, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool test_vmcall()
	{
		volatile SHORT statuses{};
		KeGenericCallDpc(broadcast_test_vmcall, (PVOID)&statuses);

		return static_cast<ULONG>(statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool hook_function(void* target_address, void* proxy_function, void** origin_function)
	{
		HookFunctionArgs args{ target_address, proxy_function, origin_function, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_hook_function, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool vmcall(void* vmcallinfo)
	{
		HookFunctionArgs args{};
		args.target_address = vmcallinfo;
		KeGenericCallDpc(broadcast_vmcall, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool get_hide_software_breakpoint(void* target_address, void* buffer, unsigned __int64 buffer_size)
	{
		HookFunctionArgs args{ target_address, buffer, (void**)buffer_size, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_get_hide_software_breakpoint, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool set_hide_software_breakpoint(void* vmcallinfo)
	{
		HookFunctionArgs args{ vmcallinfo, nullptr, nullptr, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_set_hide_software_breakpoint, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}
}
