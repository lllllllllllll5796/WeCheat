#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		PVOID RtlAllocateMemory(bool InZeroMemory, SIZE_T InSize);

		//±ØÐëÒª¹Ò¿¿
		PVOID AllocateInjectMemory(IN HANDLE ProcessHandle, IN PVOID DesiredAddress, IN SIZE_T DesiredSize);

		NTSTATUS FreeInjectMemory(IN HANDLE ProcessHandle, IN PVOID* BaseAddress);

		NTSTATUS RtlSuperCopyMemory(IN VOID UNALIGNED* Destination, IN CONST VOID UNALIGNED* Source, IN ULONG Length);

		BOOL WriteReadCopy(void* Destination, void* Source, SIZE_T Length);

		void* MmMemSearch(const void* SearchBase,
			SIZE_T SearchSize,
			const void* Pattern,
			SIZE_T PatternSize);
	}
}