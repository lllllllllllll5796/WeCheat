#pragma once
#include <wmilib.h>

namespace wdk
{
    enum WMI_CLOCK_TYPE
    {
        WmiClockTypeDefault,
        WmiClockTypeSystemTime,
        WmiClockTypePerfCounter,
        WmiClockTypeProcess,
        WmiClockTypeThread,
        WmiClockTypeCPUCycle,
    };
}