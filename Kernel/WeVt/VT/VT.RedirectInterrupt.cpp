#include "WeVt.pch.h"

namespace VT_RedirectInterrupt
{
	using namespace VmxHelper;

	int vmx_HasRedirectedInt1()
	{
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_INT1REDIRECTED;
		return (int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_Redirect_Interrupt1(VMXInterruptRedirectType redirecttype, unsigned int newintvector, unsigned int int1cs, UINT_PTR int1eip)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			unsigned int redirecttype;
			unsigned int newintvector;
			UINT64 int1eip;
			unsigned int int1cs;
		} vmcallinfo;
#pragma pack()

		//LOG_DEBUG("Vmx_Redirect_Interrupt1: redirecttype=%d int1cs=%x int1eip=%llx sizeof(vmcallinfo)=%x\n", redirecttype, int1cs, int1eip, sizeof(vmcallinfo));
		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_REDIRECTINT1;
		vmcallinfo.redirecttype = redirecttype;
		vmcallinfo.newintvector = newintvector;
		vmcallinfo.int1eip = int1eip;
		vmcallinfo.int1cs = int1cs;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_Redirect_Interrupt3(VMXInterruptRedirectType redirecttype, unsigned int newintvector, unsigned int int3cs, UINT_PTR int3eip)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			unsigned int redirecttype;
			unsigned int newintvector;
			unsigned long long int3eip;
			unsigned int int3cs;
		} vmcallinfo;
#pragma pack()

		LOG_DEBUG("Vmx_Redirect_Interrupt3: int3cs=%x int3eip=%x sizeof(vmcallinfo)=%x\n", int3cs, int3eip, sizeof(vmcallinfo));
		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_REDIRECTINT3;
		vmcallinfo.redirecttype = redirecttype;
		vmcallinfo.newintvector = newintvector;
		vmcallinfo.int3eip = int3eip;
		vmcallinfo.int3cs = int3cs;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_Redirect_Interrupt14(VMXInterruptRedirectType redirecttype, unsigned int newintvector, unsigned int int14cs, UINT_PTR int14eip)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			unsigned int redirecttype;
			unsigned int newintvector;
			unsigned long long int14eip;
			unsigned int int14cs;
		} vmcallinfo;
#pragma pack()

		LOG_DEBUG("Vmx_Redirect_Interrupt14: int14cs=%x int14eip=%x sizeof(vmcallinfo)=%x\n", int14cs, int14eip, sizeof(vmcallinfo));
		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_REDIRECTINT14;
		vmcallinfo.redirecttype = redirecttype;
		vmcallinfo.newintvector = newintvector;
		vmcallinfo.int14eip = int14eip;
		vmcallinfo.int14cs = int14cs;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}
}