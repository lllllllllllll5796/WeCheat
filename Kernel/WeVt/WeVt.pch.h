#pragma once

#include "WeVt.Config.h"
#include "KernelCommon.h"

#include "RequestType.h"
#include "RequestData.h"
#include "WeVt.Global.h"

#include <Zycore/Status.h>
#include <Zycore/Types.h>
#include <zydis/include/Zydis/Zydis.h>

#include "VT/VT.h"
#include "VT/VT.InterruptHook.h"

#include "VT/VT.Util.h"
// #include "CE/CE.NoExceptions.h"
// #include "CE/CE.Ultimap.h"
// #include "CE/CE.Ultimap2.h"
// #include "CE/CE.ProcessList.h"
// #include "CE/CE.MemScan.h"
// #include "CE/CE.Thread.h"

#include "VT/VT.Debugger.h"
#include "VT/VT.Apic.h"
#include "VT/VT.VmxOffLoad.h"

#include "VT/VT.h"
#include "VT/VT.BreakPoint.h"
#include "VT/VT.Watch.h"
#include "VT/VT.Ultimap.h"
#include "VT/VT.RedirectInterrupt.h"
#include "VT/VT.Test.h"