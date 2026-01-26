#pragma once

#include "SymbolicAccess.ProcessCreatorInterface.h"

namespace symbolic_access
{
	class ProcessCreatorFactory
	{
	public:
		ProcessCreatorFactory() = delete;

		static eastl::unique_ptr<ProcessCreatorInterface> Create();
	};
}