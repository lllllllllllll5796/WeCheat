#pragma once

namespace wdk
{
#pragma region AccessMask
    enum DbgkAccessMask : ACCESS_MASK
    {
        DebugReadEvent          = 0x0001,
        DebugProcessAssign      = 0x0002,
        DebugSetInformation     = 0x0004,
        DebugQueryInformation   = 0x0008,

        DebugAllAccess = StandardRightsRequired | Synchronize | 
            DebugReadEvent | DebugProcessAssign | DebugSetInformation | DebugQueryInformation
    };
#pragma endregion
}
