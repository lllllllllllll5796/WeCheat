#pragma once

void Hook_NtDebugActiveProcess();
void UnHook_NtDebugActiveProcess();

void Hook_DbgUiIssueRemoteBreakin();
void UnHook_DbgUiIssueRemoteBreakin();

void Hook_WaitForDebugEvent();
void UnHook_WaitForDebugEvent();

void Hook_ContinueDebugEvent();
void UnHook_ContinueDebugEvent();

void Hook_OutputDebugStringA();
void UnHook_OutputDebugStringA();

void Hook_OutputDebugStringW();
void UnHook_OutputDebugStringW();

void Hook_SetThreadContext();
void UnHook_SetThreadContext();

void Hook_GetThreadContext();
void UnHook_GetThreadContext();

void Hook_WriteProcessMemory();
void UnHook_WriteProcessMemory();

void Hook_ReadProcessMemory();
void UnHook_ReadProcessMemory();

void Hook_VirtualProtectEx();
void UnHook_VirtualProtectEx();