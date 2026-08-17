#pragma once

#include "WeDebug.Config.h"
#include "KernelCommon.h"

#include "RequestType.h"
#include "RequestData.h"
#include "../../Shared/SharedStruct.h"

#include "WeDebug.List.h"
#include "WeDebug.Global.h"
#include "WeDebug.DbgkCompat.h"
#include "WeDebug.Hypervisor.h"
#include "WeDebug.vmcall_reason.h"
#include "WeDebug.AsmVmcall.h"


#define CHECK_FUNC_WITHOUT_RETURN(X) \
if (Global::X == NULL)\
{\
	LOG_DEBUG("%s 函数指针为空，直接返回\r\n",#X);\
	return;\
}