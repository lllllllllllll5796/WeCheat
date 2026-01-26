#pragma once

extern "C"
{
	NTSYSAPI
		PVOID
		NTAPI
		RtlPcToFileHeader(
			_In_ PVOID PcValue,
			_Out_ PVOID* BaseOfImage
		);

	NTSYSAPI
		PVOID
		NTAPI
		KeQueryPrcbAddress(
			__in ULONG Number
		);

	NTSYSAPI 
		BOOLEAN
		NTAPI 
		PsIsProtectedProcess(IN PEPROCESS Process);

	DECLSPEC_NORETURN
	NTSYSAPI
	VOID
	__cdecl
	RtlRestoreContext(
		_In_ PCONTEXT ContextRecord,
		_In_opt_ struct _EXCEPTION_RECORD* ExceptionRecord
	);

	NTKERNELAPI
	NTSTATUS
	NTAPI
	KeRaiseUserException(
		IN NTSTATUS ExceptionCode
	);

	NTHALAPI
	ULONG
	HalGetInterruptVector(
		_In_ INTERFACE_TYPE  InterfaceType,
		_In_ ULONG BusNumber,
		_In_ ULONG BusInterruptLevel,
		_In_ ULONG BusInterruptVector,
		_Out_ PKIRQL Irql,
		_Out_ PKAFFINITY Affinity
	);

	NTSYSAPI
	NTSTATUS
	NTAPI
	LdrFindResource_U(
		IN  PVOID                       DllHandle,
		IN  PULONG_PTR                  ResourceIdPath,
		IN  ULONG                       ResourceIdPathLength,
		OUT PIMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry
	);

	NTSYSAPI
	NTSTATUS
	NTAPI
	LdrAccessResource(
		IN  PVOID                       DllHandle,
		IN  PIMAGE_RESOURCE_DATA_ENTRY  ResourceDataEntry,
		OUT PVOID* Address OPTIONAL,
		OUT PULONG                      Size OPTIONAL
	);

	NTKERNELAPI
	PEPROCESS
	IoThreadToProcess(
		_In_ PETHREAD Thread
	);

	NTKERNELAPI
	BOOLEAN
	IoIsSystemThread(
		_In_ PETHREAD Thread
	);


	NTKERNELAPI
		VOID
		FASTCALL
		ExfUnblockPushLock(
			IN OUT PEX_PUSH_LOCK PushLock,
			IN OUT PVOID WaitBlock
		);

	NTKERNELAPI
	NTSTATUS
	NTAPI
	PsLookupProcessThreadByCid(
		PCLIENT_ID ClientId,
		PEPROCESS* Process,
		PETHREAD* Thread
	);

	
	NTSYSCALLAPI
	NTSTATUS
	NTAPI
	NtQuerySystemInformationEx(
		wdk::SYSTEM_INFORMATION_CLASS SystemInformationClass,
		PVOID InputBuffer,
		ULONG InputBufferLength,
		PVOID SystemInformation,
		ULONG SystemInformationLength,
		PULONG ReturnLength
	);

	
	NTSYSCALLAPI
	NTSTATUS
	NTAPI
	NtQueryInformationProcess(
		HANDLE ProcessHandle,
		PROCESSINFOCLASS ProcessInformationClass,
		PVOID ProcessInformation,
		ULONG ProcessInformationLength,
		PULONG ReturnLength
	);
}
