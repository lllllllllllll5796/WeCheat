#include "SymbolicAccess.pch.h"
#include "SymbolicAccess.ProcessCreatorInterface.h"
#include "SymbolicAccess.ProcessCreatorFactory.h"
#include "SymbolicAccess.ProcessCreator.h"

namespace symbolic_access
{
	namespace
	{
		template <typename T>
		PEPROCESS PidToProcess(T Pid)
		{
			PEPROCESS Process;
			PsLookupProcessByProcessId((HANDLE)Pid, &Process);
			return Process;
		}

		PEPROCESS GetProcessByName(eastl::wstring_view ProcessName)
		{
			ULONG processInfoSize{};
			ZwQuerySystemInformation(SystemProcessInformation, 0, 0, &processInfoSize);
			const auto buffer = eastl::make_unique<char[]>(processInfoSize + 0x1000);
			if (!buffer)
				return {};

			if (!NT_SUCCESS(ZwQuerySystemInformation(SystemProcessInformation, buffer.get(), processInfoSize, &processInfoSize)))
				return {};

			const auto processInfo = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(buffer.get());
			for (auto entry = processInfo; entry->NextEntryOffset; entry = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(reinterpret_cast<size_t>(entry) + entry->NextEntryOffset))
			{
				if (entry->ImageName.Buffer && eastl::wstring_view{ entry->ImageName.Buffer }.find(ProcessName) != eastl::wstring_view::npos)
					return PidToProcess(entry->UniqueProcessId);
			}

			return {};
		}
	}

	eastl::unique_ptr<ProcessCreatorInterface> ProcessCreatorFactory::Create()
	{
		UNICODE_STRING zwFunctionName{};
		RtlInitUnicodeString(&zwFunctionName, L"ZwCreateTransactionManager");

#ifdef _X86_
		const auto zwCreateUserProcess = reinterpret_cast<size_t>(MmGetSystemRoutineAddress(&zwFunctionName)) - 0x14;
#elif defined (_AMD64_) || defined(_M_ARM64 )
		const auto zwCreateUserProcess = reinterpret_cast<size_t>(MmGetSystemRoutineAddress(&zwFunctionName)) + 0x20;
#endif
		if (!zwCreateUserProcess)
		{
			PrintDbg("ZwCreateUserProcess address is null\n");
			return {};
		}

		const auto csrssProcess = GetProcessByName(oxorany(L"csrss.exe"));
		if (!csrssProcess)
		{
			PrintDbg("Failed to get csrss process\n");
			return {};
		}

		return eastl::unique_ptr<ProcessCreatorInterface>(new ProcessCreator(csrssProcess, zwCreateUserProcess));
	}
}