#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.eprocess.h"

namespace KernelCommon
{
	namespace Utils
	{
		PEPROCESS GetProcessByName(const WCHAR* ProcessName)
		{
			NTSTATUS Status;
			ULONG Bytes;

			ImpCall(ZwQuerySystemInformation, wdk::SYSTEM_INFORMATION_CLASS::SystemProcessInformation, NULL, NULL, &Bytes);
			wdk::PSYSTEM_PROCESS_INFORMATION ProcInfo = (wdk::PSYSTEM_PROCESS_INFORMATION)ImpCall(ExAllocatePool, NonPagedPool, Bytes);
			if (ProcInfo == NULL)
				return NULL;

			RtlSecureZeroMemory(ProcInfo, Bytes);

			Status = ImpCall(ZwQuerySystemInformation, wdk::SYSTEM_INFORMATION_CLASS::SystemProcessInformation, ProcInfo, Bytes, &Bytes);
			if (NT_SUCCESS(Status) == FALSE)
			{
				ImpCall(ExFreePool, ProcInfo);
				return NULL;
			}

			UNICODE_STRING ProcessImageName;
			ImpCall(RtlCreateUnicodeString, &ProcessImageName, ProcessName);

			for (wdk::PSYSTEM_PROCESS_INFORMATION Entry = ProcInfo; Entry->NextEntryOffset != NULL; Entry = (wdk::PSYSTEM_PROCESS_INFORMATION)((UCHAR*)Entry + Entry->NextEntryOffset))
			{
				if (Entry->ImageName.Buffer != NULL)
				{
					if (ImpCall(RtlCompareUnicodeString, &Entry->ImageName, &ProcessImageName, TRUE) == 0)
					{
						PEPROCESS CurrentPeprocess;
						ImpCall(PsLookupProcessByProcessId, (HANDLE)(Entry->UniqueProcessId), &CurrentPeprocess);
						ImpCall(ObfDereferenceObject, CurrentPeprocess);
						ImpCall(ExFreePool, ProcInfo);
						return CurrentPeprocess;
					}
				}
			}

			ImpCall(ExFreePool, ProcInfo);
			return NULL;
		}

		PEPROCESS GetCsrssProcess()
		{
			PEPROCESS Process = NULL;

			// Sometimes it doesn't return csrss process at the first try which is strange because it must exist
			do
			{
				Process = GetProcessByName(oxorany(L"csrss.exe"));
			} while (Process == NULL);

			return Process;
		}

		PEPROCESS GetEProcessByProcessId(HANDLE ProcessId)
		{
			PEPROCESS Process = NULL;
			if (NT_SUCCESS(ImpCall(PsLookupProcessByProcessId, ProcessId, &Process)))
			{
				ImpCall(ObfDereferenceObject, Process);
			}
			return Process;
		}

		eastl::wstring GetProcessFullName(PEPROCESS Process)
		{
			eastl::wstring strName;
			PUNICODE_STRING Image;
			if (Process)
			{
				auto ns = ImpCall(SeLocateProcessImageName, Process, &Image);

				if (NT_SUCCESS(ns))
				{
					if (ValidateUnicodeString(Image))
					{
						if (Image->Buffer != NULL)
						{
							wchar_t wstr[MAX_PATH] = { 0 };
							ImpCall(wcsncpy, wstr, Image->Buffer, Image->Length / 2);
							wstr[Image->Length / 2] = 0;
							//LOG_DEBUG("1111:%S\r\n", wstr);
							strName = wstr;
						}
					}
					ImpCall(ExFreePool, Image);
				}
			}
			return strName;
		}

		UNICODE_STRING PsQueryFullProcessImageName(PEPROCESS TargetProcess)
		{
			UNICODE_STRING TruncatedFullImageName = { 0 };

			eastl::wstring wsName = GetProcessFullName(TargetProcess);

			UNICODE_STRING FullImageName;

			ImpCall(RtlInitUnicodeString, &FullImageName, wsName.c_str());

			//LOG_DEBUG("PsQueryFullProcessImageName--->FullImageName:%wZ\r\n", FullImageName);

			if (FullImageName.Buffer != NULL || FullImageName.Length != 0)
			{
				for (size_t i = FullImageName.Length / 2; i > 0; i--)
				{
					if (FullImageName.Buffer[i] == L'\\')
					{
						ImpCall(RtlInitUnicodeString, &TruncatedFullImageName, &FullImageName.Buffer[i + 1]);
						break;
					}
				}
			}

			//LOG_DEBUG("PsQueryFullProcessImageName--->TruncatedFullImageName:%wZ\r\n", TruncatedFullImageName);
			return TruncatedFullImageName;
		}

		PEPROCESS GetEProcessByProcessName(wchar_t* ImageName)
		{
			PEPROCESS Process = NULL;
			for (size_t ProcessId = 4; ProcessId < 40000; ProcessId += 4)
			{
				Process = GetEProcessByProcessId((HANDLE)ProcessId);
				if (Process)
				{
					if (GetProcessFullName(Process).find(ImageName) != std::wstring::npos)
					{
						return Process;
					}
				}
			}
			return NULL;
		}

		BOOL IsWow64Process(PEPROCESS Process)
		{
			UNICODE_STRING uRoutineName;
			ImpCall(RtlInitUnicodeString, &uRoutineName, L"PsGetProcessWow64Process");
			auto PsGetProcessPebWow64 = (fnPsGetProcessWow64Process)ImpCall(MmGetSystemRoutineAddress, &uRoutineName);
			return PsGetProcessPebWow64(Process) != 0;
		}

		PVOID GetProcessPebWow64(PEPROCESS Process)
		{
			UNICODE_STRING uRoutineName;
			ImpCall(RtlInitUnicodeString, &uRoutineName, L"PsGetProcessWow64Process");
			auto PsGetProcessPebWow64 = (fnPsGetProcessWow64Process)ImpCall(MmGetSystemRoutineAddress, &uRoutineName);
			return (PVOID)PsGetProcessPebWow64(Process);
		}

		PVOID GetProcessPeb64(PEPROCESS Process)
		{
			UNICODE_STRING uRoutineName;
			ImpCall(RtlInitUnicodeString, &uRoutineName, L"PsGetProcessPeb");
			auto PsGetProcessPeb64 = (fnPsGetProcessPeb)ImpCall(MmGetSystemRoutineAddress, &uRoutineName);
			return (PVOID)PsGetProcessPeb64(Process);
		}

		PVOID GetProcessDebugPort(_In_ PEPROCESS Process)
		{
			return ImpCall(PsGetProcessDebugPort, Process);
		}

	}
}