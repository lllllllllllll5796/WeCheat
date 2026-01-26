#include "pch.h"
#include "dllmain.h"
#include "Global.h"
#include "FunctionSet.h"
#include "Ioctl.h"

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		InitGlobalVariables();
		InitFunction();
		if (InitializeDevice())
		{
			InstallHook();
			Test();
			InitSymbols();
			InitDebug();
		}
		else
		{
			LOG_DEBUG("[-] 初始化设备失败\n");
		}
	}
	break;
	case DLL_PROCESS_DETACH:
		UnInstallHook();
		break;
	}

	return TRUE;
}

void InitGlobalVariables()
{
	Global::g_ProcessInfo = { 0 };
	Global::g_dwNumberOfProcessors = GetNumberOfProcessors();
	LOG_DEBUG("[+] CPU数量:%d\n", Global::g_dwNumberOfProcessors);
}

void InitFunction()
{
	Global::BaseThreadInitThunk = GetProcAddress(GetModuleHandleA(oxorany("kernel32.dll")), "BaseThreadInitThunk");
	LOG_DEBUG("[+] BaseThreadInitThunk:0x%p\n", Global::BaseThreadInitThunk);
}

//连接驱动
HANDLE CreateDeviceHandle()
{
	DWORD error = 0;
	return CreateFileW(L"\\\\.\\WeDebug", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

//初始化设备
BOOL InitializeDevice()
{
	Global::g_hGeneralDriverDevice = CreateDeviceHandle();

	if (Global::g_hGeneralDriverDevice == INVALID_HANDLE_VALUE)
	{
		LOG_DEBUG("[-] 连接驱动失败 error: %d\n", GetLastError());
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL UnInitializeDevice()
{
	if (Global::g_hGeneralDriverDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(Global::g_hGeneralDriverDevice);
		Global::g_hGeneralDriverDevice = INVALID_HANDLE_VALUE;
	}
}

void InstallHook()
{
	Hook_NtDebugActiveProcess();
	Hook_DbgUiIssueRemoteBreakin();
	Hook_WaitForDebugEvent();
	Hook_ContinueDebugEvent();
	Hook_OutputDebugStringA();
	Hook_OutputDebugStringW();
	Hook_SetThreadContext();
 	Hook_GetThreadContext();
 	Hook_WriteProcessMemory();
 	Hook_ReadProcessMemory();
 	Hook_VirtualProtectEx();

	LOG_DEBUG("[+] 初始化成功，钩子安装完毕\n");
}

void UnInstallHook()
{
	UnHook_NtDebugActiveProcess();
	UnHook_DbgUiIssueRemoteBreakin();
	UnHook_WaitForDebugEvent();
	UnHook_ContinueDebugEvent();
	UnHook_OutputDebugStringA();
	UnHook_OutputDebugStringW();
	UnHook_SetThreadContext();
 	UnHook_GetThreadContext();
 	UnHook_WriteProcessMemory();
 	UnHook_ReadProcessMemory();
 	UnHook_VirtualProtectEx();
}