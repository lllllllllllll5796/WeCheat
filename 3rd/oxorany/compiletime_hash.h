#pragma once
#include <cstdint>

// FNV-1a hash function for compile-time
constexpr uint32_t
fnv1a_hash666(const char *str, std::size_t length, uint32_t hash = 2166136261u)
{
    for (size_t i = 0; i < length; ++i)
    {
        hash = (hash ^ str[i]) * 16777619u;
    }
    return hash;
}

constexpr uint32_t operator"" h(const char *str, size_t length)
{
    return fnv1a_hash666(str, length);
}
