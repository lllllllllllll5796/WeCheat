#pragma once
#include "Wdk.TsStruct.inl"

namespace wdk
{
    extern"C"
    {
        PVOID NTAPI
            PsGetCurrentThreadStackLimit(VOID);

        PVOID NTAPI
            PsGetCurrentThreadStackBase(VOID);

        KPROCESSOR_MODE NTAPI
            PsGetCurrentThreadPreviousMode(VOID);

        BOOLEAN NTAPI
            PsIsThreadTerminating(
                _In_ PETHREAD Thread
            );

        NTSTATUS NTAPI
            PsGetContextThread(
                _In_ PETHREAD Thread,
                _Inout_ PCONTEXT ThreadContext,
                _In_ KPROCESSOR_MODE Mode
            );

        NTSTATUS NTAPI
            PsSetContextThread(
                _In_ PETHREAD Thread,
                _In_ PCONTEXT ThreadContext,
                _In_ KPROCESSOR_MODE Mode
            );

        VOID NTAPI
            PsSetThreadHardErrorsAreDisabled(
                _In_ PETHREAD Thread,
                _In_ BOOLEAN HardErrorsAreDisabled
            );

        VOID NTAPI
            PsSetThreadWin32Thread(
                _Inout_ PETHREAD Thread,
                _In_ PVOID Win32Thread,
                _In_ PVOID PrevWin32Thread
            );

        CCHAR NTAPI
            PsGetThreadFreezeCount(
                _In_ PETHREAD Thread
            );

        BOOLEAN NTAPI
            PsGetThreadHardErrorsAreDisabled(
                _In_ PETHREAD Thread
            );

        PEPROCESS NTAPI
            PsGetThreadProcess(
                _In_ PETHREAD Thread
            );

        PEPROCESS NTAPI
            PsGetCurrentThreadProcess(
                VOID
            );

        HANDLE NTAPI
            PsGetCurrentThreadProcessId(
                VOID
            );

        inline CLIENT_ID NTAPI
            PsGetThreadClientId(PETHREAD Thread)
        {
            return { PsGetThreadProcessId(Thread), PsGetThreadId(Thread) };
        }

        ULONG NTAPI
            PsGetThreadSessionId(
                _In_ PETHREAD Thread
            );

#define  PsGetThreadTcb(Thread) ((PKTHREAD)(Thread))

        PVOID NTAPI
            PsGetThreadTeb(
                _In_ PETHREAD Thread
            );

        PVOID NTAPI
            PsGetThreadWin32Thread(
                _In_ PETHREAD Thread
            );

        PVOID NTAPI
            PsGetCurrentThreadWin32Thread(
                VOID
            );

        PVOID NTAPI
            PsGetCurrentThreadWin32ThreadAndEnterCriticalRegion(
                __out PHANDLE ProcessId
            );

        BOOLEAN NTAPI
            PsIsSystemThread(
                _In_ PETHREAD Thread
            );

        BOOLEAN NTAPI
            PsIsThreadImpersonating(
                _In_ PETHREAD Thread
            );
    }
}

namespace wdk
{
    extern"C"
    {
        inline auto NtCurrentTeb(VOID)
            -> struct _TEB*
        {
#ifdef _WIN64
            return (struct _TEB *)__readgsqword(offsetof(NT_TIB, Self));
#else
            return (struct _TEB *)__readfsdword(offsetof(NT_TIB, Self));
#endif
        }

       

        inline auto TsInitSystem() -> NTSTATUS
        {
            return STATUS_SUCCESS;
        }
    }
}
