#pragma once

namespace VT_Ultimap
{
	unsigned int Vmx_Ultimap_GetDebugInfo(VmxHelper::PULTIMAPDEBUGINFO debuginfo);

	unsigned int vmx_ultimap(UINT_PTR cr3towatch, UINT64 debugctl_value, void* storeaddress);

	unsigned int vmx_ultimap_disable();

	unsigned int vmx_ultimap_pause();

	unsigned int vmx_ultimap_resume();
}