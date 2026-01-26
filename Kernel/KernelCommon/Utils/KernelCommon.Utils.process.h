#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		ULONG GetProcessIdByProcessHandle(HANDLE ProcessHandle);
		PEPROCESS GetProcessByProcessHandle(HANDLE ProcessHandle);
		HANDLE GetProcessHandleByEProcess(PEPROCESS Process);
		ULONG GetProcessIdFromThreadHandle(HANDLE ThreadHandle);
		HANDLE GetProcessId(PWCHAR ProcessName);
		BOOL IsHandleValid(HANDLE hObject, BOOL bThread);
		HANDLE GetProcessHandleByProcessId(IN HANDLE ProcessId);
		BOOL GetProcessInfo(CONST CHAR* Name, ULONG64& ImageSize, PVOID& ImageBase);
	}
}