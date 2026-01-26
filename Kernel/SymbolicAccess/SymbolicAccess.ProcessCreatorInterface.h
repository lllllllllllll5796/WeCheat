#pragma once

#include "SymbolicAccess.ScopedHandle.h"
// #include <utility>
// #include <string_view>

namespace symbolic_access
{
	class ProcessCreatorInterface
	{
	public:
		virtual ~ProcessCreatorInterface() = default;

		virtual eastl::pair<ScopedHandle, ScopedHandle> CreateUmProcess(eastl::wstring_view ImagePath,
			eastl::wstring_view CommandLine, eastl::wstring_view CurrentDirectory, eastl::wstring_view DllPath) = 0;
	};
}