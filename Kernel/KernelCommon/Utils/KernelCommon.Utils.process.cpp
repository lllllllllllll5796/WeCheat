#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.process.h"

namespace KernelCommon
{
	namespace Utils
	{
		ULONG GetProcessIdByProcessHandle(HANDLE ProcessHandle)
		{
			ULONG Pid = 0;
			PEPROCESS Process = NULL;
			if (!NT_SUCCESS(ObReferenceObjectByHandle(ProcessHandle, 0, *PsProcessType, ExGetPreviousMode(), (PVOID*)&Process, nullptr)))
			{
				if (Process == NULL)
				{
					if (!NT_SUCCESS(ObReferenceObjectByHandle(ProcessHandle, 0, *PsProcessType, KernelMode, (PVOID*)&Process, NULL)))
					{
						return NULL;
					}
				}
			}

			if (Process)
			{
				Pid = (ULONG)(ULONG_PTR)(PsGetProcessId(Process));
				ObfDereferenceObject(Process);
			}

			return Pid;
		}

		PEPROCESS GetProcessByProcessHandle(HANDLE ProcessHandle)
		{
			PEPROCESS Process = NULL;

			if (!NT_SUCCESS(ObReferenceObjectByHandle(ProcessHandle, 0, *PsProcessType, ExGetPreviousMode(), (PVOID*)&Process, nullptr)))
			{
				if (Process == NULL)
				{
					if (!NT_SUCCESS(ObReferenceObjectByHandle(ProcessHandle, 0, *PsProcessType, KernelMode, (PVOID*)&Process, NULL)))
					{
						return NULL;
					}
				}
			}

			if (Process)
			{
				ObfDereferenceObject(Process);
			}

			return Process;
		}

		HANDLE GetProcessHandleByEProcess(PEPROCESS Process)
		{
			HANDLE Handle = NULL;

			ObOpenObjectByPointer(Process,
				0,
				0,
				0,
				NULL,
				KernelMode,
				&Handle);

			return Handle;
		}

		ULONG GetProcessIdFromThreadHandle(HANDLE ThreadHandle)
		{
			ULONG Pid = 0;
			PETHREAD Thread = NULL;
			if (NT_SUCCESS(ObReferenceObjectByHandle(ThreadHandle, 0, *PsThreadType, ExGetPreviousMode(), (PVOID*)&Thread, nullptr)))
			{
				PEPROCESS Process = PsGetThreadProcess(Thread);

				Pid = (ULONG)(ULONG_PTR)PsGetProcessId(Process);
				ObfDereferenceObject(Thread);
			}
			return Pid;
		}

		HANDLE GetProcessId(PWCHAR ProcessName)
		{
			NTSTATUS Status = STATUS_SUCCESS;
			UNICODE_STRING		TagName = { 0 };
			RtlInitUnicodeString(&TagName, ProcessName);

			ULONG buffer_size = 0;
			PVOID pBuffer = NULL;
			Status = ZwQuerySystemInformation(wdk::SystemProcessInformation, pBuffer, 0, &buffer_size);
			while (Status == STATUS_INFO_LENGTH_MISMATCH)
			{
				if (pBuffer)
				{
					ExFreePool(pBuffer);
				}

				pBuffer = ExAllocatePool(NonPagedPool, buffer_size);
				Status = ZwQuerySystemInformation(wdk::SystemProcessInformation, pBuffer, buffer_size, &buffer_size);
			}

			wdk::PSYSTEM_PROCESS_INFORMATION ProcessInformation = (wdk::PSYSTEM_PROCESS_INFORMATION)(pBuffer);

			HANDLE ProcessList = NULL;
			for (;;)
			{
				if (FsRtlIsNameInExpression(&TagName, &(ProcessInformation->ImageName), FALSE, NULL) == TRUE)
				{
					ProcessList = ProcessInformation->UniqueProcessId;
					break;
				}

				if (ProcessInformation->NextEntryOffset == 0)
				{
					break;
				}

				ProcessInformation = (wdk::PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)ProcessInformation) + ProcessInformation->NextEntryOffset);
			}

			ExFreePool(pBuffer);

			return ProcessList;
		}

		BOOL IsHandleValid(HANDLE hObject, BOOL bThread)
		{
			PEPROCESS Process = NULL;
			PETHREAD Thread = NULL;
			NTSTATUS ns = STATUS_UNSUCCESSFUL;
			if (bThread)
			{
				ns = ObReferenceObjectByHandle(
					hObject,
					THREAD_GET_CONTEXT,
					*PsThreadType,
					KernelMode,
					(PVOID*)&Thread,
					NULL);
			}
			else
			{
				ns = ObReferenceObjectByHandle(
					hObject,
					PROCESS_SET_PORT,
					*PsProcessType,
					KernelMode,
					(PVOID*)&Process,
					NULL);
			}

			if (NT_SUCCESS(ns))
			{
				if (bThread)
				{
					ObfDereferenceObject(Thread);
					Process = IoThreadToProcess(Thread);
				}
				else
				{
					ObfDereferenceObject(Process);
				}
				return TRUE;
			}

			return FALSE;
		}

		HANDLE GetProcessHandleByProcessId(IN HANDLE ProcessId)
		{
			HANDLE ProcessHandle = NULL;
			PEPROCESS Process = NULL;

			Process = GetEProcessByProcessId(ProcessId);

			if (Process)
			{
				ObOpenObjectByPointer(
					Process,
					0,
					NULL,
					PROCESS_ALL_ACCESS,
					*PsProcessType,
					KernelMode,     //UserMode,
					&ProcessHandle);

			}
			return ProcessHandle;
		}

		BOOL GetProcessInfo(CONST CHAR* Name, ULONG64& ImageSize, PVOID& ImageBase)
		{
			//LOG_DEBUG("GetProcessInfo Begin\r\n");
			ULONG Bytes;
			NTSTATUS Status = ZwQuerySystemInformation(wdk::SYSTEM_INFORMATION_CLASS::SystemModuleInformation, 0, 0, &Bytes);
			PSYSTEM_MODULE_INFORMATION Mods = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(NonPagedPool, Bytes);
			if (Mods == NULL)
			{
				return FALSE;
			}

			RtlSecureZeroMemory(Mods, Bytes);

			Status = ZwQuerySystemInformation(wdk::SYSTEM_INFORMATION_CLASS::SystemModuleInformation, Mods, Bytes, &Bytes);
			if (NT_SUCCESS(Status) == FALSE)
			{
				ExFreePool(Mods);
				return FALSE;
			}

			STRING TargetProcessName;
			RtlInitString(&TargetProcessName, Name);

			for (ULONG i = 0; i < Mods->Count; i++)
			{
				STRING CurrentModuleName;
				RtlInitString(&CurrentModuleName, (PCSZ)Mods->Module[i].FullPathName);

				if (RtlStringContains(&CurrentModuleName, &TargetProcessName, TRUE) != NULL)
				{
					if (Mods->Module[i].ImageSize != NULL)
					{
						ImageSize = Mods->Module[i].ImageSize;
						ImageBase = Mods->Module[i].ImageBase;
						ExFreePool(Mods);
						LOG_DEBUG("GetProcessInfo End Success\r\n");
						return TRUE;
					}
				}
			}

			ExFreePool(Mods);

			//LOG_DEBUG("GetProcessInfo End Error\r\n");
			return FALSE;
		}
	}
}