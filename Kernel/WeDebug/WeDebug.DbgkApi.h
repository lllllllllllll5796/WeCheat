#pragma once

NTSTATUS DbgkInitialize();

VOID DbgkUnInitialize();


POBJECT_TYPE GetDebugObjectType(UNICODE_STRING Name);

EXTERN_C
VOID
DbgkpCloseObject(
	IN _EPROCESS* Process,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG_PTR SystemHandleCount
);

BOOLEAN IsDebugTargetProcess(IN _EPROCESS* Process, OUT PDEBUG_PROCESS* DebugProcess);

VOID DeleteDebugProcess(PDEBUG_OBJECT DebugObject);