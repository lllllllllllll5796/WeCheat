#pragma once

#ifndef _KERNEL_MODE
#include "RequestType.h"
#endif //_KERNEL_MODE

#define PTR64(x) ULONG64

typedef struct _USER_DATA
{
	ULONG Count;  //用户队列的最大计数
	ULONG uSize;  //数据大小
	PTR64(PVOID) pUserData;  //结构体指针
}USER_DATA, * PUSER_DATA;

//---------------------------
typedef struct _REQUEST_Test
{
	IN DWORD64 InValue;
	OUT DWORD64 OutValue;
} REQUEST_Test, * PREQUEST_Test;

typedef struct _RING3_PROCESS_CR3
{
	IN PTR64(HANDLE) ProcessHandle;
	OUT unsigned __int64 Cr3; //目标进程的cr3 
}RING3_PROCESS_CR3, * PRING3_PROCESS_CR3;
