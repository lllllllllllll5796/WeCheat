#include "KernelCommon.pch.h"

namespace VmxHelper
{
	using namespace KernelCommon;

	eastl::unordered_map<PEPROCESS, eastl::unordered_map<QWORD, QWORD>> g_ClockRecoveryMap;

	int vmx_Cloak_Activate(QWORD physicalAddress, int mode)   //隐身激活
	/*
	 Copies a page to a shadow page and marks the original page as execute only (or no access at all if the cpu does not support it)

	 On read/write the shadow page's contents are read/written, but execute will execute the original page

	 To access the contents of the original(executing) page use vmx_cloak_readOriginal and vmx_cloak_writeOriginal

	 possible issue: the read and execute operation can be in the same page at the same time, so when the page is swapped by the contents of the unmodified page to facilitate the read of unmodified memory
					 the unmodified code will execute as well (integrity check checking itself)

	 possible solutions:  do not cloak pages with integrity checks and then edit the integrity check willy nilly
						  use single byte edits (e.g int3 bps to facilitate changes)
						  make edits so integrity check routines are jumped over

	 Note: Affects ALL cpu's so only needs to be called once
	 将页面复制到影子页面并将原始页面标记为仅执行（或者如果CPU不支持则根本无法访问）

	 读/写时，影子页的内容被读/写，但执行将执行原始页

	 要访问原始（执行）页面的内容，请使用 vmx_cloak_readOriginal 和 vmx_cloak_writeOriginal

	 可能的问题：读取和执行操作可以同时在同一个页面中，因此当页面被未修改页面的内容交换时，以方便读取未修改的内存
						未修改的代码也将执行（完整性检查本身）

	 可能的解决方案：不要用完整性检查隐藏页面，然后随意编辑完整性检查
							使用单字节编辑（例如 int3 bps 以便于更改）
	 进行编辑，以便跳过完整性检查例程

	 注意：影响所有 cpu，因此只需要调用一次
	*/
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;  //VMCALL_CLOAK_ACTIVATE
			QWORD physicalAddress;
			QWORD mode;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_CLOAK_ACTIVATE;
		vmcallinfo.physicalAddress = physicalAddress;
		vmcallinfo.mode = mode;      //single step

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));
		}
		__except (1)
		{
			r = -1;
		}

		return r;
	}

	int vmx_Cloak_ReadOriginal(QWORD PhysicalAddress, void* destination)
	/*
	reads 4096 bytes from the cloaked page and put it into original (original must be able to hold 4096 bytes, and preferably on a page boundary)
	*/
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD physicalAddress;
			QWORD destination;
		} vmcallinfo;
#pragma pack()
		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_CLOAK_READORIGINAL;
		vmcallinfo.physicalAddress = PhysicalAddress;

		vmcallinfo.destination = (QWORD)destination;


		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));  //0 on success, anything else fail
		}
		__except (1)
		{
			r = -1;
		}

		return r;
	}

	int vmx_Cloak_WriteOriginal(QWORD PhysicalAddress, void* source)
	/*
	reads 4096 bytes from the cloaked page and put it into original (original must be able to hold 4096 bytes, and preferably on a page boundary)
	*/
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			QWORD physicalAddress;
			QWORD source;
		} vmcallinfo;
#pragma pack()
		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_CLOAK_WRITEORIGINAL;

		vmcallinfo.physicalAddress = PhysicalAddress;
		vmcallinfo.source = (QWORD)source;

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));  //0 on success, anything else fail
		}
		__except (1)
		{
			r = -1;
		}

		return r;
	}

	int vmx_Cloak_Deactivate(QWORD PhysicalAddress)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;  //password2
			unsigned int command;     //VMCALL_CLOAK_DEACTIVATE
			QWORD physicalAddress;
			QWORD mode;
		} vmcallinfo;
#pragma pack()
		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_CLOAK_DEACTIVATE;
		vmcallinfo.physicalAddress = PhysicalAddress;

		int r;
		__try
		{
			r = static_cast<int>(DoVmcall(&vmcallinfo));
		}
		__except (1)
		{
			r = -1;
		}

		return r;
	}

	void AddRecoveryMap(PEPROCESS Process, QWORD VirtualAddress, QWORD PhysicalAddress)
	{
		//LOG_DEBUG("AddRecoveryMap--->Process:0x%llX\n", Process);
		g_ClockRecoveryMap[Process].emplace(VirtualAddress, PhysicalAddress);
	}

	void DelRecoveryMap(PEPROCESS Process, QWORD VirtualAddress, QWORD PhysicalAddress)
	{
		// 查找指定进程在映射表中的条目
		auto ProcessEntry = g_ClockRecoveryMap.find(Process);
		if (ProcessEntry != g_ClockRecoveryMap.end())
		{
			// 查找指定虚拟地址在进程映射表中的条目
			auto& VirtualAddressMap = ProcessEntry->second;
			auto VirtualAddressEntry = VirtualAddressMap.find(VirtualAddress);
			if (VirtualAddressEntry != VirtualAddressMap.end()) 
			{
				// 查找指定物理地址是否匹配
				if (VirtualAddressEntry->second == PhysicalAddress) 
				{
					// 删除映射关系
					VirtualAddressMap.erase(VirtualAddressEntry);
					// 如果进程映射表为空，则从全局映射表中删除该进程
					if (VirtualAddressMap.empty()) 
					{
						g_ClockRecoveryMap.erase(ProcessEntry);
					}
				}
				else 
				{
					// 物理地址不匹配，可能需要记录错误或者采取其他操作
				}
			}
			else 
			{
				// 没有找到指定的虚拟地址映射
			}
		}
		else 
		{
			// 没有找到指定的进程映射
		}
	}

	void RecoveryCloaks(PEPROCESS Process)
	{
		BOOL bFind = FALSE;
		PHYSICAL_ADDRESS PhysicalAddress;
		
		for (auto Item : g_ClockRecoveryMap)
		{
			if (Item.first == Process)
			{
				//LOG_DEBUG("监控到目标进程关闭 RecoveryCloaks--->Process:0x%llX\n", Process);

				bFind = TRUE;

				//挂靠进程
				::KAPC_STATE apc_state;
				RtlZeroMemory(&apc_state, sizeof(apc_state));
				ImpCall(KeStackAttachProcess, Process, &apc_state);

				for (auto v : Item.second)
				{
					QWORD VirtualAddress = v.first;

					
					if (ImpCall(MmIsAddressValid, (PVOID)v.first))
					{
						PhysicalAddress.QuadPart = 0;
						PhysicalAddress = ImpCall(MmGetPhysicalAddress, (PVOID)VirtualAddress);

						if (PhysicalAddress.QuadPart == v.second)
						{
							//必须页对齐一下
							uintptr_t PABase = v.second & ~0xFFF;

							//LOG_DEBUG("应该还原内存:VA:0x%llX PA:0x%llX 物理页对齐地址:0x%llX\r\n", VirtualAddress, v.second, PABase);
							VmxHelper::vmx_Cloak_Deactivate(PABase);
						}
					}
				}

				ImpCall(KeUnstackDetachProcess, &apc_state);

				break;
			}
		}

		if (bFind)
		{
			g_ClockRecoveryMap.erase(Process);
		}
	}

}

