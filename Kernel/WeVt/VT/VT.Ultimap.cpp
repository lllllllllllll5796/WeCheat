#include "WeVt.pch.h"

namespace VT_Ultimap
{
	using namespace VmxHelper;
	unsigned int Vmx_Ultimap_GetDebugInfo(PULTIMAPDEBUGINFO debuginfo)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			ULTIMAPDEBUGINFO debuginfo;
		} vmcallinfo;
#pragma pack()

		unsigned int i;

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ULTIMAP_DEBUGINFO;

		i = (unsigned int)DoVmcall(&vmcallinfo);
		*debuginfo = vmcallinfo.debuginfo;

		return i;
	}

	unsigned int vmx_ultimap(UINT_PTR cr3towatch, UINT64 debugctl_value, void* storeaddress)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			UINT64 cr3;
			UINT64 debugctl;
			UINT64 storeaddress;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ULTIMAP;
		vmcallinfo.cr3 = (UINT64)cr3towatch;
		vmcallinfo.debugctl = (UINT64)debugctl_value;
		vmcallinfo.storeaddress = (UINT64)(UINT_PTR)storeaddress;

		LOG_DEBUG("vmx_ultimap(%I64x, %I64x, %I64x)\n", (UINT64)vmcallinfo.cr3, (UINT64)vmcallinfo.debugctl, vmcallinfo.storeaddress);


		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_ultimap_disable()
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ULTIMAP_DISABLE;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_ultimap_pause()
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ULTIMAP_PAUSE;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_ultimap_resume()
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ULTIMAP_RESUME;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}
}