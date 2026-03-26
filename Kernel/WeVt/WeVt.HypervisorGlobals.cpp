#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "WeVt.Trace.h"
#include "WeVt.HypervisorGlobals.tmh"

__pseudo_descriptor64 g_gdtr = { 0 };
__pseudo_descriptor64 g_idtr = { 0 };
unsigned __int64 g_guest_cr0;
unsigned __int64 g_guest_cr3;
unsigned __int64 g_guest_cr4;
__vmm_context g_vmm_context;
uint16_t guest_vpid = 1;
int eptWatchListSize;
int eptWatchListPos;
volatile long eptWatchList_lock = 0;

EPTWatchEntry eptWatchList[EPTWATCHLISTSIZE];

namespace hv
{
	void InitGlobalVariables()
	{
		//g_guest_cr3 = __readcr3();
		//__sgdt(&g_gdtr);                                 // 将当前逻辑处理器的gdt存储在全局变量g_gdtr中
		//__sidt(&g_idtr);                                 // 将当前逻辑处理器的idt存储在全局变量g_idtr中
		//g_guest_cr0 = __readcr0();
		//g_guest_cr4 = __readcr4();
	}
}