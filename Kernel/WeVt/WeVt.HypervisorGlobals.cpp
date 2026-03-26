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

	// read MTRR data into a single structure
	// 获取MTRR配置信息
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
	// 计算单个页面的 MTRR 内存类型
	static uint8_t calc_mtrr_mem_type(mtrr_data const& mtrrs, uint64_t const pfn) {
		if (!mtrrs.def_type.mtrr_enable)
		{
			// MTRRs被禁用这意味着所有的物理内存都将被视为UC
			return MEMORY_TYPE_UNCACHEABLE;
		}

		// fixed range MTRRs
		// 固定范围MTRRs
		if (pfn < 0x100 && mtrrs.cap.fixed_range_supported && mtrrs.def_type.fixed_range_mtrr_enable)
		{
			// 如果pfn小于256 且开启了固定范围MTRRs
			// 则将物理内存视为UC类型
			return MEMORY_TYPE_UNCACHEABLE;
		}

		uint8_t curr_mem_type = MEMORY_TYPE_INVALID;

		// variable-range MTRRs
		// 可变范围MTRRs
		for (uint32_t i = 0; i < mtrrs.var_count; ++i) {
			auto const base = mtrrs.variable[i].base.page_frame_number;
			auto const mask = mtrrs.variable[i].mask.page_frame_number;


			//Vol.3A[12.11.3]
			//范围内的任何地址与mask进行 按位与 运算时，它将返回base与mask进行 按位与 运算时相同的值。
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
	// 计算给定物理内存范围的 MTRR 内存类型
	uint8_t calc_mtrr_mem_type(mtrr_data const& mtrrs, uint64_t address, uint64_t size) {
		// base address must be on atleast a 4KB boundary
		// 基地址必须至少位于 4KB 边界上
		address &= ~0xFFFull;

		// minimum range size is 4KB
		// 最小范围大小为 4KB
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
		// 由于我们将所有的物理地址映射在了host pt.pml4[255]开始的地方
		// 故我们需要控制GPA pml4_idx从host pml4[255]处开始
		// 从host pt.pml4[255]处开始
		auto const pml4 = reinterpret_cast<pml4e_64*>(host_physical_memory_base + (guest_cr3.address_of_page_directory << 12));
		auto const pml4e = pml4[vaddr.pml4_idx];

		//判断该页是否存在
		//当P=1指示表或物理页面已加载到物理内存中
		if (!pml4e.present)
			return 0;

		// guest PDPT
		// 因为vm里的所以地址，对于host来说都是虚拟地址
		// 所以我们在host里仍然是将gpa的地址当做线性地址来解析
		// 我们仍需要将线性地址的pml4_idx从pml4[255]处开始
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
	// 将 GVA 翻译为 HVA。offset_to_next_page 是下一页的字节数（即可通过 HVA 安全访问以修改 GVA 的字节数）
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
		// 这个是hva
		auto const dst = reinterpret_cast<uint8_t*>(hva);

		size_t bytes_read = 0;

		// translate and read 1 page at a time
		while (bytes_read < size) {
			size_t src_remaining = 0;

			// translate the guest virtual address to a host virtual address
			// 将guest虚拟地址映射到host虚拟地址
			// 如果跨页curr_src将会指向下一个页
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
	// 读取guest中当前进程的虚拟内存
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
			// 这意味着目标内存未被调入页。我们对此无能为力，因为我们目前不在该进程的上下文中。
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



}