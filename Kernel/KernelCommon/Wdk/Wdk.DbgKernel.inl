#pragma once
#include "Wdk.DbgStruct.inl"

namespace wdk
{
    extern"C"
    {
        inline auto DbgInitSystem() -> NTSTATUS
        {
            return STATUS_SUCCESS;
        }
    }
}
