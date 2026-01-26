#include "KernelCommon.pch.h"
#include "KernelCommon.VmxHelper.EptHook.h"
#include "KernelCommon.VmxHelper.Cloak.h"

// #include "asmjit/asmjit/src/asmjit/x86.h"
// #include "asmjit/asmtk/src/asmtk/asmtk.h"

namespace VmxHelper
{
	using namespace KernelCommon;

	eastl::unordered_map<eastl::string, eastl::shared_ptr<EptHookItem>> g_EptHooks;

	PVOID ConstructDetour(uint64_t& instructionPointer, const PVOID& target_address, ZydisDecoder& decoder, ZydisDecodedInstruction& instruction, uint32_t hookcode_length)
	{
#pragma pack(1)
		struct RetJmp
		{
			unsigned char sub_rsp[4] = { 0x48,0x83,0xEC,0x08 };
			struct
			{
				unsigned char mov_rsp[3] = { 0xC7,0x04,0x24 };
				unsigned int value;
			}L;
			struct
			{
				unsigned char mov_rsp[4] = { 0xC7,0x44,0x24,0x04 };
				unsigned int value;
			}H;
			char ret = (char)(0xC3);
		};
		static_assert(sizeof(RetJmp) == 20, "error size");
#pragma pack()

		char* data = (char*)target_address;
		instructionPointer = (ULONG64)target_address;
		ZydisDecoderContext context = {};
		ZyanUSize remain_length = 100;
		int broken_length = 0;

		for (int i = 0; i < hookcode_length; i += instruction.length)
		{
			if (!ZYAN_SUCCESS(ZydisDecoderDecodeInstruction(&decoder, &context, (const ZyanU8*)data, remain_length, &instruction)))
				break;

			LOG_DEBUG("[+] instruction_length:%d\r\n", instruction.length);
			broken_length += instruction.length;
			remain_length -= instruction.length;

			data += instruction.length;
		}

		LOG_DEBUG("[+] broken_length:%d\r\n", broken_length);
		if (broken_length > 0)
		{
			
			//ExAllocatePool
			PVOID trampoline_address = ImpCall(ExAllocatePool, NonPagedPool, 100);
			if (trampoline_address)
			{
				memcpy(trampoline_address, target_address, broken_length);  //被破坏的内容
				RetJmp ret;
				ret.H.value = ((ULONG64)target_address + broken_length) >> 32;
				ret.L.value = ((ULONG64)target_address + broken_length) & 0xffffffff;
				memcpy(PVOID((ULONG64)trampoline_address + broken_length), &ret, sizeof(RetJmp));  //填充回跳
				return trampoline_address;
			}
		}

		return NULL;
	}

	BOOL EptHook(const eastl::string& function_name, void* target_address, void* fake_function, void* trampoline_address, void* hookcode, size_t hookcode_length)
	{
		if (target_address && fake_function && trampoline_address)
		{
			KIRQL  Irql = KernelCommon::Utils::RaiseIrql();

			auto AutoLowIrql = eastl::experimental::make_scope_exit(
				[&]()
				{
					KernelCommon::Utils::LowerIrql(Irql);
				}
			);

			//为了对抗YDArk的VTInlineHook检测
			//跳板函数缓冲区就是100个字节,详见AsmTrampoline.asm

			((PBYTE)trampoline_address)[99] = hookcode_length;

			BOOL bFirstPageHook = FALSE;
			int r;
			int i;

// 			SIZE_T PageSize = PAGE_SIZE;
// 
// 			if (NT_SUCCESS(ImpCall(ZwLockVirtualMemory, (HANDLE)-1i64, &target_address, &PageSize, LOCK_VM_IN_WORKING_SET | LOCK_VM_IN_RAM)))
// 			{
// 
// 			}

			//target_address是要Hook的位置
			QWORD PA = ImpCall(MmGetPhysicalAddress, (void*)target_address).QuadPart;

			r = vmx_Add_R0Hook(PA, (QWORD)target_address, (QWORD)fake_function, (QWORD)trampoline_address);

			if (r != 0)
			{
				return TRUE;  //已经挂钩过了
			}

			r = vmx_Cloak_Activate(PA, 1);  //英特尔cpu上使PA不可读,而AMD上使其不可执行并且读的时候还是假数据

			if (r == 1)
			{
				bFirstPageHook = FALSE;
			}
			else if (r == 0)
			{
				bFirstPageHook = TRUE;
			}
			else if (r == -1)
			{
				return FALSE;
			}

			//LOG_DEBUG("[+] bFirstPageHook:%d\r\n", bFirstPageHook);

			KernelIntrin__invlpg((void*)target_address);   //INVLPG 将使与源操作数地址匹配的TLB条目无效

			PVOID target_page_address = MAKE_PAGE_ALIGN(target_address);
			ULONG64 target_address_offset = (ULONG64)target_address - (ULONG64)target_page_address;

			//LOG_DEBUG("目标函数所在页面起始地址:%p 偏移:0x%llX\r\n", target_page_address, target_address_offset);

			uint8_t executable[0x1000];                   //正好一个页面
			r = vmx_Cloak_ReadOriginal(PA, executable);   //读取原始内容

			if (r != 0)
			{
				r = vmx_Cloak_Deactivate(PA);
				return FALSE;
			}

			if (bFirstPageHook == TRUE)
			{
				LOG_DEBUG("[+] 页面第一次被Hook\r\n");
				g_EptHooks[function_name] = eastl::make_shared<EptHookItem>();
				g_EptHooks[function_name]->target_address = target_address;
	
				g_EptHooks[function_name]->target_page_address = target_page_address;
				g_EptHooks[function_name]->target_address_offset = target_address_offset;
				g_EptHooks[function_name]->hookcode_length = hookcode_length;

				g_EptHooks[function_name]->backup_page_context = (unsigned char*)malloc(0x1000);   //用来保存备份的最原始内容
				memcpy(g_EptHooks[function_name]->backup_page_context, executable, 0x1000);
			}
			else
			{
				LOG_DEBUG("[+] 此页面并不是第一次被Hook\r\n");
				BOOL IsSamePage = FALSE;
				unsigned char* backup_page_context = nullptr;

				for (auto v : g_EptHooks)
				{
					if (v.second->target_page_address == target_page_address)
					{
						IsSamePage = TRUE;
						backup_page_context = v.second->backup_page_context;
						break;
					}
				}
				
				if (IsSamePage == TRUE && backup_page_context)
				{
					g_EptHooks[function_name] = eastl::make_shared<EptHookItem>();

					g_EptHooks[function_name]->target_address = target_address;

					g_EptHooks[function_name]->target_page_address = target_page_address;
					g_EptHooks[function_name]->target_address_offset = target_address_offset;
					g_EptHooks[function_name]->hookcode_length = hookcode_length;

					g_EptHooks[function_name]->backup_page_context = backup_page_context;
				}
 			}

			//LOG_DEBUG("执行页面的中间缓冲区地址:%p\r\n", executable);

			unsigned char* buf = executable + target_address_offset;

			memcpy(buf, hookcode, hookcode_length);

			//--------------
			r = vmx_Cloak_WriteOriginal(PA, executable);

			if (r == 0)
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	VOID EptUnHook(const eastl::string& function_name)
	{
		KIRQL  Irql = KernelCommon::Utils::RaiseIrql();

		auto AutoLowIrql = eastl::experimental::make_scope_exit([&]() {	KernelCommon::Utils::LowerIrql(Irql); });

		if (g_EptHooks.count(function_name))
		{
			void* target_address = g_EptHooks[function_name]->target_address;

			PVOID target_page_address = MAKE_PAGE_ALIGN(target_address);

			ULONG64 target_address_offset = (ULONG64)target_address - (ULONG64)target_page_address;

			int r;
			QWORD PA = ImpCall(MmGetPhysicalAddress, (void*)target_address).QuadPart;
			if (PA == 0)
			{
				LOG_DEBUG("获得物理地址失败\n");
				return;
			}

			uint8_t buf[0x1000];               //正好一个页面
			r = vmx_Cloak_ReadOriginal(PA, buf);
			if (r == 0)
			{
				unsigned char* backup_target_address = g_EptHooks[function_name]->backup_page_context + target_address_offset;
				int hookcode_length = g_EptHooks[function_name]->hookcode_length;

// 				for (int i = 0; i < hookcode_length;i++)
// 				{
// 					LOG_DEBUG("0x%X\r\n", backup_target_address[i]);
// 				}

				memcpy(buf + target_address_offset, backup_target_address, hookcode_length);  //恢复被破坏的字节

				r = vmx_Cloak_WriteOriginal(PA, buf);

				if (r == 0)
				{
					//LOG_DEBUG("恢复原始内容\r\n");
					r = vmx_Del_R0Hook(PA);

					if (r == 0)
					{
						LOG_DEBUG("Delete %s Hook\r\n", function_name.c_str());

						unsigned char* backup_page_context = g_EptHooks[function_name]->backup_page_context;

						g_EptHooks.erase(function_name);  //先从容器里剔除

						//然后遍历一次,看看还有没有一样的页面
						int SamePageCount = 0;

						for (auto v : g_EptHooks)
						{
							if (v.second->target_page_address == target_page_address)
							{
								SamePageCount += 1;
								LOG_DEBUG("发现相同页面还存在Hook\r\n");
							}
						}

						if (SamePageCount == 0)
						{
							LOG_DEBUG("没有其他页面 需要释放备份的内存\r\n");
							r = vmx_Cloak_Deactivate(PA);
							if (r == 0)
							{
								if (backup_page_context)
								{
									free(backup_page_context);
									backup_page_context = NULL;
								}
							}
						}
					}
				}
			}
		}
	}

#pragma pack(push,1)
	struct PushRetCode
	{

		unsigned char push = 0x68;
		ULONG lowAddr;				// push xxxxxxxx
		unsigned char op1 = 0xc7;
		unsigned char op2 = 0x44;
		unsigned char op3 = 0x24;
		unsigned char op4 = 0x04;
		ULONG highAddr;
		unsigned char ret = 0xc3;
	};

	struct JmpCode 
	{
		unsigned char jmp_op1 = 0xFF;
		unsigned char jmp_op2 = 0x25;
		unsigned char jmp_op3 = 0x00;
		unsigned char jmp_op4 = 0x00;
		unsigned char jmp_op5 = 0x00;
		unsigned char jmp_op6 = 0x00;
		ULONG_PTR Addr;
	};
#pragma pack(pop)

	BOOL HookFunction(const eastl::string& function_name, void* target_address, void* fake_function, PVOID* trampoline_address, HookCodeType hookcode_type)
	{
		BOOL bRet = FALSE;

		//LOG_DEBUG("target_address:%p: fake_function:%p trampoline_address:%p\r\n", target_address, fake_function, trampoline_address);
		uint32_t hookcode_length = 0;
		unsigned char* hookcode = nullptr;
		//------------------
		uint8_t cc_code[] = { 0xCC };
		//------------------
		uint8_t ud_code[] = { 0x0F, 0x0B };

		PushRetCode pushret_code;
		ULONG_PTR addr = (ULONG_PTR)fake_function;
		pushret_code.lowAddr = addr & 0xFFFFFFFF;
		pushret_code.highAddr = (addr >> 32) & 0xFFFFFFFF;

		//------------------
		JmpCode jmp_code;
		jmp_code.Addr = (ULONG_PTR)fake_function;
		//------------------

		switch (hookcode_type)
		{
		case cc_type:      //断点挂钩方式
			hookcode_length = sizeof(cc_code);
			hookcode = cc_code;
			break;
		case ud_type:      //UD挂钩方式
			hookcode_length = sizeof(ud_code);
			hookcode = ud_code;
			break;
		case pushret_type: //pushret挂钩方式
			hookcode_length = sizeof(pushret_code);
			hookcode = (unsigned char*)&pushret_code;
			break;
		case jmp_type:     //jmp挂钩方式
			hookcode_length = sizeof(jmp_code);
			hookcode = (unsigned char*)&jmp_code;
			break;
		default:
			break;
		}

		// Initialize decoder context.
		{
			ZydisDecoder decoder;
			ZydisDecoderInit(
				&decoder,
				ZYDIS_MACHINE_MODE_LONG_64,
				ZYDIS_STACK_WIDTH_64);
			// Initialize formatter. Only required when you actually plan to
			// do instruction formatting ("disassembling"), like we do here.
			ZydisFormatter formatter;
			ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
			uint64_t instructionPointer;
			ZydisDecodedInstruction instruction;

			*trampoline_address = ConstructDetour(instructionPointer, target_address, decoder, instruction, hookcode_length);
		}

		LOG_DEBUG("function_name:%s target_address:0x%p fake_function:0x%p trampoline_address:0x%p \r\n", function_name.c_str(), target_address, fake_function, *trampoline_address);

		//Ept无痕挂钩
		bRet = EptHook(function_name, target_address, fake_function, *trampoline_address, hookcode, hookcode_length);

		return bRet;
	}

	void UnHookFunction(const eastl::string& function_name)
	{
		EptUnHook(function_name);
	}
}
