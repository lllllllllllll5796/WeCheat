#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		//---------------3»·----------------
		PVOID GetUserModeModule(PEPROCESS TargetProcess, CONST WCHAR* ModuleName, BOOLEAN IsWow64);

		BOOL GetSectionData(CONST CHAR* ImageName, CONST CHAR* SectionName, ULONG64& SectionSize, PVOID& SectionBaseAddress);

		PVOID GetModuleBase(PEPROCESS TargetProcess, const eastl::wstring& DllName, BOOL IsX64);

		PVOID AttachedProcess_GetModuleBase_x86(PEPROCESS pEProcess, PWCHAR pModuleName, OUT ULONG* SizeOfImage);

		PVOID AttachedProcess_GetModuleBase_x64(PEPROCESS pEProcess, PWCHAR pModuleName, OUT ULONG* SizeOfImage);

		NTSTATUS GetProcessModuleInfo(UINT64 ProcessID, WCHAR* DllName, OUT PULONG64 ModuleBase, OUT PULONG ModuleSize);

		PVOID GetModuleBaseBySystemApi(PEPROCESS TargetProcess, PWCHAR ModuleName);
		//---------------0»·----------------
		PVOID GetNtOsKernelBase();

		PVOID GetKernelBase(PULONG pImageSize);

		VOID GetSystemModuleBase(CHAR* ModuleName, ULONG64* pModuleBase, ULONG* pBufferSize);

		NTSTATUS GetModuleNameForAddress(IN PVOID ProcessVa, OUT PCHAR FileNameBuff);
	}
}