#include "KernelCommon.pch.h"

namespace VmxHelper
{
	using namespace KernelCommon;

	eastl::unordered_map<PEPROCESS, eastl::unordered_map<QWORD, QWORD>> g_ClockRecoveryMap;

	int vmx_Cloak_Activate(QWORD physicalAddress, int mode)   //��������
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
	 ��ҳ�渴�Ƶ�Ӱ��ҳ�沢��ԭʼҳ����Ϊ��ִ�У��������CPU��֧��������޷����ʣ�

	 ��/дʱ��Ӱ��ҳ�����ݱ���/д����ִ�н�ִ��ԭʼҳ

	 Ҫ����ԭʼ��ִ�У�ҳ������ݣ���ʹ�� vmx_cloak_readOriginal �� vmx_cloak_writeOriginal

	 ���ܵ����⣺��ȡ��ִ�в�������ͬʱ��ͬһ��ҳ���У���˵�ҳ�汻δ�޸�ҳ������ݽ���ʱ���Է����ȡδ�޸ĵ��ڴ�
						δ�޸ĵĴ���Ҳ��ִ�У������Լ�鱾����

	 ���ܵĽ����������Ҫ�������Լ������ҳ�棬Ȼ������༭�����Լ��
							ʹ�õ��ֽڱ༭������ int3 bps �Ա��ڸ��ģ�
	 ���б༭���Ա����������Լ������

	 ע�⣺Ӱ������ cpu�����ֻ��Ҫ����һ��
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
		// ����ָ��������ӳ����е���Ŀ
		auto ProcessEntry = g_ClockRecoveryMap.find(Process);
		if (ProcessEntry != g_ClockRecoveryMap.end())
		{
			// ����ָ�������ַ�ڽ���ӳ����е���Ŀ
			auto& VirtualAddressMap = ProcessEntry->second;
			auto VirtualAddressEntry = VirtualAddressMap.find(VirtualAddress);
			if (VirtualAddressEntry != VirtualAddressMap.end()) 
			{
				// ����ָ��������ַ�Ƿ�ƥ��
				if (VirtualAddressEntry->second == PhysicalAddress) 
				{
					// ɾ��ӳ���ϵ
					VirtualAddressMap.erase(VirtualAddressEntry);
					// �������ӳ���Ϊ�գ����ȫ��ӳ�����ɾ���ý���
					if (VirtualAddressMap.empty()) 
					{
						g_ClockRecoveryMap.erase(ProcessEntry);
					}
				}
				else 
				{
					// ������ַ��ƥ�䣬������Ҫ��¼������߲�ȡ��������
				}
			}
			else 
			{
				// û���ҵ�ָ���������ַӳ��
			}
		}
		else 
		{
			// û���ҵ�ָ���Ľ���ӳ��
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
				//LOG_DEBUG("��ص�Ŀ����̹ر� RecoveryCloaks--->Process:0x%llX\n", Process);

				bFind = TRUE;

				//�ҿ�����
				::KAPC_STATE apc_state;
				RtlZeroMemory(&apc_state, sizeof(apc_state));
				KeStackAttachProcess(Process, &apc_state);

				for (auto v : Item.second)
				{
					QWORD VirtualAddress = v.first;

					
					if (MmIsAddressValid((PVOID)v.first))
					{
						PhysicalAddress.QuadPart = 0;
						PhysicalAddress = MmGetPhysicalAddress((PVOID)VirtualAddress);

						if (PhysicalAddress.QuadPart == v.second)
						{
							//����ҳ����һ��
							uintptr_t PABase = v.second & ~0xFFF;

							//LOG_DEBUG("Ӧ�û�ԭ�ڴ�:VA:0x%llX PA:0x%llX ����ҳ�����ַ:0x%llX\r\n", VirtualAddress, v.second, PABase);
							VmxHelper::vmx_Cloak_Deactivate(PABase);
						}
					}
				}

				KeUnstackDetachProcess(&apc_state);

				break;
			}
		}

		if (bFind)
		{
			g_ClockRecoveryMap.erase(Process);
		}
	}

}

