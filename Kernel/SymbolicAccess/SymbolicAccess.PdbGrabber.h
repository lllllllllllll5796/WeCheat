#pragma once

#include "SymbolicAccess.ProcessCreatorInterface.h"
#include "SymbolicAccess.FileStream.h"

namespace symbolic_access
{
	class PdbGrabber
	{
	public:
		PdbGrabber(eastl::unique_ptr<ProcessCreatorInterface>&& ProcessCreator);
		PdbGrabber(const PdbGrabber&) = delete;
		PdbGrabber& operator=(const PdbGrabber&) = delete;
		PdbGrabber(PdbGrabber&&) = default;
		PdbGrabber& operator=(PdbGrabber&&) = default;

		eastl::optional<FileStream> GetPdbFileStream(size_t ModuleBaseAddress);
	private:
		eastl::pair<eastl::wstring, eastl::wstring> GetPdbNameAndGuidPlusAge(size_t ImageDosHeaderAddress);

		eastl::wstring CreateDownloadCommand(eastl::wstring_view PdbName, eastl::wstring_view GuidPlusAge, eastl::wstring_view PdbFinalPath);

		eastl::wstring CreatePdbFilePath(eastl::wstring_view PdbName, eastl::wstring_view GuidPlusAge);

		eastl::unique_ptr<ProcessCreatorInterface> m_ProcessCreator;
		eastl::wstring m_DllPath = LR"(C:\WINDOWS\system32\WindowsPowerShell\v1.0;C:\Windows\system32;C:\Windows\system;C:\Windows;%SystemRoot%\system32\WindowsPowerShell\v1.0\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\)";
		eastl::wstring m_SymbolsDir = LR"(\??\C:\Symbols\)";
		eastl::wstring m_CurrentDir = LR"(C:\)";
		eastl::wstring m_PowershellPath = 
#ifdef _KERNEL_MODE
			LR"(\??\C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe)";
#else
			LR"(C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe)";
#endif
	};
}