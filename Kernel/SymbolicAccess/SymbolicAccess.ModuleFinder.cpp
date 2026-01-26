#include "SymbolicAccess.pch.h"
#include "SymbolicAccess.ModuleFinder.h"

extern "C"
NTKERNELAPI NTSTATUS NTAPI ZwQuerySystemInformation
(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

namespace symbolic_access
{
	// Must be with extension e.g. ntoskrnl.exe, win32k.sys, ntdll.dll
	size_t GetModuleAddress(eastl::wstring_view ModuleName)
	{
		ULONG moduleInfoSize{};
		ZwQuerySystemInformation(SystemModuleInformation, 0, moduleInfoSize, &moduleInfoSize);

		const auto buffer = eastl::make_unique<char[]>(moduleInfoSize + 1000);
		if (!buffer)
			return {};

		if (!NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, buffer.get(), moduleInfoSize, &moduleInfoSize)))
			return {};

		const auto mods = reinterpret_cast<RTL_PROCESS_MODULES*>(buffer.get());
		for (size_t i{}; i < mods->NumberOfModules; ++i)
		{
			eastl::string_view tmpPathName{ reinterpret_cast<char*>(&mods->Modules[i].FullPathName[0]) };
			
			eastl::wstring pathName = KernelCommon::Utils::ConvertToWString(tmpPathName.data());

			if (pathName.find(eastl::wstring{ ModuleName }) != eastl::wstring::npos)
				return reinterpret_cast<size_t>(mods->Modules[i].ImageBase);
		}

		return {};
	}
}