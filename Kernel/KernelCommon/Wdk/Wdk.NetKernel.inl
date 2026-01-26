#pragma once
#include "Wdk.NetStruct.inl"


namespace wdk
{
    extern"C"
    {
        inline auto NetInitSystem() -> NTSTATUS
        {
            return STATUS_SUCCESS;
        }
    }
}
