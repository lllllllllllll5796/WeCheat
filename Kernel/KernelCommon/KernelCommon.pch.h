#pragma once

#define ENABLE_LOG 0
#define ENABLE_TRACE 1

#define CODE_NOTOBF __attribute((__annotate__(("notobf"))))
#define CODE_OBF __attribute((__annotate__(("needobf"))))
#define CODE_OBF_FLT  __attribute((__annotate__(("needobf flt"))))
#define CODE_OBF_MBA_FAST __attribute((__annotate__(("needobf const_subs"))))
#define CODE_OBF_MBA_ONEBIT  __attribute((__annotate__(("needobf const_enc_subs"))))
#define CODE_OBF_MBA_TWOBIT  __attribute((__annotate__(("needobf const_enc_ex_subs"))))

#define CODE_OBF_BB2F  __attribute((__annotate__(("needobf const_subs bb2func"))))
#define CODE_OBF_MFLT  __attribute((__annotate__(("needobf const_subs flt"))))
#define CODE_OBF_MAX  __attribute((__annotate__(("needobf split flt const_enc_ex_subs ipobf indbr indcall indgv"))))

// #pragma comment(linker, "/MERGE:.rdata=INIT")
// #pragma comment(linker, "/MERGE:.pdata=INIT")

#define _CRT_ALLOCATION_DEFINED

#define ASMJIT_EMBED 1
#define ASMJIT_BUILD_X86 1

#pragma warning(disable:4005)

#include "KernelCommon.cheader.h"
#include "KernelCommon.cppheader.h"
#include "KernelCommon.macro.h"
#include "KernelCommon.version.h"
#include "KernelIntrin.h"


#include <Zycore/Status.h>
#include <Zycore/Types.h>
#include <zydis/include/Zydis/Zydis.h>

#include "Log/Log.h"

#include "oxorany/oxorany.h"

#include "Wdk2/KernelCommon.Wdk2.h"

#include "Wdk/KernelCommon.Wdk.h"

#include "UnDoc/KernelCommon.UnDoc.h"

#include "Dbgk/KernelCommon.Dbgk.h"

#include "Lock/KernelCommon.Lock.h"

#include "KernelCommon.FuncDef.h"

#include "Kcpp/KernelCommon.Kcpp.h"

#include "CrossSystem/KernelCommon.CrossSystem.h"

#include "Driver/KernelCommon.Driver.h"

#include "Utils/KernelCommon.Utils.h"

#include "ia32-doc/ia32.hpp"

#include "Log2File/KernelCommon.Log2File.h"

#include "SerialPort/SerialPort.h"

#include "Encrypt/KernelCommon.Encrypt.Blowfish.h"

#include "VmxHelper/KernelCommon.VmxHelper.h"

namespace phnt
{
#include "phnt/phnt.h"
}

__declspec(selectany) PVOID g_KernelBase;
__declspec(selectany) ULONG g_CurrentWindowsBuildNumber;
__declspec(selectany) PEPROCESS g_KernelProcess;
__declspec(selectany) PEPROCESS g_ControlProcess;




