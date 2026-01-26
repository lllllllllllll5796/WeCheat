#pragma once

#include "WeDebug.Config.h"
#include "KernelCommon.h"

#include "RequestType.h"
#include "RequestData.h"

#include "WeDebug.List.h"
#include "WeDebug.Global.h"


#define CHECK_FUNC_WITHOUT_RETURN(X) \
if (Global::X == NULL)\
{\
	LOG_DEBUG("%s 未初始化，无法继续\r\n",#X);\
	return;\
}