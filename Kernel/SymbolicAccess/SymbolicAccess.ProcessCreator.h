#pragma once

#include "SymbolicAccess.ProcessCreatorInterface.h"


namespace symbolic_access
{
	class ProcessCreator : public ProcessCreatorInterface
	{
	public:
		ProcessCreator(PEPROCESS CsrssProcess, size_t ZwCreateUserProcessAddress);

		eastl::pair<ScopedHandle, ScopedHandle> CreateUmProcess(eastl::wstring_view ImagePath,
			eastl::wstring_view CommandLine, eastl::wstring_view CurrentDirectory, eastl::wstring_view DllPath) override;

	private:
		const size_t m_MaxCurDirSize = 0x208;
		PEPROCESS m_CsrssProcess;
		size_t m_ZwCreateUserProcess;
	};
}