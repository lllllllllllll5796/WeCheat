#pragma once

#ifndef _KERNEL_MODE
#include "RequestType.h"
#endif //_KERNEL_MODE

//#define PTR64(x) ULONG64
//
//
//
////---------------------------
//typedef struct _REQUEST_Test
//{
//	IN DWORD64 InValue;
//	OUT DWORD64 OutValue;
//} REQUEST_Test, * PREQUEST_Test;
//
//typedef struct _RING3_PROCESS_CR3
//{
//	IN PTR64(HANDLE) ProcessHandle;
//	OUT unsigned __int64 Cr3; //目标进程的cr3 
//}RING3_PROCESS_CR3, * PRING3_PROCESS_CR3;
