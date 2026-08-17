#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "WeVt.hypervisor_routines.h"
#include "WeVt.cpuid.h"
#include "WeVt.vmcs.h"
#include "WeVt.interrupt.h"
#include "WeVt.ntapi.h"
#include "WeVt.vmexit_handler.h"
#include "WeVt.crx.h"
#include "WeVt.msr.h"

#define NON_CANONICIAL_ADDRESS_END 0xFFFF800000000000
#define NON_CANONICIAL_ADDRESS_START 0x0000800000000000

namespace hv
{
	volatile long vmcs_dump_lock = 0;

	/// <summary>
	/// Inject interrupt/exception to guest system
	/// </summary>
	/// <param name="vector"></param>
	/// <param name="type"></param>
	/// <param name="error_code"></param>
	/// <param name="deliver_error_code"></param>
	void inject_interruption(unsigned __int32 vector, unsigned __int32 type, unsigned __int32 error_code, bool deliver_error_code)
	{
		__vmentry_interrupt_info interrupt = { 0 };

		interrupt.interruption_type = type;
		interrupt.interrupt_vector = vector;
		interrupt.deliver_error_code = deliver_error_code;
		interrupt.valid = 1;

		if (type == INTERRUPT_TYPE_SOFTWARE_EXCEPTION || type == INTERRUPT_TYPE_PRIVILEGED_SOFTWARE_INTERRUPT || type == INTERRUPT_TYPE_SOFTWARE_INTERRUPT)
			hv::vmwrite<unsigned __int64>(VM_ENTRY_INSTRUCTION_LENGTH, hv::vmread(VM_EXIT_INSTRUCTION_LENGTH));

		if (deliver_error_code == true)
			hv::vmwrite<unsigned __int64>(VM_ENTRY_EXCEPTION_ERROR_CODE, error_code);

		hv::vmwrite<unsigned __int64>(VM_ENTRY_INTERRUPTION_INFO_FIELD, interrupt.all);
	}

	/// <summary>
	/// Write to reset io port to perform hard reset
	/// </summary>
	void hard_reset()
	{
		__reset_control_register reset_register;
		reset_register.all = __inbyte(RESET_IO_PORT);

		//
		// Reset CPU bit set, determines type of reset based on:
		//        - System Reset = 0; soft reset by activating INIT# for 16 PCI clocks.
		//        - System Reset = 1; then hard reset by activating PLTRST# and SUS_STAT#.
		//        - System Reset = 1; main power well reset.
		//

		reset_register.reset_cpu = 1;
		reset_register.system_reset = 1;

		__outbyte(RESET_IO_PORT, reset_register.all);
	}

	/// <summary>
	/// Swap cr3 with current process dtb
	/// </summary>
	/// <returns> old cr3 </returns>
	unsigned __int64 swap_context()
	{
		__nt_kprocess* current_process;
		//注意由于将host里的gs寄存器清零了，固无法继续使用CurrentProcess、CurrentThread系列函数
		current_process = (__nt_kprocess*)IoGetCurrentProcess();
		unsigned __int64 current_cr3 = __readcr3();
		unsigned __int64 guest_cr3 = current_process->DirectoryTableBase;

		__writecr3(guest_cr3);
		return current_cr3;
	}

	void swap_context(__vcpu* vcpu, unsigned __int64 new_cr3)
	{
		vcpu->target_cr3.flags = new_cr3;
	}

	/// <summary>
	/// Restore cr3
	/// </summary>
	/// <param name="old_cr3"></param>
	void restore_context(__vcpu* vcpu)
	{
		vcpu->target_cr3.flags = 0;
	}

	/// <summary>
	/// Get system directory table base
	/// </summary>
	/// <returns></returns>
	unsigned __int64 get_system_directory_table_base()
	{
		return ((__nt_kprocess*)PsInitialSystemProcess)->DirectoryTableBase;
	}

	/// <summary>
	/// Read vmcs field
	/// </summary>
	/// <param name="vmcs_field"></param>
	/// <returns></returns>
	unsigned __int64 vmread(unsigned __int64 vmcs_field)
	{
		unsigned __int64 value;
		__vmx_vmread(vmcs_field, &value);
		return value;
	}

	/// <summary>
	/// Check if address is canonicial (level 4 paging)
	/// </summary>
	/// <param name="address"></param>
	/// <returns></returns>
	bool is_address_canonical(unsigned __int64 address)
	{
		if (address < NON_CANONICIAL_ADDRESS_END && address > NON_CANONICIAL_ADDRESS_START)
			return false;
		return true;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <returns> Return current guest privilage level</returns>
	unsigned __int8 get_guest_cpl()
	{
		return vmread(GUEST_CS_SELECTOR) & 3;
	}

	//获取guest的当前cpl
	uint16_t current_guest_cpl()
	{
		vmx_segment_access_rights ss;
		ss.flags = static_cast<uint32_t>(vmread(GUEST_SS_ACCESS_RIGHTS));
		return ss.descriptor_privilege_level;
	}

	/// <summary>
	/// Set 1 msr in msr bitmap
	/// </summary>
	/// <param name="msr"> Msr number </param>
	/// <param name="vcpu"> Pointer to current vcpu </param>
	/// <param name="read"> If set vmexit occur on reading this msr </param>
	/// <param name="write"> If set vmexit occur on writing to this msr </param>
	/// <param name="value"> If true set msr bit else clear </param>
	void set_msr_bitmap(vmx_msr_bitmap& bitmap, uint32_t const msr, bool const enable_exiting)
	{
		auto const bit = static_cast<uint8_t>(enable_exiting ? 1 : 0);

		if (msr <= MSR_ID_LOW_MAX)
			// set the bit in the low bitmap
			bitmap.rdmsr_low[msr / 8] = (bit << (msr & 0b0111));
		else if (msr >= MSR_ID_HIGH_MIN && msr <= MSR_ID_HIGH_MAX)
			// set the bit in the high bitmap
			bitmap.rdmsr_high[(msr - MSR_ID_HIGH_MIN) / 8] = (bit << (msr & 0b0111));
	}


	/// <summary>
	/// Set or unset bit in io port bitmap
	/// </summary>
	/// <param name="io_port"> IO port which you want to set</param>
	/// <param name="vcpu"> Pointer to current vcpu </param>
	/// <param name="value"> If true then set bit else unset bit</param>
	void set_io_bitmap(unsigned __int16 io_port, __vcpu* vcpu, bool value)
	{
		unsigned __int16 bitmap_position;
		unsigned __int8 bitmap_bit;

		//
		// IO ports from 0x8000 to 0xFFFF are encoded in io bitmap b
		if (io_port >= 0x8000)
		{
			io_port -= 0x8000;
			bitmap_position = io_port / 8;
			bitmap_bit = io_port % 8;

			if (value == true)
				*(vcpu->vcpu_bitmaps.io_bitmap_b + bitmap_position) |= (1 << bitmap_bit);
			else
				*(vcpu->vcpu_bitmaps.io_bitmap_b + bitmap_position) &= ~(1 << bitmap_bit);
		}

		//
		// IO ports from 0 to 0x7fff are encoded in io bitmap a
		else
		{
			bitmap_position = io_port / 8;
			bitmap_bit = io_port % 8;

			if (value == true)
				*(vcpu->vcpu_bitmaps.io_bitmap_a + bitmap_position) |= (1 << bitmap_bit);
			else
				*(vcpu->vcpu_bitmaps.io_bitmap_a + bitmap_position) &= ~(1 << bitmap_bit);
		}
	}

	/// <summary>
	/// Used to get address passed by user in inpvcid
	/// </summary>
	/// <param name="guest_registers"></param>
	/// <returns></returns>
	unsigned __int64 get_guest_address(__vcpu* vcpu)
	{
		__vmexit_instruction_information2 instruction_information;

		instruction_information.all = vcpu->vmexit_info.instruction_information;

		unsigned __int64 displacement = vcpu->vmexit_info.qualification;

		unsigned __int64 base_value = !instruction_information.base_reg_invalid ? *(&vcpu->vmexit_info.guest_registers->rax - instruction_information.base_reg) : 0;

		unsigned __int64 index_value = !instruction_information.index_reg_invalid ? *(&vcpu->vmexit_info.guest_registers->rax - instruction_information.index_reg) : 0;

		index_value = index_value * (1ULL << instruction_information.scaling);

		unsigned __int64 segment_base = hv::vmread(GUEST_ES_BASE + (instruction_information.segment_register << 1));

		unsigned __int64 guest_address = displacement + base_value + index_value + segment_base;

		return guest_address;
	}

	void set_mtf(bool set)
	{
		__vmx_primary_processor_based_control primary_controls{ hv::vmread(PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROL) };
		primary_controls.monitor_trap_flag = set;
		hv::vmwrite(PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROL, primary_controls.all);
	}

	/// <summary>
	/// Check if cpu support virtualization
	/// <summary>
	/// </summary>
	/// <returns></returns>
	bool virtualization_support()
	{
		__cpuid_info cpuid = { 0 };
		__cpuid(&cpuid.cpu_info[0], 1);
		return cpuid.cpuid_eax_01.feature_information_ecx.virtual_machine_extensions;
	}

	/// <summary>
	/// Disable vmx operation
	/// </summary>
	/// <returns></returns>
	void disable_vmx_operation()
	{
		__try
		{
			__cr4 cr4 = { 0 };
			__ia32_feature_control_msr feature_msr = { 0 };
			cr4.all = __readcr4();
			cr4.vmx_enable = 0;
			__writecr4(cr4.all);
		}
		__except (1)
		{
		}
	}

	bool vmx_on(unsigned __int64 vmxon_phys_addr)
	{
		//0则表示操作成功
		unsigned char res = __vmx_on(&vmxon_phys_addr);
		if (res)
		{
			return false;
		}
		return true;
	}

	bool vmx_vmclear(unsigned __int64 vmcs_phys)
	{
		//0则表示操作成功
		unsigned char res = __vmx_vmclear(&vmcs_phys);
		if (res)
		{
			return false;
		}
		return true;
	}

	bool vmx_vmptrld(unsigned __int64 vmcs_phys)
	{
		//0则表示操作成功
		unsigned char res = __vmx_vmptrld(&vmcs_phys);
		if (res)
		{
			return false;
		}
		return true;
	}

	void dump_vmcs()
	{
	}

}
