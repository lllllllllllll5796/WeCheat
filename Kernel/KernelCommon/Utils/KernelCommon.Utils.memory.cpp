#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.memory.h"

namespace KernelCommon
{
	namespace Utils
	{
		PVOID RtlAllocateMemory(bool InZeroMemory, SIZE_T InSize)
		{
			void* Result = ImpCall(ExAllocatePoolWithTag, NonPagedPool, InSize, GetPoolTag());
			if (InZeroMemory && (Result != NULL))
				RtlZeroMemory(Result, InSize);
			return Result;
		}

		VOID RtlFreeMemory(void* InPointer)
		{
			ImpCall(ExFreePool, InPointer);
		}

		//必须要挂靠
		PVOID AllocateInjectMemory(IN HANDLE ProcessHandle, IN PVOID DesiredAddress, IN SIZE_T DesiredSize)
		{
			wdk::MEMORY_BASIC_INFORMATION mbi;
			SIZE_T AllocateSize = DesiredSize;

			//检查是否x86内核地址
			if ((ULONG_PTR)DesiredAddress >= 0x70000000 && (ULONG_PTR)DesiredAddress < 0x80000000)
				DesiredAddress = (PVOID)0x70000000;

			while (1)
			{
				if (!NT_SUCCESS(ImpCall(ZwQueryVirtualMemory, ProcessHandle,
					DesiredAddress,
					(::MEMORY_INFORMATION_CLASS)0,
					&mbi, sizeof(mbi), NULL)))
				{
					LOG_DEBUG("faield QueryVirtualMemory\r\n");
					return NULL;
				}
				//Windows会以64-KB为边界计算区域的启始地址 分配粒度为64kb 0x10000 16页
				if (DesiredAddress != mbi.AllocationBase)
				{
					DesiredAddress = mbi.AllocationBase;
				}
				else
				{
					DesiredAddress = (PVOID)((ULONG_PTR)mbi.AllocationBase - 0x10000);
				}
				//BaseAddress只是分配页面中的起始虚拟地址
				if (mbi.State == MEM_FREE)
				{
					//先设置为保留内存
					if (NT_SUCCESS(ImpCall(ZwAllocateVirtualMemory, ProcessHandle, &mbi.BaseAddress, 0, &AllocateSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
					{
						//提交到物理内存
						if (NT_SUCCESS(ImpCall(ZwAllocateVirtualMemory, ProcessHandle, &mbi.BaseAddress, 0, &AllocateSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE)))
						{
							//LOG_DEBUG("Addr :0x%llX \r\n", mbi.BaseAddress);
							return mbi.BaseAddress;
						}
					}
				}
			}
			return NULL;
		}

		NTSTATUS FreeInjectMemory(IN HANDLE ProcessHandle, IN PVOID* BaseAddress)
		{
			SIZE_T allocSize = 0;
			NTSTATUS STATE = NT_SUCCESS(ImpCall(ZwFreeVirtualMemory, ProcessHandle, BaseAddress, &allocSize, MEM_RELEASE));
			return STATE;
		}

		NTSTATUS RtlSuperCopyMemory(IN VOID UNALIGNED* Destination, IN CONST VOID UNALIGNED* Source, IN ULONG Length)
		{
			KIRQL oldIrql;
			KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

			PMDL mdl = ImpCall(IoAllocateMdl, Destination, Length, FALSE, FALSE, nullptr);
			if (mdl == nullptr) {
				KeLowerIrql(oldIrql);
				return STATUS_NO_MEMORY;
			}

			ImpCall(MmBuildMdlForNonPagedPool, mdl);
			// Hack: prevent bugcheck from Driver Verifier and possible future version of Windows
#pragma prefast(push)
	// Disables the warnings specified in a given warning list.
#pragma prefast(disable:__WARNING_MODIFYING_MDL,"Trust me I'm a scientist")	
			CSHORT flags = mdl->MdlFlags;
			mdl->MdlFlags |= MDL_PAGES_LOCKED;
			mdl->MdlFlags &= ~MDL_SOURCE_IS_NONPAGED_POOL;

			// Map pages and do the copy
			PVOID mapped = ImpCall(MmMapLockedPagesSpecifyCache, mdl, KernelMode, MmCached, nullptr, FALSE, HighPagePriority);
			if (mapped == nullptr) {
				mdl->MdlFlags = flags;
				ImpCall(IoFreeMdl, mdl);
				KeLowerIrql(oldIrql);
				return STATUS_NONE_MAPPED;
			}

			RtlCopyMemory(mapped, Source, Length);

			ImpCall(MmUnmapLockedPages, mapped, mdl);
			mdl->MdlFlags = flags;
#pragma prefast(pop)

			ImpCall(IoFreeMdl, mdl);
			KeLowerIrql(oldIrql);

			return STATUS_SUCCESS;
		}

		BOOL WriteReadCopy(void* Destination, void* Source, SIZE_T Length)
		{
			auto mdl = ImpCall(IoAllocateMdl, Destination, static_cast<ULONG>(Length), FALSE, FALSE, nullptr);

			// 		const std::unique_ptr<MDL, decltype(&IoFreeMdl)> mdl(
			// 			IoAllocateMdl(destination, static_cast<ULONG>(size), FALSE, FALSE, nullptr),
			// 			&IoFreeMdl
			// 		);

			if (!mdl)
				return FALSE;

			ImpCall(MmProbeAndLockPages, mdl, KernelMode, IoReadAccess);

			const auto mapped_page = ImpCall(MmMapLockedPagesSpecifyCache, mdl, KernelMode, MmNonCached, nullptr, FALSE, NormalPagePriority);

			if (!mapped_page)
			{
				ImpCall(IoFreeMdl, mdl);
				return FALSE;
			}

			if (!NT_SUCCESS(ImpCall(MmProtectMdlSystemAddress, mdl, PAGE_EXECUTE_READWRITE)))
			{
				ImpCall(IoFreeMdl, mdl);
				return FALSE;
			}

			memcpy(mapped_page, Source, Length);

			ImpCall(MmUnmapLockedPages, mapped_page, mdl);
			ImpCall(MmUnlockPages, mdl);
			ImpCall(IoFreeMdl, mdl);
			return TRUE;
		}

		void* MmMemSearch(const void* SearchBase,
			SIZE_T SearchSize,
			const void* Pattern,
			SIZE_T PatternSize)
		{
			if (PatternSize > SearchSize)
			{
				return nullptr;
			}
			auto searchBase = static_cast<const char*>(SearchBase);
			for (size_t i = 0; i <= SearchSize - PatternSize; i++)
			{
				if (!memcmp(Pattern, &searchBase[i], PatternSize))
				{
					return const_cast<char*>(&searchBase[i]);
				}
			}
			return nullptr;
		}



	}
}
