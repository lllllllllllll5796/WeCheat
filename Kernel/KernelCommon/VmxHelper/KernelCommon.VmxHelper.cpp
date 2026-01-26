#include "KernelCommon.pch.h"
#include "KernelCommon.VmxHelper.h"

namespace VmxHelper
{
	DOVMCALL* DoVmcall;

	unsigned int vmxusable;
	UINT64   vmx_password1;
	unsigned int vmx_password2;
	UINT64   vmx_password3;
	unsigned int vmx_version;

#ifdef AMD64
	extern "C" UINT_PTR dovmcall_intel(void* vmcallinfo/*, unsigned int level1pass*/);
	extern "C" UINT_PTR dovmcall_amd(void* vmcallinfo/*, unsigned int level1pass*/);
	//dovmcall is defined in VmxHelperAsm.asm
#else
	_declspec(naked) UINT_PTR dovmcall_intel(void* vmcallinfo)
	{
		__asm
		{
			push edx
			mov eax, [esp + 8]  //+8 because of push
			mov edx, dword ptr vmx_password1
			mov ecx, dword ptr vmx_password3
			__emit 0x0f
			__emit 0x01
			__emit 0xc1 //vmcall, eax will be edited, or a UD exception will be raised
			pop edx
			ret 8
		}
	}

	_declspec(naked) UINT_PTR dovmcall_amd(void* vmcallinfo)
	{
		__asm
		{
			push edx
			mov eax, [esp + 8]
			mov edx, dword ptr  vmx_password1
			mov ecx, dword ptr  vmx_password3
			__emit 0x0f
			__emit 0x01
			__emit 0xd9 //vmmcall, eax will be edited, or a UD exception will be raised
			pop edx
			ret 8
		}
	}
#endif

	void vmx_Init_DoVmcall(int isIntel)
	{
		if (isIntel)
			DoVmcall = dovmcall_intel;
		else
			DoVmcall = dovmcall_amd;
	}

	unsigned int vmx_add_memory(UINT64* list, int count)
	{
		int r = 0;
		int j = 0;
#pragma pack(1)
		typedef struct _vmcall_add_memory
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			UINT64 PhysicalPages[0];
		} AddMemoryInfoCall, * PAddMemoryInfoCall;
#pragma pack()
		PAddMemoryInfoCall vmcallinfo = (PAddMemoryInfoCall)ImpCall(ExAllocatePool, NonPagedPool, sizeof(AddMemoryInfoCall) + count * sizeof(UINT64));


// 		LOG_DEBUG("vmx_add_memory(%p,%d)\n", list, count);
// 		LOG_DEBUG("vmx_add_memory(vmx_password1=%x,vmx_password2=%x)\n", vmx_password1, vmx_password2);
// 
// 		LOG_DEBUG("structsize at offset %d\n", (UINT64)(&vmcallinfo->structsize) - (UINT64)vmcallinfo);
// 		LOG_DEBUG("level2pass at offset %d\n", (UINT64)(&vmcallinfo->level2pass) - (UINT64)vmcallinfo);
// 		LOG_DEBUG("command at offset %d\n", (UINT64)(&vmcallinfo->command) - (UINT64)vmcallinfo);
// 		LOG_DEBUG("PhysicalPages[0] at offset %d\n", (UINT64)(&vmcallinfo->PhysicalPages[0]) - (UINT64)vmcallinfo);
// 		LOG_DEBUG("PhysicalPages[1] at offset %d\n", (UINT64)(&vmcallinfo->PhysicalPages[1]) - (UINT64)vmcallinfo);

		__try
		{
			int i;
			vmcallinfo->structsize = sizeof(AddMemoryInfoCall) + count * sizeof(UINT64);
			//LOG_DEBUG("vmcallinfo->structsize=%d\n", vmcallinfo->structsize);
			vmcallinfo->level2pass = vmx_password2;
			vmcallinfo->command = VMCALL_ADD_MEMORY;
			j = 1;
			for (i = 0; i < count; i++)
			{
				vmcallinfo->PhysicalPages[i] = list[i];
			}
			j = 2;

			r = (unsigned int)DoVmcall(vmcallinfo);
			j = 3; //never
		}
		__except (1)
		{
			//LOG_DEBUG("vmx_add_memory(%p,%d) gave an exception at part %d with exception code %x\n", list, count, j, GetExceptionCode());
			//vmdisk头文件化这里会过来
			r = 0x100;
		}

		ImpCall(ExFreePool, vmcallinfo);
		return r;
	}

	unsigned int vmx_getversion()
	/*
	This will either raise a unhandled opcode exception, or return the used dbvm version
	*/
	{
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;

		//LOG_DEBUG("Vmx_GetVersion()\n");

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_GETVERSION;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	BOOL isHookAddress(PVOID Address)
	{
		LOG_DEBUG("IsHookAddress:%p\r\n", Address);
		//......
		return TRUE;
	}

	unsigned int vmx_Test()
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;
#pragma pack()

		LOG_DEBUG("Vmx_Test()\n");

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_TEST;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_Add_R3BreakPoint(QWORD physical_address, QWORD target_address, QWORD breakpoint_type)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD physical_address;
			QWORD target_address;
			QWORD breakpoint_type;
		} vmcallinfo;
#pragma pack()

		//LOG_DEBUG("vmx_Add_BreakPoint()\n");

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ADD_BREAKPOINT;
		vmcallinfo.physical_address = physical_address;
		vmcallinfo.target_address = target_address;
		vmcallinfo.breakpoint_type = breakpoint_type;

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));
		}
		__except (1)
		{
			r = -1;
		}

		LOG_DEBUG("[+] vmx_Add_R3BreakPoint--->返回值r:0x%X\r\n", r);

		return r;
	}

	unsigned int vmx_Del_R3BreakPoint(QWORD physical_address)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD physical_address;
		} vmcallinfo;
#pragma pack()

		//LOG_DEBUG("vmx_Add_BreakPoint()\n");

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_DEL_BREAKPOINT;
		vmcallinfo.physical_address = physical_address;

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));
		}
		__except (1)
		{
			r = -1;
		}

		LOG_DEBUG("[+] vmx_Del_R3BreakPoint--->返回值r:0x%X\r\n", r);

		return r;
	}

	unsigned int vmx_Add_R0Hook(QWORD physical_address, QWORD target_address, QWORD fake_address, QWORD trampoline_address)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD physical_address;
			QWORD target_address;
			QWORD fake_address;
			QWORD trampoline_address;
		} vmcallinfo;
#pragma pack()

		//LOG_DEBUG("vmx_Add_R0Hook()\n");

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_ADD_R0HOOK;
		vmcallinfo.physical_address = physical_address;
		vmcallinfo.target_address = target_address;
		vmcallinfo.fake_address = fake_address;
		vmcallinfo.trampoline_address = trampoline_address;

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));
		}
		__except (1)
		{
			r = -1;
		}

		LOG_DEBUG("[+] 返回值r:0x%X\r\n", r);
		return r;
	}

	unsigned int vmx_Del_R0Hook(QWORD physical_address)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD physical_address;
		} vmcallinfo;
#pragma pack()
		//LOG_DEBUG("Vmx_Del_R0Hook()\n");

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_DEL_R0HOOK;
		vmcallinfo.physical_address = physical_address;

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));
		}
		__except (1)
		{
			r = -1;
		}

		LOG_DEBUG("返回值r:0x%X\r\n", r);
		return r;

	}

	void broadcast_invept_all_contexts(KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		UNREFERENCED_PARAMETER(DeferredContext);
		UNREFERENCED_PARAMETER(Dpc);
		//每个CPU都会执行一遍
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD AllCpu;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_INVEPT_CONTEXT;
		vmcallinfo.AllCpu = TRUE;

		DoVmcall(&vmcallinfo);
		ImpCall(KeSignalCallDpcSynchronize,SystemArgument2);
		ImpCall(KeSignalCallDpcDone,SystemArgument1);
	}

	void vmx_Invept()
	{
		ImpCall(KeGenericCallDpc, broadcast_invept_all_contexts, NULL);   //不要用lambda实现
	}

	unsigned int vmx_GetRealCR0()
	{
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_GETCR0;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	UINT_PTR vmx_GetRealCR3()
	{
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_GETCR3;

		return DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_GetRealCR4()
	{
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
		} vmcallinfo;

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_GETCR4;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}


	unsigned int vmx_Register_Cr3_Callback(unsigned int cs, unsigned int eip, unsigned int ss, unsigned int esp)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			unsigned int callbacktype; //32-bit for this driver, so always 0
			unsigned long long callback_eip;
			unsigned int callback_cs;
			unsigned long long callback_esp;
			unsigned int callback_ss;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_REGISTER_CR3_EDIT_CALLBACK;
		vmcallinfo.callbacktype = 0;
		vmcallinfo.callback_eip = eip;
		vmcallinfo.callback_cs = cs;
		vmcallinfo.callback_esp = esp;
		vmcallinfo.callback_ss = ss;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_Exit_Cr3_Callback(unsigned int newcr3)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			unsigned long long newcr3;
		} vmcallinfo;
#pragma pack()

		//LOG_DEBUG("vmx_exit_cr3_callback(%x)\n",newcr3);

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_RETURN_FROM_CR3_EDIT_CALLBACK;
		vmcallinfo.newcr3 = newcr3;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_ChangeRegOnBp(QWORD physicalAddress, CHANGEREGONBPINFO* changereginfo)
	/*
	places an int3 bp at the given address, and on execution changes the state to the given state
	if a cloaked page is given, the BP will be set in the executing page

	if no cloaked page is given, cloak it (needed for the single step if no IP change is done)

	Note: effects ALL cpu's

	在给定地址放置一个 int3 bp，并在执行时将状态更改为给定状态
	如果给定一个隐藏页面，BP 将在执行页面中设置

	如果没有给出隐藏页面，则隐藏它（如果没有进行 IP 更改，则需要单步执行）

	注意：影响所有 CPU
	*/
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command; //VMCALL_CLOAK_CHANGEREGONBP
			QWORD physicalAddress;
			CHANGEREGONBPINFO changereginfo;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_CLOAK_CHANGEREGONBP;
		vmcallinfo.physicalAddress = physicalAddress;
		vmcallinfo.changereginfo = *changereginfo;

		return (unsigned int)DoVmcall(&vmcallinfo); //0 on success, anything else fail
	}

	unsigned int vmx_disable_dataPageFaults()
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
		vmcallinfo.command = VMCALL_DISABLE_DATAPAGEFAULTS;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	unsigned int vmx_enable_dataPageFaults()
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
		vmcallinfo.command = VMCALL_ENABLE_DATAPAGEFAULTS;

		return (unsigned int)DoVmcall(&vmcallinfo);
	}

	UINT_PTR vmx_getLastSkippedPageFault()
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
		vmcallinfo.command = VMCALL_GETLASTSKIPPEDPAGEFAULT;

		return (UINT_PTR)DoVmcall(&vmcallinfo);
	}

	int vmx_causedCurrentDebugBreak()
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
		vmcallinfo.command = VMCALL_CAUSEDDEBUGBREAK;

		return (int)DoVmcall(&vmcallinfo);
	}
}

