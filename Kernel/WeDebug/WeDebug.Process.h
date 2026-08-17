#pragma once

EXTERN_C NTSTATUS __fastcall Fake_PspInsertProcess(
	PEPROCESS TargetProcess,
	PEPROCESS ParentProcess,
	ACCESS_MASK DesiredAccess,
	ULONG Flags,
	ULONG JobMemberLevel,
	HANDLE DebugObjectHandle,
	ULONG CrossThreadFlags,
	PVOID ObjectCreationState);
