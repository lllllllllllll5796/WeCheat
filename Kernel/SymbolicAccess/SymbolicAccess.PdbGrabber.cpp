#include "SymbolicAccess.pch.h"
#include "SymbolicAccess.PdbGrabber.h"
#include "SymbolicAccess.Registry.h"

namespace symbolic_access
{
	struct PdbInfo
	{
		ULONG Signature;
		GUID Guid;
		ULONG Age;
		char PdbFileName[1];
	};

	PdbGrabber::PdbGrabber(eastl::unique_ptr<ProcessCreatorInterface>&& ProcessCreator) : m_ProcessCreator(std::move(ProcessCreator))
	{
		const auto& systemRootData = GetRegistryData(LR"(\REGISTRY\MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion)", L"SystemRoot");
		if (systemRootData.empty())
			return;

		for (auto string : { &m_DllPath, &m_SymbolsDir, &m_CurrentDir, &m_PowershellPath })
			std::replace(string->begin(), string->end(), L'C', *reinterpret_cast<const wchar_t*>(&systemRootData[0]));
	}

	eastl::optional<FileStream> PdbGrabber::GetPdbFileStream(size_t ModuleBaseAddress)
	{
		const auto& [pdbName, guidPlusAge] = GetPdbNameAndGuidPlusAge(ModuleBaseAddress);
		if (pdbName.empty() || guidPlusAge.empty())
		{
			PrintDbg("PdbName or GuidPlusAge is empty\n");
			return {};
		}

		const auto& pdbPath = CreatePdbFilePath(pdbName, guidPlusAge);
		if (pdbPath.empty())
		{
			PrintDbg("PdbPath is empty\n");
			return {};
		}

		auto fileStream = FileStream(pdbPath);
		if (!fileStream)
		{
			if (!m_ProcessCreator)
			{
				PrintDbg("ProcessCreator is null and there is no pdb on a disk\n");
				return {};
			}

			const auto& [processHandle, threadHandle] = m_ProcessCreator->CreateUmProcess(
				m_PowershellPath,
				CreateDownloadCommand(pdbName, guidPlusAge, eastl::wstring_view{ pdbPath.data() + pdbPath.find_first_of(LR"(C)") }),
				m_CurrentDir,
				m_DllPath);

			if (!processHandle || !threadHandle)
				return {};

			processHandle.WaitFor();

			fileStream = FileStream(pdbPath);
			if (!fileStream)
			{
				PrintDbg("Couldn't create handle to pdb file %S\n", pdbPath.data());
				return {};
			}
		}

		return fileStream;
	}

	_Use_decl_annotations_ EXTERN_C int __cdecl __my_stdio_common_vswprintf_s(
		unsigned __int64 _Options, wchar_t* _Buffer, size_t _BufferCount,
		const wchar_t* _Format, _locale_t _Locale, va_list _ArgList)
	{
		UNREFERENCED_PARAMETER(_Options);
		UNREFERENCED_PARAMETER(_Locale);

		// Calls _vsnwprintf exported by ntoskrnl
		using _vsnwprintf_type =
			int __cdecl(wchar_t*, size_t, const wchar_t*, va_list);
		static _vsnwprintf_type* local__vsnwprintf = nullptr;
		if (!local__vsnwprintf)
		{
			UNICODE_STRING proc_name_U = {};
			RtlInitUnicodeString(&proc_name_U, L"_vsnwprintf");
			local__vsnwprintf = reinterpret_cast<_vsnwprintf_type*>(
				MmGetSystemRoutineAddress(&proc_name_U));
		}

		return local__vsnwprintf(_Buffer, _BufferCount, _Format, _ArgList);
	}

	_Success_(return >= 0)
	_Check_return_opt_
	_CRT_STDIO_INLINE int __CRTDECL _my_vswprintf_s_l(
		_Out_writes_(_BufferCount) _Always_(_Post_z_) wchar_t* const _Buffer,
		_In_                                          size_t         const _BufferCount,
		_In_z_ _Printf_format_string_params_(2)       wchar_t const* const _Format,
		_In_opt_                                      _locale_t      const _Locale,
		va_list              _ArgList
	)
	{
		int const _Result = __my_stdio_common_vswprintf_s(
			_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS,
			_Buffer, _BufferCount, _Format, _Locale, _ArgList);

		return _Result < 0 ? -1 : _Result;
	}

	_Success_(return >= 0)
	_CRT_STDIO_INLINE int __CRTDECL my_swprintf_s(
		_Out_writes_(_BufferCount) _Always_(_Post_z_) wchar_t* const _Buffer,
		_In_                                          size_t         const _BufferCount,
		_In_z_ _Printf_format_string_                 wchar_t const* const _Format,
		...)
	{
		int _Result;
		va_list _ArgList;
		__crt_va_start(_ArgList, _Format);
		_Result = _my_vswprintf_s_l(_Buffer, _BufferCount, _Format, NULL, _ArgList);
		__crt_va_end(_ArgList);
		return _Result;
	}


	eastl::pair<eastl::wstring, eastl::wstring> PdbGrabber::GetPdbNameAndGuidPlusAge(size_t ImageDosHeaderAddress)
	{
		const auto imageDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(ImageDosHeaderAddress);
		const auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(ImageDosHeaderAddress + imageDosHeader->e_lfanew);
		const auto imageDebugDir = reinterpret_cast<IMAGE_DEBUG_DIRECTORY*>(
			ImageDosHeaderAddress + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);

		const auto pdbInfo = reinterpret_cast<PdbInfo*>(ImageDosHeaderAddress + imageDebugDir->AddressOfRawData);
		eastl::wstring guidPlusAgeString(40, 0);

		my_swprintf_s(guidPlusAgeString.data(), guidPlusAgeString.size(), L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X%x",
			pdbInfo->Guid.Data1, pdbInfo->Guid.Data2, pdbInfo->Guid.Data3,
			pdbInfo->Guid.Data4[0], pdbInfo->Guid.Data4[1], pdbInfo->Guid.Data4[2], pdbInfo->Guid.Data4[3],
			pdbInfo->Guid.Data4[4], pdbInfo->Guid.Data4[5], pdbInfo->Guid.Data4[6], pdbInfo->Guid.Data4[7],
			pdbInfo->Age);

		guidPlusAgeString.resize(wcslen(guidPlusAgeString.data()));

		eastl::string_view pdbFileName{ pdbInfo->PdbFileName };

		return eastl::make_pair(KernelCommon::Utils::ConvertToWString(pdbFileName.data()), std::move(guidPlusAgeString));
	}

	eastl::wstring PdbGrabber::CreateDownloadCommand(eastl::wstring_view PdbName, eastl::wstring_view GuidPlusAge, eastl::wstring_view PdbFinalPath)
	{
		eastl::wstring result{};

		result += L"powershell.exe ";
		result += L"mkdir ";
		result += eastl::wstring{ eastl::wstring_view{ PdbFinalPath.data(), PdbFinalPath.find_last_of(LR"(\)") } };
		result += L"; $cli = New-Object System.Net.WebClient; \
		$cli.Headers['User-Agent'] = 'Microsoft-Symbol-Server/10.0.10036.206';\
		$cli.DownloadFile('http://msdl.microsoft.com/download/symbols/";
		result += eastl::wstring{ PdbName };
		result += L"/";
		result += eastl::wstring{ GuidPlusAge };
		result += L"/";
		result += eastl::wstring{ PdbName };
		result += L"', '";
		result += eastl::wstring{ PdbFinalPath };
		result += L"');";

		return result;
	}

	eastl::wstring PdbGrabber::CreatePdbFilePath(eastl::wstring_view PdbName, eastl::wstring_view GuidPlusAge)
	{
		eastl::wstring path{};
		path += m_SymbolsDir;
		path += eastl::wstring{ PdbName };
		path += LR"(\)";
		path += eastl::wstring{ GuidPlusAge };
		path += LR"(\)";
		path += eastl::wstring{ PdbName };
		return path;
	}
}