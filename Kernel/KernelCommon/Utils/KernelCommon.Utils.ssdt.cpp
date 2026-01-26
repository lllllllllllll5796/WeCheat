#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.ssdt.h"

namespace KernelCommon
{
	namespace Utils
	{
		ULONG GetSSDTSyscallNumber(const eastl::string& FunctionName)
		{
			ULONG Index = -1;
			auto NtdllBase = Load_Dll(eastl::wstring(oxorany(L"\\SystemRoot\\System32\\ntdll.dll")));

			if (!NtdllBase)
			{
				LOG_DEBUG("获得NtDll地址失败\r\n");
				return ULONG(-1);
			}

			//LOG_DEBUG("NtdllBase:%p FunctionName:%s\r\n", NtdllBase, FunctionName.c_str());

			auto rva = GetProcAddressFromPEExport(NtdllBase, FunctionName);
			if (rva)
			{
				//LOG_DEBUG("执行到这里了 rva:0x%llX\r\n", rva);

				PUCHAR Func = (PUCHAR)NtdllBase + rva;

				//LOG_DEBUG("Func:0x%llX\r\n", Func);
#ifdef _X86_
			// check for mov eax,imm32
				if (*Func == 0xB8)
				{
					// return imm32 argument (syscall numbr)
					Index = *(PULONG)((PUCHAR)Func + 1);
				}
#elif _AMD64_
			// check for mov eax,imm32
				if (*(Func + 3) == 0xB8)
				{
					// return imm32 argument (syscall numbr)
					Index = *(PULONG)(Func + 4);
				}
#endif
			}
			if (NtdllBase)
				Free_Dll(NtdllBase);

			return Index;
		}
	}
}