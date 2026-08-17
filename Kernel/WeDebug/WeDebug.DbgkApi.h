#pragma once

NTSTATUS DbgkInitialize();

VOID DbgkUnInitialize();

POBJECT_TYPE GetDebugObjectType(UNICODE_STRING Name);

EXTERN_C
VOID
DbgkpCloseObject(
	IN PEPROCESS Process,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG_PTR SystemHandleCount
);

BOOLEAN IsDebugTargetProcess(IN PEPROCESS Process, OUT PDEBUG_PROCESS* DebugProcess);

VOID DeleteDebugProcess(PDEBUG_OBJECT DebugObject);

VOID InsertDebuggerList(PDEBUGGER_TABLE_ENTRY Debugger);

BOOLEAN IsDebugger(PEPROCESS Process);

VOID PrintProcessName(PEPROCESS Process);

EXTERN_C NTSTATUS NTAPI Fake_NtCreateDebugObject(OUT PHANDLE DebugHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG Flags);
EXTERN_C NTSTATUS NTAPI Fake_NtDebugActiveProcess(IN HANDLE ProcessHandle, IN HANDLE DebugHandle);
EXTERN_C NTSTATUS __fastcall Fake_NtWaitForDebugEvent(IN HANDLE DebugHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL, OUT PDBGUI_WAIT_STATE_CHANGE StateChange);
EXTERN_C NTSTATUS __fastcall Fake_NtDebugContinue(IN HANDLE DebugObjectHandle, IN PCLIENT_ID ClientId, IN NTSTATUS ContinueStatus);
EXTERN_C NTSTATUS __fastcall Fake_NtSetInformationDebugObject(IN HANDLE DebugObjectHandle, IN DEBUGOBJECTINFOCLASS DebugObjectInformationClass, IN PVOID DebugInformation, IN ULONG DebugInformationLength, OUT PULONG ReturnLength OPTIONAL);
EXTERN_C NTSTATUS __fastcall Fake_NtRemoveProcessDebug(IN HANDLE ProcessHandle, IN HANDLE DebugObjectHandle);
EXTERN_C BOOLEAN NTAPI Fake_DbgkForwardException(IN PEXCEPTION_RECORD ExceptionRecord, IN BOOLEAN DebugPort, IN BOOLEAN SecondChance);
EXTERN_C NTSTATUS __fastcall Fake_DbgkpQueueMessage(IN PEPROCESS Process, IN PETHREAD Thread, IN PDBGKM_APIMSG Message, IN ULONG Flags, IN PDEBUG_OBJECT TargetObject OPTIONAL);
EXTERN_C VOID __fastcall Fake_DbgkMapViewOfSection(IN PEPROCESS Process, IN PVOID SectionObject, IN PVOID BaseAddress, IN ULONG SectionOffset, IN ULONG_PTR ViewSize);
EXTERN_C VOID __fastcall Fake_DbgkUnMapViewOfSection(IN PEPROCESS Process, IN PVOID BaseAddress);
EXTERN_C VOID __fastcall Fake_DbgkCreateThread(IN PETHREAD Thread);
EXTERN_C VOID __fastcall Fake_DbgkExitThread(NTSTATUS ExitStatus);
EXTERN_C VOID __fastcall Fake_DbgkExitProcess(NTSTATUS ExitStatus);
EXTERN_C NTSTATUS __fastcall Fake_DbgkOpenProcessDebugPort(IN PEPROCESS Process, IN KPROCESSOR_MODE PreviousMode, OUT HANDLE* DebugHandle);
EXTERN_C VOID __fastcall Fake_PspExitThread(_In_ NTSTATUS ExitStatus);