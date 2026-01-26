#include "pch.h"
#include "Hook.h"
#include "detours.h"
#include "Global.h"

//安装Hook
void HookOn(_In_ PVOID* pfun, _In_ PVOID proxy_fun, _In_ HANDLE hThread)
{
	//修改目标内存页保护属性
	DetourTransactionBegin();
	//暂停目标线程
	DetourUpdateThread(hThread);
	//开始Hook
	DetourAttach(pfun, proxy_fun);
	//提交执行
	DetourTransactionCommit();
}

//卸载Hook
void HookOff(_In_ PVOID* pfun, _In_ PVOID proxy_fun, _In_ HANDLE hThread)
{
	//修改目标内存页保护属性
	DetourTransactionBegin();
	//暂停目标线程
	DetourUpdateThread(hThread);
	//卸载Hook
	DetourDetach(pfun, proxy_fun);
	//提交执行
	DetourTransactionCommit();
}