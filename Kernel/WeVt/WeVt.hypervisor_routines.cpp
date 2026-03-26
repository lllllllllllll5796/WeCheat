#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "WeVt.hypervisor_routines.h"
#include "WeVt.cpuid.h"

namespace hv
{
	/// <summary>
	/// Check if cpu support virtualization
	/// 检查cpu是否支持虚拟化
	/// </summary>
	/// <returns></returns>
	bool virtualization_support()
	{
		__cpuid_info cpuid = { 0 };
		__cpuid(&cpuid.cpu_info[0], 1);
		return cpuid.cpuid_eax_01.feature_information_ecx.virtual_machine_extensions;
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
}