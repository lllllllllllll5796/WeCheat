#pragma once

typedef enum _SYSDBG_COMMAND
{
	SysDbgGetTriageDump = 29,
} SYSDBG_COMMAND, *PSYSDBG_COMMAND;

typedef enum _WINDOWINFOCLASS {
	WindowProcess,
	WindowThread,
	WindowActiveWindow,
	WindowFocusWindow,
	WindowIsHung,
	WindowClientBase,
	WindowIsForegroundThread,
} WINDOWINFOCLASS;

typedef enum _THREAD_STATE_ROUTINE
{
	THREADSTATE_GETTHREADINFO,
	THREADSTATE_ACTIVEWINDOW
} THREAD_STATE_ROUTINE;

typedef enum
{
	StateInitialized,
	StateReady,
	StateRunning,
	StateStandby,
	StateTerminated,
	StateWait,
	StateTransition,
	StateUnknown

} THREAD_STATE;

typedef enum _SHUTDOWN_ACTION
{
	ShutdownNoReboot,
	ShutdownReboot,
	ShutdownPowerOff
} SHUTDOWN_ACTION, * PSHUTDOWN_ACTION;

typedef enum _PSW32THREADCALLOUTTYPE {
	PsW32ThreadCalloutInitialize,
	PsW32ThreadCalloutExit
} PSW32THREADCALLOUTTYPE;