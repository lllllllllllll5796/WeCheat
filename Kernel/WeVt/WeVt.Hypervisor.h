#pragma once

namespace hvgt
{
	/// <summary>
	/// Turn off virtual machine
	/// </summary>
	void vmoff(unsigned __int32 processor_count);

	/// <summary>
	/// Invalidates mappings in the translation lookaside buffers (TLBs)
	/// and paging-structure caches that were derived from extended page tables (EPT)
	/// </summary>
	/// <param name="invept_all"> If true invalidates all contexts, otherwise invalidate only single context </param>
	void invept(bool invept_all);

	/// <summary>
	/// Set/Unset presence of hypervisor
	/// </summary>
	/// <param name="value"> If false, hypervisor is not visible via cpuid interface; if true, it becomes visible </param>
	void hypervisor_visible(bool value);

	/// <summary>
	/// Unhook all pages and invalidate tlb
	/// </summary>
	bool ept_unhook();

	/// <summary>
	/// Unhook single page and invalidate tlb
	/// </summary>
	bool ept_unhook(void* function_address);

	/// <summary>
	/// Check if we can communicate with hypervisor
	/// </summary>
	bool test_vmcall();
}
