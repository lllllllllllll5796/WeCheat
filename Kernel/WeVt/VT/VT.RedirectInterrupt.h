#pragma once

namespace VT_RedirectInterrupt
{
	int vmx_HasRedirectedInt1();
	unsigned int vmx_Redirect_Interrupt1(VmxHelper::VMXInterruptRedirectType redirecttype, unsigned int newintvector, unsigned int int1cs, UINT_PTR int1eip);
	unsigned int vmx_Redirect_Interrupt3(VmxHelper::VMXInterruptRedirectType redirecttype, unsigned int newintvector, unsigned int int3cs, UINT_PTR int3eip);
	unsigned int vmx_Redirect_Interrupt14(VmxHelper::VMXInterruptRedirectType redirecttype, unsigned int newintvector, unsigned int int14cs, UINT_PTR int14eip);
}