#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "WeVt.vmm.h"
#include "WeVt.cpuid.h"
#include "../../Shared/SharedStruct.h"
#include "WeVt.ept.h"
#include "WeVt.crx.h"
#include "WeVt.msr.h"
#include "WeVt.vmcs.h"
#include "WeVt.hypervisor_routines.h"
#include "WeVt.ntapi.h"
#include "WeVt.AllocateMem.h"

#include "WeVt.Trace.h"
#include "WeVt.vmm.tmh"

void free_vmm_context()
{
	if (g_vmm_context.pool_manager != nullptr)
	{
		pool_manager::uninitialize();
		free_pool(g_vmm_context.pool_manager);
		g_vmm_context.pool_manager = nullptr;
	}

	if (g_vmm_context.vcpu != nullptr)
	{
		for (unsigned int i = 0; i < g_vmm_context.processor_count; i++)
		{
			if (g_vmm_context.vcpu[i].vcpu_bitmaps.io_bitmap_a != nullptr)
			{
				free_aligned_pool(g_vmm_context.vcpu[i].vcpu_bitmaps.io_bitmap_a);
				g_vmm_context.vcpu[i].vcpu_bitmaps.io_bitmap_a = nullptr;
			}

			if (g_vmm_context.vcpu[i].vcpu_bitmaps.io_bitmap_b != nullptr)
			{
				free_aligned_pool(g_vmm_context.vcpu[i].vcpu_bitmaps.io_bitmap_b);
				g_vmm_context.vcpu[i].vcpu_bitmaps.io_bitmap_b = nullptr;
			}

			if (g_vmm_context.vcpu[i].ept_state != nullptr)
			{
				if (g_vmm_context.vcpu[i].ept_state->ept_pointer != nullptr)
				{
					free_aligned_pool(g_vmm_context.vcpu[i].ept_state->ept_pointer);
					g_vmm_context.vcpu[i].ept_state->ept_pointer = nullptr;
				}

				if (g_vmm_context.vcpu[i].ept_state->ept_page_table != nullptr)
				{
					free_aligned_pool(g_vmm_context.vcpu[i].ept_state->ept_page_table);
					g_vmm_context.vcpu[i].ept_state->ept_page_table = nullptr;
				}

				free_pool(g_vmm_context.vcpu[i].ept_state);
				g_vmm_context.vcpu[i].ept_state = nullptr;
			}
		}

		free_aligned_pool(g_vmm_context.vcpu);
		g_vmm_context.vcpu = nullptr;
	}

	g_vmm_context.processor_count = 0;
	g_vmm_context.hv_presence = false;
}


//分配g_vmm_context上下文
bool allocate_vmm_context()
{
	__cpuid_info cpuid_reg = { 0 };

	//
	// Allocate virtual cpu context for every logical core
	// 为每个逻辑处理器分配虚拟 CPU 上下文
	//
	//
	//g_vmm_context.processor_count = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	g_vmm_context.processor_count = KeQueryActiveProcessorCount(NULL);
	auto const arr_size = sizeof(__vcpu) * g_vmm_context.processor_count;
	g_vmm_context.vcpu = allocate_aligned_pool<__vcpu*>(arr_size);
	if (g_vmm_context.vcpu == nullptr)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] vcpu_table could not be allocated");
#endif
		return false;
	}
	RtlSecureZeroMemory(g_vmm_context.vcpu, arr_size);
#if ENABLE_TRACE
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
		"[+] vcpu array va=%p size=0x%llX count=%u alignok=%d",
		g_vmm_context.vcpu,
		(unsigned __int64)arr_size,
		g_vmm_context.processor_count,
		(((ULONG_PTR)g_vmm_context.vcpu) % 0x1000 == 0) ? 1 : 0);
#endif

	//
	// Build mtrr map for physcial memory caching informations
	// 构建 mtrr 映射来存储物理内存缓存信息
	//
	//
	ept::build_mtrr_map();

	//提前在guest里分配了内存
	if (pool_manager::initialize() == false)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] pool_manager::initialize() error");
#endif
		return false;
	}

	for (unsigned int iter = 0; iter < g_vmm_context.processor_count; iter++)
	{
		if (init_vcpu(&g_vmm_context.vcpu[iter]) == false)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] init_vcpu error");
#endif
			return false;
		}
	}

	g_vmm_context.hv_presence = true;

	__cpuid((int*)&cpuid_reg.eax, 0);
	g_vmm_context.highest_basic_leaf = cpuid_reg.eax;

	//创建host页表
	//将所有物理内存映射到我们的地址空间
	create_host_page_tables();

	return true;
}

//调节控制寄存器 cr4 cr0来启用vmx模式
void adjust_control_registers()
{
	__cr4 cr4;
	__cr0 cr0;
	__cr_fixed cr_fixed;

	_disable();
	cr_fixed.all = __readmsr(IA32_VMX_CR0_FIXED0);
	cr0.all = __readcr0();
	cr0.all |= cr_fixed.split.low;
	cr_fixed.all = __readmsr(IA32_VMX_CR0_FIXED1);
	cr0.all &= cr_fixed.split.low;
	__writecr0(cr0.all);
	cr_fixed.all = __readmsr(IA32_VMX_CR4_FIXED0);
	cr4.all = __readcr4();
	cr4.all |= cr_fixed.split.low;
	cr_fixed.all = __readmsr(IA32_VMX_CR4_FIXED1);
	cr4.all &= cr_fixed.split.low;
	__writecr4(cr4.all);
	_enable();

	//设置IA32_FEATURE_CONTROL寄存器的bit0 bit2支持开启vmx模式
	__ia32_feature_control_msr feature_msr = { 0 };
	feature_msr.all = __readmsr(IA32_FEATURE_CONTROL);

	if (feature_msr.lock == 0)
	{
		feature_msr.vmxon_outside_smx = 1;
		feature_msr.lock = 1;

		__writemsr(IA32_FEATURE_CONTROL, feature_msr.all);
	}
}

bool init_logical_processor2(unsigned int iter)
{
	//DbgBreakPoint();
	unsigned __int64 processor_number = iter;

	__vcpu* vcpu = &g_vmm_context.vcpu[processor_number];

	//调节控制寄存器 cr4 cr0来启用vmx模式
	adjust_control_registers();

	if (!hv::enter_vmx_operation(vcpu->vmxon))  //进入vmx模式
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Failed to put vcpu %d into VMX operation", (int)processor_number);
#endif
		return false;
	}

	if (!hv::load_vmcs_pointer(vcpu->vmcs))
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] vcpu %d load_vmcs_pointer error", (int)processor_number);
#endif
		return false;
	}

	//创建host的idt和gdt
	hv::prepare_external_structures(vcpu);
	vcpu->vcpu_status.vmx_on = true;
#if ENABLE_TRACE
	TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] vcpu %d is now in VMX operation", (int)processor_number);
#endif

	//配置vmcs区域
	fill_vmcs(vcpu, 0);
	vcpu->vcpu_status.vmm_launched = true;

	//从GUEST_RIP指定的位置继续执行
	//运行vm虚拟机	
	if (!hv::vm_launch()) {
		vcpu->vmexit_info.instruction_error = hv::vmread(VM_INSTRUCTION_ERROR);
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Vmlaunch failed error: %d", (int)vcpu->vmexit_info.instruction_error);
#endif
		vcpu->vcpu_status.vmm_launched = false;
		vcpu->vcpu_status.vmx_on = false;
		__vmx_off();  //退出vmx模式
		return false;
	}

	return true;
}

bool initalize_vcpu(unsigned int iter)
{
	return init_logical_processor2(iter);
}

//初始化vmm 并运行
bool vmm_init()
{
	//分配vmm上下文
	if (allocate_vmm_context() == false)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] allocate_vmm_context error");
#endif
		return false;
	}

	//我们需要在低于 DISPATCH_LEVEL 的 IRQL 下运行，以便 KeSetSystemAffinityThreadEx 立即生效
	NT_ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	// virtualize every cpu
	for (unsigned int iter = 0; iter < g_vmm_context.processor_count; iter++)
	{
		// restrict execution to the specified cpu
		auto const orig_affinity = KeSetSystemAffinityThreadEx(1ull << iter);

		if (!initalize_vcpu(iter)) {
			// TODO: handle this bruh -_-
			KeRevertToUserAffinityThreadEx(orig_affinity);
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] initalize_vcpu error");
#endif
			return false;
		}

		KeRevertToUserAffinityThreadEx(orig_affinity);
	}
	return true;
}

//创建host页表
void create_host_page_tables()
{
	PEPROCESS Process = NULL;


	for (Process = Global::PsGetNextProcess(NULL);
		Process != NULL;
		Process = Global::PsGetNextProcess(Process))
	{
		WCHAR SubStr[256] = { 0 };
		UNICODE_STRING ImageFileName, targetImage;
		NTSTATUS Status = KernelCommon::Utils::GetProcessName(Process, &SubStr[0]);
		if (NT_SUCCESS(Status))
		{
			RtlInitUnicodeString(&ImageFileName, SubStr);
			RtlInitUnicodeString(&targetImage, L"dwm.exe");
			if (RtlEqualUnicodeString(&ImageFileName, &targetImage, TRUE))
			{
				hv::ghv.system_cr3.flags = ((__nt_kprocess*)Process)->DirectoryTableBase;

				::KAPC_STATE ApcState;
				KeStackAttachProcess(Process, &ApcState);
				hv::prepare_host_page_tables();
				KeUnstackDetachProcess(&ApcState);
				break;
			}
		}
	}

	//hv::ghv.system_cr3.flags = hv::get_system_directory_table_base();
	//hv::prepare_host_page_tables();
}

//分配vcpu结构内存
bool init_vcpu(__vcpu* vcpu)
{
	//vcpu->vmm_stack = allocate_pool<void*>(VMM_STACK_SIZE);
	//if (vcpu->vmm_stack == nullptr)
	//{
	//	LogError("vmm stack could not be allocated");
	//	return false;
	//}
	//RtlSecureZeroMemory(vcpu->vmm_stack, VMM_STACK_SIZE);

	vcpu->vcpu_bitmaps.io_bitmap_a = allocate_aligned_pool<unsigned __int8*>(PAGE_SIZE);
	if (vcpu->vcpu_bitmaps.io_bitmap_a == nullptr)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] io bitmap a could not be allocated");
#endif
		return false;
	}
	RtlSecureZeroMemory(vcpu->vcpu_bitmaps.io_bitmap_a, PAGE_SIZE);
	vcpu->vcpu_bitmaps.io_bitmap_a_physical = MmGetPhysicalAddress(vcpu->vcpu_bitmaps.io_bitmap_a).QuadPart;

	vcpu->vcpu_bitmaps.io_bitmap_b = allocate_aligned_pool<unsigned __int8*>(PAGE_SIZE);
	if (vcpu->vcpu_bitmaps.io_bitmap_b == nullptr)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] io bitmap b could not be allocated");
#endif
		return false;
	}
	RtlSecureZeroMemory(vcpu->vcpu_bitmaps.io_bitmap_b, PAGE_SIZE);
	vcpu->vcpu_bitmaps.io_bitmap_b_physical = MmGetPhysicalAddress(vcpu->vcpu_bitmaps.io_bitmap_b).QuadPart;

	//
	// Allocate ept state structure
	//
	vcpu->ept_state = allocate_pool<__ept_state>();
	if (vcpu->ept_state == nullptr)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] ept state could not be allocated");
#endif
		return false;
	}
	RtlSecureZeroMemory(vcpu->ept_state, sizeof(__ept_state));
	InitializeListHead(&vcpu->ept_state->hooked_page_list);

	RtlSecureZeroMemory(&vcpu->host_tss, sizeof(task_state_segment_64));
	RtlSecureZeroMemory(&vcpu->host_gdt, sizeof(segment_descriptor_32) * HOST_GDT_DESCRIPTOR_COUNT);
	RtlSecureZeroMemory(&vcpu->host_idt, sizeof(segment_descriptor_interrupt_gate_64) * HOST_IDT_DESCRIPTOR_COUNT);

	//
	// Initialize ept structure
	// 初始化 ept 结构
	//
	//
	if (ept::initialize(*vcpu->ept_state) == false)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] initialize ept error");
#endif
		return false;
	}

#if ENABLE_TRACE
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[-] vcpu entry allocated successfully at 0x%p", vcpu);
#endif

	return true;
}
