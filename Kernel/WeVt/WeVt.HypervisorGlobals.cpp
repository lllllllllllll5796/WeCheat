#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "../../Shared/SharedStruct.h"
#include "WeVt.AsmCallset.h"
#include "WeVt.hypervisor_routines.h"
#include "WeVt.invalid_ept.h"
#include "WeVt.vmcs.h"
#include "WeVt.idt.h"
#include "WeVt.gdt.h"
#include "WeVt.interrupt.h"
#include "WeVt.msr.h"

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
	// selectors for the host GDT
	segment_selector host_cs_selector = { 0, 0, 1 };
	segment_selector host_tr_selector = { 0, 0, 2 };

	//将host的物理地址映射在pml4[255]处
	uint64_t host_physical_memory_pml4_idx = 255;

	// directly access physical memory by using [base + offset]
	// directly access physical memory by using [base + offset]
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

		vmxon_region.revision_id = vmx_basic.vmcs_revision_id;
		vmxon_region.must_be_zero = 0;

		unsigned __int64 vmxon_phys = (unsigned __int64)MmGetPhysicalAddress(&vmxon_region).QuadPart;
		unsigned char vmxon_status = 0;
		__ia32_feature_control_msr feature_msr = { 0 };
		feature_msr.all = __readmsr(IA32_FEATURE_CONTROL);

#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
			"[+] VMXON va=%p pa=0x%llX aligned=%d cr0=0x%llX cr4=0x%llX feature_ctrl=0x%llX lock=%d vmx_outside_smx=%d rev=0x%X",
			&vmxon_region,
			vmxon_phys,
			(vmxon_phys % 0x1000 == 0) ? 1 : 0,
			__readcr0(),
			__readcr4(),
			feature_msr.all,
			(int)feature_msr.lock,
			(int)feature_msr.vmxon_outside_smx,
			vmx_basic.vmcs_revision_id);
#endif

		if (vmxon_phys % 0x1000 != 0)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] VMXON region is not 4KB aligned");
#endif
			return false;
		}

		if (feature_msr.lock != 0 && feature_msr.vmxon_outside_smx == 0)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] IA32_FEATURE_CONTROL locked without VMX outside SMX");
#endif
			return false;
		}

		vmxon_status = __vmx_on(&vmxon_phys);
		if (vmxon_status != 0)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] __vmx_on failed status=%u (1=VMfailInvalid)", (unsigned)vmxon_status);
#endif
			return false;
		}

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
	// directly map physical memory into the host page tables
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
	// initialize the host page tables
	void prepare_host_page_tables() {
		auto& pt = ghv.host_page_tables;
		memset(&pt, 0, sizeof(pt));

		map_physical_memory(pt);

		//先获得kernel system进程的cr3的pml4的物理地址
		PHYSICAL_ADDRESS pml4_address;
		pml4_address.QuadPart = ghv.system_cr3.address_of_page_directory << 12;

		// kernel PML4 address
		// kernel PML4 address
		// 由kernel pml4物理地址得到虚拟线性地址
		auto const guest_pml4 = static_cast<pml4e_64*>(MmGetVirtualForPhysical(pml4_address));

#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] guest_pml4: 0x%llX", guest_pml4->flags);
#endif

		// copy the top half of the System pml4 (a.k.a. the kernel address space)
		// copy the top half of the System pml4 (a.k.a. the kernel address space)
		// 复制system pml4 的后半部分（又称内核地址空间）
		//outDebug("&guest_pml4[256]: %p\n", &guest_pml4[256]);
		memcpy(&pt.pml4[256], &guest_pml4[256], sizeof(pml4e_64) * 256);
	}

	// read MTRR data into a single structure
	// read MTRR data into a single structure
	mtrr_data read_mtrr_data() {
		mtrr_data mtrrs;

		mtrrs.cap.flags = __readmsr(IA32_MTRR_CAPABILITIES);
		mtrrs.def_type.flags = __readmsr(IA32_MTRR_DEF_TYPE);
		mtrrs.var_count = 0;

		for (uint32_t i = 0; i < mtrrs.cap.variable_range_count; ++i) {
			ia32_mtrr_physmask_register mask;
			mask.flags = __readmsr(IA32_MTRR_PHYSMASK0 + i * 2);

			if (!mask.valid)
				continue;

			mtrrs.variable[mtrrs.var_count].mask = mask;
			mtrrs.variable[mtrrs.var_count].base.flags = __readmsr(IA32_MTRR_PHYSBASE0 + i * 2);

			++mtrrs.var_count;
		}

		return mtrrs;
	}

	// calculate the MTRR memory type for a single page
	// calculate the MTRR memory type for a single page
	static uint8_t calc_mtrr_mem_type(mtrr_data const& mtrrs, uint64_t const pfn) {
		if (!mtrrs.def_type.mtrr_enable)
		{
			// MTRRs被禁用这意味着所有的物理内存都将被视为UC
			return MEMORY_TYPE_UNCACHEABLE;
		}

		// fixed range MTRRs
		// fixed range MTRRs
		if (pfn < 0x100 && mtrrs.cap.fixed_range_supported && mtrrs.def_type.fixed_range_mtrr_enable)
		{
			// 如果pfn小于256 且开启了固定范围MTRRs
			// 则将物理内存视为UC类型
			return MEMORY_TYPE_UNCACHEABLE;
		}

		uint8_t curr_mem_type = MEMORY_TYPE_INVALID;

		// variable-range MTRRs
		// variable-range MTRRs
		for (uint32_t i = 0; i < mtrrs.var_count; ++i) {
			auto const base = mtrrs.variable[i].base.page_frame_number;
			auto const mask = mtrrs.variable[i].mask.page_frame_number;


			//Vol.3A[12.11.3]
			//Vol.3A[12.11.3]
			if ((pfn & mask) == (base & mask)) {
				auto const type = static_cast<uint8_t>(mtrrs.variable[i].base.type);

				//判断是否是UC类型，如果是则立即返回
				if (type == MEMORY_TYPE_UNCACHEABLE)
					return MEMORY_TYPE_UNCACHEABLE;

				// this works for WT and WB, which is the only other "defined" overlap scenario
				if (type < curr_mem_type)
					curr_mem_type = type;
			}
		}

		// no MTRR covers the specified address
		//未被 MTRR 映射的地址范围应设置为默认类型
		if (curr_mem_type == MEMORY_TYPE_INVALID)
			return mtrrs.def_type.default_memory_type;

		return curr_mem_type;
	}

	// calculate the MTRR memory type for the given physical memory range
	// calculate the MTRR memory type for the given physical memory range
	uint8_t calc_mtrr_mem_type(mtrr_data const& mtrrs, uint64_t address, uint64_t size) {
		// base address must be on atleast a 4KB boundary
		// base address must be on atleast a 4KB boundary
		address &= ~0xFFFull;

		// minimum range size is 4KB
		// minimum range size is 4KB
		size = (size + 0xFFF) & ~0xFFFull;

		//先将其初始化为无效的内存类型
		uint8_t curr_mem_type = MEMORY_TYPE_INVALID;

		for (uint64_t curr = address; curr < address + size; curr += 0x1000) {
			auto const type = calc_mtrr_mem_type(mtrrs, curr >> 12/*得到pfn*/);

			if (type == MEMORY_TYPE_UNCACHEABLE)
				return type;

			// use the worse memory type between the two
			if (type < curr_mem_type)
				curr_mem_type = type;
		}

		if (curr_mem_type == MEMORY_TYPE_INVALID)
			return MEMORY_TYPE_UNCACHEABLE;

		return curr_mem_type;
	}


	// 将guest虚拟地址转为guest物理地址
	// 将 GVA 转换为 GPA, offset_to_next_page 是下一页的字节数（即可通过 GPA 安全访问以修改 GVA 的字节数）。
	uint64_t gva2gpa(cr3 const guest_cr3, void* const gva, size_t* const offset_to_next_page) {
		if (offset_to_next_page)
			*offset_to_next_page = 0;

		pml4_virtual_address const vaddr = { gva };

		// guest PML4
		// guest PML4
		// 由于我们将所有的物理地址映射在了host pt.pml4[255]开始的地方
		// 由于我们将所有的物理地址映射在了host pt.pml4[255]开始的地方
		auto const pml4 = reinterpret_cast<pml4e_64*>(host_physical_memory_base + (guest_cr3.address_of_page_directory << 12));
		auto const pml4e = pml4[vaddr.pml4_idx];

		//判断该页是否存在
		//当P=1指示表或物理页面已加载到物理内存中
		if (!pml4e.present)
			return 0;

		// guest PDPT
		// guest PDPT
		// 因为vm里的所以地址，对于host来说都是虚拟地址
		// 所以我们在host里仍然是将gpa的地址当做线性地址来解析
		auto const pdpt = reinterpret_cast<pdpte_64*>(host_physical_memory_base + (pml4e.page_frame_number << 12));
		auto const pdpte = pdpt[vaddr.pdpt_idx];

		if (!pdpte.present)
			return 0;

		if (pdpte.large_page) {
			pdpte_1gb_64 pdpte_1gb;
			pdpte_1gb.flags = pdpte.flags;

			auto const offset = (vaddr.pd_idx << 21) + (vaddr.pt_idx << 12) + vaddr.offset;

			// 1GB
			if (offset_to_next_page)
				*offset_to_next_page = 0x40000000 - offset;

			return (pdpte_1gb.page_frame_number << 30) + offset;
		}

		// guest PD
		auto const pd = reinterpret_cast<pde_64*>(host_physical_memory_base + (pdpte.page_frame_number << 12));
		auto const pde = pd[vaddr.pd_idx];

		if (!pde.present)
			return 0;

		if (pde.large_page) {
			pde_2mb_64 pde_2mb;
			pde_2mb.flags = pde.flags;

			auto const offset = (vaddr.pt_idx << 12) + vaddr.offset;

			// 2MB page
			if (offset_to_next_page)
				*offset_to_next_page = 0x200000 - offset;

			return (pde_2mb.page_frame_number << 21) + offset;
		}

		// guest PT
		auto const pt = reinterpret_cast<pte_64*>(host_physical_memory_base + (pde.page_frame_number << 12));
		auto const pte = pt[vaddr.pt_idx];

		if (!pte.present)
			return 0;

		// 4KB page
		if (offset_to_next_page)
			*offset_to_next_page = 0x1000 - vaddr.offset;

		//(pte.page_frame_number << 12) 4KB物理页的起始地址 + offset则得到具体的物理地址
		return (pte.page_frame_number << 12) + vaddr.offset;
	}

	//将GVA转换为GPA
	uint64_t get_physical_address(unsigned __int64 guest_cr3, _In_ PVOID BaseAddress)
	{
		if (!guest_cr3)
		{
			return NULL;
		}

		cr3 tmp_cr3;
		tmp_cr3.flags = guest_cr3;
		return gva2gpa(tmp_cr3, BaseAddress);
	}

	// translate a GVA to an HVA. offset_to_next_page is the number of bytes to
	// the next page (i.e. the number of bytes that can be safely accessed through
	// the HVA in order to modify the GVA.
	void* gva2hva(cr3 const guest_cr3, void* const gva, size_t* const offset_to_next_page) {
		auto const gpa = gva2gpa(guest_cr3, gva, offset_to_next_page);
		if (!gpa)
			return nullptr;
		return host_physical_memory_base + gpa;  //将gpa映射到hva
	}

	// translate a GVA to an HVA. offset_to_next_page is the number of bytes to
	// the next page (i.e. the number of bytes that can be safely accessed through
	// the HVA in order to modify the GVA.
	// translate a GVA to an HVA. offset_to_next_page is the number of bytes to
	void* gva2hva(void* const gva, size_t* const offset_to_next_page) {
		cr3 guest_cr3;
		guest_cr3.flags = vmread(GUEST_CR3);
		return gva2hva(guest_cr3, gva, offset_to_next_page);
	}

	// attempt to read the memory at the specified guest virtual address from root-mode
	size_t read_guest_virtual_memory(cr3 const guest_cr3,
		void* const gva, void* const hva, size_t const size)
	{
		// the GVA that we're reading from
		auto const src = reinterpret_cast<uint8_t*>(gva);

		// the HVA that we're writing to
		// the HVA that we're writing to
		auto const dst = reinterpret_cast<uint8_t*>(hva);

		size_t bytes_read = 0;

		// translate and read 1 page at a time
		while (bytes_read < size) {
			size_t src_remaining = 0;

			// translate the guest virtual address to a host virtual address
			// translate the guest virtual address to a host virtual address
			// 将guest虚拟地址映射到host虚拟地址
			auto const curr_src = gva2hva(guest_cr3, src + bytes_read, &src_remaining);

			// paged out
			if (!curr_src)
				return bytes_read;

			// the maximum allowed size that we can read at once with the translated HVA
			auto const curr_size = MIN(size - bytes_read, src_remaining);

			host_exception_info e = { 0 };
			memcpy_safe(e, dst + bytes_read, curr_src, curr_size);

			// this shouldn't ever happen...
			if (e.exception_occurred) {
				return bytes_read;
			}

			bytes_read += curr_size;
		}

		return bytes_read;
	}

	// attempt to read the memory at the specified guest virtual address from root-mode
	// attempt to read the memory at the specified guest virtual address from root-mode
	size_t read_guest_virtual_memory(void* const gva, void* const hva, size_t const size)
	{
		cr3 guest_cr3;
		guest_cr3.flags = vmread(GUEST_CR3);
		return read_guest_virtual_memory(guest_cr3, gva, hva, size);
	}

	size_t write_guest_virtual_memory(cr3 const guest_cr3,
		void* const gva, void* const hva, size_t const size)
	{
		size_t bytes_read = 0;

		// 这个是gva
		auto const dst = reinterpret_cast<uint8_t*>(gva);

		// 这个是hva
		auto const src = reinterpret_cast<uint8_t*>(hva);

		while (bytes_read < size) {
			size_t dst_remaining = 0;

			// remaining返回页面的剩余字节数
			// 如果跨页curr_dst将会指向下一个页
			auto const curr_dst = gva2hva(guest_cr3, dst + bytes_read, &dst_remaining);

			// this means that the target memory isn't paged in. there's nothing
			// we can do about that since we're not currently in that process's context.
			// this means that the target memory isn't paged in. there's nothing
			if (!curr_dst)
				return bytes_read;


			auto const curr_size = MIN(size - bytes_read, dst_remaining);

			host_exception_info e = { 0 };
			memcpy_safe(e, curr_dst, src + bytes_read, curr_size);

			if (e.exception_occurred) {
				// 这真的不应该发生……永远……
				return bytes_read;
			}

			bytes_read += curr_size;
		}
		return bytes_read;
	}

	// 写入guest中当前进程的虚拟内存
	size_t write_guest_virtual_memory(void* const gva, void* const hva, size_t const size)
	{
		cr3 guest_cr3;
		guest_cr3.flags = vmread(GUEST_CR3);
		return write_guest_virtual_memory(guest_cr3, gva, hva, size);
	}

	//获取空闲的id
	int getIdleWatchID()
	{
		int i;
		for (i = 0; i < EPTWATCHLISTSIZE; i++)
		{
			if (eptWatchList[i].inuse == 0)  //查找没有被使用的位置
			{
				return i;  //找到后返回index
			}
		}
		return -1;
	}

	cr0 read_effective_guest_cr0() {
		// TODO: cache this value
		auto const mask = vmread(CR0_GUEST_HOST_MASK);

		// bits set to 1 in the mask are read from CR0, otherwise from the shadow
		cr0 cr0;
		cr0.flags = (vmread(CR0_READ_SHADOW) & mask) | (vmread(GUEST_CR0) & ~mask);

		return cr0;
	}

	cr4 read_effective_guest_cr4() {
		// TODO: cache this value
		auto const mask = vmread(CR4_GUEST_HOST_MASK);

		// bits set to 1 in the mask are read from CR4, otherwise from the shadow
		cr4 cr4;
		cr4.flags = (vmread(CR4_READ_SHADOW) & mask) | (vmread(GUEST_CR4) & ~mask);

		return cr4;
	}

	vmx_interruptibility_state read_interruptibility_state()
	{
		vmx_interruptibility_state value;
		value.flags = static_cast<uint32_t>(vmread(GUEST_INTERRUPTIBILITY_STATE));
		return value;
	}

	void write_interruptibility_state(vmx_interruptibility_state const value)
	{
		hv::vmwrite(GUEST_INTERRUPTIBILITY_STATE, value.flags);
	}

	// inject an NMI into the guest
	void inject_nmi() {
		//vmentry_interrupt_information interrupt_info;
		//interrupt_info.flags = 0;
		//interrupt_info.vector = nmi;
		//interrupt_info.interruption_type = non_maskable_interrupt;
		//interrupt_info.deliver_error_code = 0;
		//interrupt_info.valid = 1;
		//vmwrite(VM_ENTRY_INTERRUPTION_INFO_FIELD, interrupt_info.flags);
		inject_interruption(EXCEPTION_VECTOR_NMII, INTERRUPT_TYPE_NMI, 0, false);
	}

	// get the KPCR of the current guest (this pointer should stay constant per-vcpu)
	PKPCR current_guest_kpcr() {
		// GS base holds the KPCR when in ring-0
		if (current_guest_cpl() == 0)
			return reinterpret_cast<PKPCR>(vmread(GUEST_GS_BASE));

		// when in ring-3, the GS_SWAP contains the KPCR
		// 在 ring-3 中，GS_SWAP 包含 内核KPCR
		return reinterpret_cast<PKPCR>(__readmsr(IA32_KERNEL_GS_BASE));
	}

	// get the ETHREAD of the current guest
	// 获取guest里的当前线程对象
	size_t current_guest_ethread()
	{
		// KPCR
		auto const kpcr = current_guest_kpcr();

		if (!kpcr)
			return NULL;

		// KPCR::Prcb
		auto const kprcb = reinterpret_cast<uint8_t*>(kpcr) + ghv.kpcr_pcrb_offset;

		// KPCRB::CurrentThread
		size_t current_thread = NULL;
		read_guest_virtual_memory(ghv.system_cr3,
			kprcb + ghv.kprcb_current_thread_offset,
			&current_thread,
			sizeof(current_thread));

		return current_thread;
	}

	PCLIENT_ID GuestCurrentThreadCid()
	{
		size_t Thread = hv::current_guest_ethread();
		size_t ptr_Cid = Thread + Global::ethread_offset::Cid;
		return (PCLIENT_ID)ptr_Cid;
	}

	bool get_breakpoint_detected(__vcpu* vcpu, PBREAKPOINT_DETECTED vmcallinfo)
	{
		BREAKPOINT_DETECTED tmp_vmcallinfo = { 0 };

		if (sizeof(BREAKPOINT_DETECTED) != hv::read_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(BREAKPOINT_DETECTED)))
		{
			return false;
		}

		if (vcpu->Cid.UniqueThread == (HANDLE)tmp_vmcallinfo.Cid.UniqueThread)
		{
			tmp_vmcallinfo.breakpoint_detected = vcpu->breakpoint_detected;

			if (sizeof(BREAKPOINT_DETECTED) != hv::write_guest_virtual_memory(vmcallinfo, &tmp_vmcallinfo, sizeof(BREAKPOINT_DETECTED)))
			{
				return false;
			}

			vcpu->breakpoint_detected = NULL;
			vcpu->Cid = { 0 };
			return true;
		}
		return false;
	}


	//向guest注入#DB事件
	void inject_single_step(__vcpu* vcpu)
	{
		//guest的模式如果是内核就不注入#DB
		int kernelmode = hv::get_guest_cpl() == 0;
		if (!kernelmode)
		{
			PCLIENT_ID Cid = GuestCurrentThreadCid();
			vcpu->Cid.UniqueThread = Cid->UniqueThread;  //记录当前guest的线程id
			hv::inject_interruption(EXCEPTION_VECTOR_SINGLE_STEP, INTERRUPT_TYPE_HARDWARE_EXCEPTION, 0, false);
		}
	}

	// set the memory type in every EPT paging structure to the specified value
	// 将每个 EPT 分页结构中的内存类型设置为指定值将每个 EPT 分页结构中的内存类型设置为指定值
	void set_ept_memory_type(__ept_state& ept_state, uint8_t const memory_type)
	{
		sLog("\n");
		for (size_t i = 0; i < EPT_PD_COUNT; ++i)
		{
			for (size_t j = 0; j < 512; ++j)
			{
				auto& pde = ept_state.ept_page_table->pml2[i][j];

				// 2MB large page
				// 2MB 大页面
				if (pde.page_directory_entry.large_page)
				{
					pde.page_directory_entry.memory_type = memory_type;
				}
				else
				{
					// PDE 指向一个 PT
					auto const pt = reinterpret_cast<ept_pte*>(host_physical_memory_base + (pde.large_page.page_frame_number << 12));

					// update the memory type for every PTE
					// 更新每个 PTE 的内存类型
					for (size_t k = 0; k < 512; ++k)
						pt[k].memory_type = memory_type;
				}
			}
		}
	}

	// update the memory types in the EPT paging structures based on the MTRRs.
	// 根据 MTRR 更新 EPT 分页结构中的内存类型。
	// this function should only be called from root-mode during vmx-operation.
	// 此函数应仅在 vmx-operation 期间从host调用。
	void update_ept_memory_type(__ept_state& ept_state)
	{
		sLog("\n");
		// TODO: completely virtualize the guest MTRRs
		// 完全虚拟化guest MTRR
		auto const mtrrs = read_mtrr_data();

		for (size_t i = 0; i < EPT_PD_COUNT; ++i) {
			for (size_t j = 0; j < 512; ++j) {
				auto& pde = ept_state.ept_page_table->pml2[i][j];

				// 2MB large page
				if (pde.page_directory_entry.large_page) {
					// update the memory type for this PDE
					pde.page_directory_entry.memory_type = calc_mtrr_mem_type(mtrrs,
						pde.page_directory_entry.page_frame_number << 21, 0x1000 << 9);
				}
				// PDE points to a PT
				else {
					auto const pt = reinterpret_cast<ept_pte*>(host_physical_memory_base + (pde.large_page.page_frame_number << 12));

					// update the memory type for every PTE
					for (size_t k = 0; k < 512; ++k) {
						pt[k].memory_type = calc_mtrr_mem_type(mtrrs, pt[k].page_frame_number << 12, 0x1000);
					}
				}
			}
		}
	}

	//读取guest通用寄存器
	uint64_t read_guest_gpr(guest_context const* const ctx, uint64_t const gpr_idx)
	{
		if (gpr_idx == VMX_EXIT_QUALIFICATION_GENREG_RSP)
			return hv::vmread(GUEST_RSP);
		return ctx->gpr[gpr_idx];
	}

	//写guest通用寄存器
	void write_guest_gpr(guest_context* const ctx, uint64_t const gpr_idx, uint64_t const value)
	{
		if (gpr_idx == VMX_EXIT_QUALIFICATION_GENREG_RSP)
			vmwrite(GUEST_RSP, value);
		else
			ctx->gpr[gpr_idx] = value;
	}

}
