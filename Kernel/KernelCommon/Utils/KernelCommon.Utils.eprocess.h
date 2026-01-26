#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		PEPROCESS GetProcessByName(const WCHAR* ProcessName);

		PEPROCESS GetCsrssProcess();

		PEPROCESS GetEProcessByProcessId(HANDLE ProcessId);

		eastl::wstring GetProcessFullName(PEPROCESS Process);

		UNICODE_STRING PsQueryFullProcessImageName(PEPROCESS TargetProcess);

		PEPROCESS GetEProcessByProcessName(wchar_t* ImageName);

		BOOL IsWow64Process(PEPROCESS Process);

		PVOID GetProcessPebWow64(PEPROCESS Process);

		PVOID GetProcessPeb64(PEPROCESS Process);

		PVOID GetProcessDebugPort(_In_ PEPROCESS Process);

	}
}