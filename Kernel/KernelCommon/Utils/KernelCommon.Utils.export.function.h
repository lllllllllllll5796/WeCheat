#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		//---------------3环----------------
		PVOID GetExportedFunctionAddress(PEPROCESS TargetProcess, PVOID ModuleBase, CONST CHAR* ExportedFunctionName);

		ULONG_PTR GetProcAddressFromPEExport(PVOID Image, const eastl::string& FunctionName);

		ULONG_PTR GetProcAddressR(ULONG_PTR hModule, const char* lpProcName, BOOL x64Module);

		ULONG64 GetEProcessFunctionAddress(PEPROCESS PEProcess, PWCHAR MoudleName, PCHAR Function);   //<---需要测试一下
		//---------------0环----------------
		ULONG_PTR GetSystemRoutineAddressByName(PWCHAR funcNameStr);

		PVOID GetSystemRoutineAddress(const eastl::string& FunctionName);
	}
}