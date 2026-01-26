#pragma once

namespace symbolic_access
{
	eastl::vector<uint8_t> GetRegistryData(eastl::wstring_view RegistryPath, eastl::wstring_view RegistryKeyName);
}