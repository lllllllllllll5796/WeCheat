#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "WeVt.AsmCallset.h"
#include "WeVt.hypervisor_routines.h"
#include "WeVt.invalid_ept.h"
#include "WeVt.vmcs.h"
#include "WeVt.idt.h"
#include "WeVt.gdt.h"

#include "WeVt.Trace.h"
#include "WeVt.HypervisorGlobals.tmh"

__pseudo_descriptor64 g_gdtr = { 0 };
__pseudo_descriptor64 g_idtr = { 0 };
unsigned __int64 g_guest_cr0;
unsigned __int64 g_guest_cr3;
unsigned __int64 g_guest_cr4;
__vmm_context g_vmm_context;
uint16_t guest_vpid = 1;
int eptWatchListSize;
int eptWatchListPos;
volatile long eptWatchList_lock = 0;

EPTWatchEntry eptWatchList[EPTWATCHLISTSIZE];

namespace hv
{
	// selectors for the host GDT
	// host的gdt选择器
	segment_selector host_cs_selector = { 0, 0, 1 };
	segment_selector host_tr_selector = { 0, 0, 2 };

	//将host的物理地址映射在pml4[255]处
	uint64_t host_physical_memory_pml4_idx = 255;

	// directly access physical memory by using [base + offset]
	// 指示有效的4级分页从pml4[255]开始
	uint8_t* host_physical_memory_base = reinterpret_cast<uint8_t*>((uint64_t)255 << (9 + 9 + 9 + 12));

	hypervisor ghv;

	ia32_vmx_procbased_ctls_register read_ctrl_proc_based() 
	{
		ia32_vmx_procbased_ctls_register value;
		value.flags = vmread(PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROL);
		return value;
	}

	void write_ctrl_proc_based(ia32_vmx_procbased_ctls_register const value) 
	{
		vmwrite(PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROL, value.flags);
	}

	void InitGlobalVariables()
	{
		g_guest_cr3 = __readcr3();
		__sgdt(&g_gdtr);                                 // 将当前逻辑处理器的gdt存储在全局变量g_gdtr中
		__sidt(&g_idtr);                                 // 将当前逻辑处理器的idt存储在全局变量g_idtr中
		g_guest_cr0 = __readcr0();
		g_guest_cr4 = __readcr4();

#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] g_guest_cr3 = 0x%llX g_guest_cr0 = 0x%llX g_guest_cr4 = 0x%llX", g_guest_cr3, g_guest_cr0, g_guest_cr4);
#endif
	}

	bool enter_vmx_operation(vmxon& vmxon_region)
	{
		ia32_vmx_basic_register vmx_basic;
		vmx_basic.flags = __readmsr(IA32_VMX_BASIC);

		// 3.24.11.5
		vmxon_region.revision_id = vmx_basic.vmcs_revision_id;
		vmxon_region.must_be_zero = 0;

		auto vmxon_phys = MmGetPhysicalAddress(&vmxon_region).QuadPart;
		NT_ASSERT(vmxon_phys % 0x1000 == 0);

		// enter vmx operation
		if (!vmx_on(vmxon_phys)) {
			return false;
		}

		// 3.28.3.3.4
		invept_all_contexts_func();

		return true;
	}

	bool load_vmcs_pointer(vmcs& vmcs_region) 
	{
		ia32_vmx_basic_register vmx_basic;
		vmx_basic.flags = __readmsr(IA32_VMX_BASIC);

		// 3.24.2
		vmcs_region.revision_id = vmx_basic.vmcs_revision_id;
		vmcs_region.shadow_vmcs_indicator = 0;

		auto vmcs_phys = MmGetPhysicalAddress(&vmcs_region).QuadPart;
		NT_ASSERT(vmcs_phys % 0x1000 == 0);

		if (!hv::vmx_vmclear(vmcs_phys)) {
			return false;
		}

		if (!hv::vmx_vmptrld(vmcs_phys)) {
			return false;
		}

		return true;
	}

	//创建host的idt和gdt
	void prepare_external_structures(__vcpu* const vcpu) 
	{
		memset(&vcpu->msr_bitmap, 0, sizeof(vcpu->msr_bitmap));
		//enable_exit_for_msr_read(vcpu->msr_bitmap, IA32_FEATURE_CONTROL, true);

		//enable_mtrr_exiting(vcpu);

		// we don't care about anything that's in the TSS
		memset(&vcpu->host_tss, 0, sizeof(vcpu->host_tss));

		prepare_host_idt(vcpu->host_idt);
		prepare_host_gdt(vcpu->host_gdt, &vcpu->host_tss);

		//prepare_ept(vcpu->ept);
	}

	// directly map physical memory into the host page tables
	// 这里只映射512GB的内存
	void map_physical_memory(host_page_tables& pt) {
		auto& pml4e = pt.pml4[host_physical_memory_pml4_idx];
		pml4e.flags = 0;
		pml4e.present = 1;
		pml4e.write = 1;
		pml4e.supervisor = 0;
		pml4e.page_level_write_through = 0;
		pml4e.page_level_cache_disable = 0;
		pml4e.accessed = 0;
		pml4e.execute_disable = 0;
		pml4e.page_frame_number = MmGetPhysicalAddress(&pt.phys_pdpt).QuadPart >> 12;

		for (uint64_t i = 0; i < HOST_PHYSICAL_MEMORY_PD_COUNT; ++i) {
			auto& pdpte = pt.phys_pdpt[i];
			pdpte.flags = 0;
			pdpte.present = 1;
			pdpte.write = 1;
			pdpte.supervisor = 0;
			pdpte.page_level_write_through = 0;
			pdpte.page_level_cache_disable = 0;
			pdpte.accessed = 0;
			pdpte.execute_disable = 0;
			pdpte.page_frame_number = MmGetPhysicalAddress(&pt.phys_pds[i]).QuadPart >> 12;

			for (uint64_t j = 0; j < 512; ++j) {
				auto& pde = pt.phys_pds[i][j];
				pde.flags = 0;
				pde.present = 1;
				pde.write = 1;
				pde.supervisor = 0;
				pde.page_level_write_through = 0;
				pde.page_level_cache_disable = 0;
				pde.accessed = 0;
				pde.dirty = 0;
				pde.large_page = 1;  //启用2mb 大页面
				pde.global = 0;
				pde.pat = 0;
				pde.execute_disable = 0;
				pde.page_frame_number = (i << 9) + j; //设置pfn 页帧号从0开始
			}
		}
	}

	// initialize the host page tables
	// 初始化host页表
	void prepare_host_page_tables() {
		auto& pt = ghv.host_page_tables;
		memset(&pt, 0, sizeof(pt));

		map_physical_memory(pt);

		//先获得kernel system进程的cr3的pml4的物理地址
		PHYSICAL_ADDRESS pml4_address;
		pml4_address.QuadPart = ghv.system_cr3.address_of_page_directory << 12;

		// kernel PML4 address
		// 由kernel pml4物理地址得到虚拟线性地址
		// 因为memcpy这些函数是操作虚拟地址的
		auto const guest_pml4 = static_cast<pml4e_64*>(MmGetVirtualForPhysical(pml4_address));

#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] guest_pml4: 0x%llX", guest_pml4->flags);
#endif

		// copy the top half of the System pml4 (a.k.a. the kernel address space)
		// 复制system pml4 的后半部分（又称内核地址空间）
		// 将256左移39位得到0x800000000000
		//outDebug("&guest_pml4[256]: %p\n", &guest_pml4[256]);
		memcpy(&pt.pml4[256], &guest_pml4[256], sizeof(pml4e_64) * 256);
	}
}