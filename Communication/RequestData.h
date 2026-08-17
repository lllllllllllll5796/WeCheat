#pragma once

#ifndef _KERNEL_MODE
#include "RequestType.h"
#endif //_KERNEL_MODE

typedef struct _REQUEST_Test
{
	IN DWORD64 InValue;
	OUT DWORD64 OutValue;
} REQUEST_Test, * PREQUEST_Test;
