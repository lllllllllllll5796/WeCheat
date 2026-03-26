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
}