#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.memory.h"

namespace KernelCommon
{
	namespace Utils
	{
		PVOID RtlAllocateMemory(bool InZeroMemory, SIZE_T InSize)
		{
			void* Result = ExAllocatePoolWithTag(NonPagedPool, InSize, GetPoolTag());
			if (InZeroMemory && (Result != NULL))
				RtlZeroMemory(Result, InSize);
			return Result;
		}

		VOID RtlFreeMemory(void* InPointer)
		{
			ExFreePool(InPointer);
		}

		PVOID AllocateInjectMemory(IN HANDLE ProcessHandle, IN PVOID DesiredAddress, IN SIZE_T DesiredSize)
		{
			wdk::MEMORY_BASIC_INFORMATION mbi;
			SIZE_T AllocateSize = DesiredSize;

			if ((ULONG_PTR)DesiredAddress >= 0x70000000 && (ULONG_PTR)DesiredAddress < 0x80000000)
				DesiredAddress = (PVOID)0x70000000;

			while (1)
			{
				if (!NT_SUCCESS(ZwQueryVirtualMemory(ProcessHandle,
					DesiredAddress,
					(::MEMORY_INFORMATION_CLASS)0,
					&mbi, sizeof(mbi), NULL)))
				{
					LOG_DEBUG("faield QueryVirtualMemory\r\n");
					return NULL;
				}

				if (DesiredAddress != mbi.AllocationBase)
				{
					DesiredAddress = mbi.AllocationBase;
				}
				else
				{
					DesiredAddress = (PVOID)((ULONG_PTR)mbi.AllocationBase - 0x10000);
				}

				if (mbi.State == MEM_FREE)
				{
					if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, &mbi.BaseAddress, 0, &AllocateSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
					{
						if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, &mbi.BaseAddress, 0, &AllocateSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE)))
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
			NTSTATUS STATE = NT_SUCCESS(ZwFreeVirtualMemory(ProcessHandle, BaseAddress, &allocSize, MEM_RELEASE));
			return STATE;
		}

		NTSTATUS RtlSuperCopyMemory(IN VOID UNALIGNED* Destination, IN CONST VOID UNALIGNED* Source, IN ULONG Length)
		{
			KIRQL oldIrql;
			KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

			PMDL mdl = IoAllocateMdl(Destination, Length, FALSE, FALSE, nullptr);
			if (mdl == nullptr) {
				KeLowerIrql(oldIrql);
				return STATUS_NO_MEMORY;
			}

			MmBuildMdlForNonPagedPool(mdl);
			// Hack: prevent bugcheck from Driver Verifier and possible future version of Windows
#pragma prefast(push)
	// Disables the warnings specified in a given warning list.
#pragma prefast(disable:__WARNING_MODIFYING_MDL,"Trust me I'm a scientist")	
			CSHORT flags = mdl->MdlFlags;
			mdl->MdlFlags |= MDL_PAGES_LOCKED;
			mdl->MdlFlags &= ~MDL_SOURCE_IS_NONPAGED_POOL;

			// Map pages and do the copy
			PVOID mapped = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmCached, nullptr, FALSE, HighPagePriority);
			if (mapped == nullptr) {
				mdl->MdlFlags = flags;
				IoFreeMdl(mdl);
				KeLowerIrql(oldIrql);
				return STATUS_NONE_MAPPED;
			}

			RtlCopyMemory(mapped, Source, Length);

			MmUnmapLockedPages(mapped, mdl);
			mdl->MdlFlags = flags;
#pragma prefast(pop)

			IoFreeMdl(mdl);
			KeLowerIrql(oldIrql);

			return STATUS_SUCCESS;
		}

		BOOL WriteReadCopy(void* Destination, void* Source, SIZE_T Length)
		{
			auto mdl = IoAllocateMdl(Destination, static_cast<ULONG>(Length), FALSE, FALSE, nullptr);

			// 		const std::unique_ptr<MDL, decltype(&IoFreeMdl)> mdl(
			// 			IoAllocateMdl(destination, static_cast<ULONG>(size), FALSE, FALSE, nullptr),
			// 			&IoFreeMdl
			// 		);

			if (!mdl)
				return FALSE;

			MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);

			const auto mapped_page = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, nullptr, FALSE, NormalPagePriority);

			if (!mapped_page)
			{
				IoFreeMdl(mdl);
				return FALSE;
			}

			if (!NT_SUCCESS(MmProtectMdlSystemAddress(mdl, PAGE_EXECUTE_READWRITE)))
			{
				IoFreeMdl(mdl);
				return FALSE;
			}

			memcpy(mapped_page, Source, Length);

			MmUnmapLockedPages(mapped_page, mdl);
			MmUnlockPages(mdl);
			IoFreeMdl(mdl);
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
