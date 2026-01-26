#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		KIRQL RaiseIrql();

		VOID LowerIrql(KIRQL irql);

		KIRQL WPOFFx64();

		VOID WPONx64(KIRQL irql);

		eastl::string GetCpuID();

		SIZE_T AlignSize(SIZE_T nSize, UINT nAlign);

		BOOL IsKernelAddress(PVOID Address);

		BOOL ProbeUserAddress(PVOID addr, SIZE_T size, ULONG alignment);

		VOID AdjustRelativePointers(PBYTE buffer, PBYTE target, SIZE_T size);

		ULONG RtlNextRandom(ULONG Min, ULONG Max);

		ULONG GetPoolTag();

		//ËÑË÷ÌØÕ÷Âë
		PVOID FindSignature(PVOID Memory, ULONG64 Size, PCSZ Pattern, PCSZ Mask);

		//BlackBoneµÄËÑË÷ÌØÕ÷Âë
		NTSTATUS BBSearchPattern(IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound);

		PVOID FindPatternImage(PCHAR base, PCHAR pattern, PCHAR mask);

		VOID NewExInitializeFastMutex(_Out_ PFAST_MUTEX FastMutex);

		PWCHAR DosFullPathToModuleNameW(PWCHAR FullPath);

		ULONG GetPreviousModeOffset();

		KPROCESSOR_MODE KeSetPreviousMode(KPROCESSOR_MODE mode);

		NTSTATUS HidePCHDriver(PDRIVER_OBJECT a_self_driver_object);

		BOOL NtFileNameToDosFileName(IN PUNICODE_STRING us, OUT WCHAR* ws);

		ULONG GetSystemStartTime();

		NTSTATUS GetProcessName(IN PEPROCESS Process, OUT WCHAR* fileName);
	}
}
