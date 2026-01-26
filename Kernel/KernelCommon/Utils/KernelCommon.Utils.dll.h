#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		_IRQL_requires_max_(PASSIVE_LEVEL)
			NTSTATUS NTAPI RtlGetKnownDllExtents(
				_In_ PUNICODE_STRING SectionName,
				_Out_ PVOID* BaseAddress,
				_Out_ SIZE_T* ImageSize
			);

		PVOID Load_Dll(eastl::wstring FileName);

		VOID Free_Dll(HANDLE hMod);

		ULONG LdrGetImageSize(PVOID ImageBase);
	}
}
