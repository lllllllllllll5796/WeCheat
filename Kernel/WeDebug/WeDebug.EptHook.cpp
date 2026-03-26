#include "WeDebug.pch.h"
#include "WeDebug.EptHook.h"
#include "WeDebug.DbgkApi.h"
#include "VmxHelper/KernelCommon.VmxHelper.EptHook.h"

//----------------------------------------------------------------------
// 所有 Hook 替换函数的前向声明（统一使用 Fake_ 前缀）
// 实现分布在 WeDebug.DbgkApi.cpp 等文件中
//----------------------------------------------------------------------

// ntos — Nt 系统调用层
EXTERN_C NTSTATUS NTAPI         Fake_NtCreateDebugObject(_Out_ PHANDLE DebugObjectHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes, _In_ ULONG Flags);
EXTERN_C NTSTATUS NTAPI         Fake_NtDebugActiveProcess(HANDLE ProcessHandle, HANDLE DebugObjectHandle);
EXTERN_C NTSTATUS __fastcall    Fake_NtWaitForDebugEvent(HANDLE DebugHandle, BOOLEAN Alertable, PLARGE_INTEGER Timeout, PDBGUI_WAIT_STATE_CHANGE StateChange);
EXTERN_C NTSTATUS __fastcall    Fake_NtDebugContinue(HANDLE DebugObjectHandle, PCLIENT_ID ClientId, NTSTATUS ContinueStatus);
EXTERN_C NTSTATUS __fastcall    Fake_NtSetInformationDebugObject(HANDLE DebugObjectHandle, DEBUGOBJECTINFOCLASS DebugObjectInformationClass, PVOID DebugInformation, ULONG DebugInformationLength, PULONG ReturnLength);
EXTERN_C NTSTATUS __fastcall    Fake_NtRemoveProcessDebug(HANDLE ProcessHandle, HANDLE DebugObjectHandle);

// ntos — Dbgk 调试子系统内部函数
EXTERN_C BOOLEAN  NTAPI         Fake_DbgkForwardException(PEXCEPTION_RECORD ExceptionRecord, BOOLEAN DebugException, BOOLEAN SecondChance);
EXTERN_C NTSTATUS __fastcall    Fake_DbgkpQueueMessage(PEPROCESS Process, PETHREAD Thread, PDBGKM_APIMSG ApiMsg, ULONG Flags, PDEBUG_OBJECT DebugObject);
EXTERN_C VOID     __fastcall    Fake_DbgkMapViewOfSection(PEPROCESS Process, PVOID SectionObject, PVOID BaseAddress, ULONG SectionOffset, ULONG_PTR ViewSize);
EXTERN_C VOID     __fastcall    Fake_DbgkUnMapViewOfSection(PEPROCESS Process, PVOID BaseAddress);
EXTERN_C VOID     __fastcall    Fake_DbgkCreateThread(PETHREAD Thread);
EXTERN_C VOID     __fastcall    Fake_DbgkExitThread(NTSTATUS ExitStatus);
EXTERN_C VOID     __fastcall    Fake_DbgkExitProcess(NTSTATUS ExitStatus);
EXTERN_C NTSTATUS __fastcall    Fake_DbgkOpenProcessDebugPort(PEPROCESS Process, KPROCESSOR_MODE PreviousMode, HANDLE* pHandle);

// ntos — Ps 进程/线程管理
EXTERN_C NTSTATUS __fastcall    Fake_PspInsertProcess(PEPROCESS CurrentProcess, PEPROCESS ParentProcess, ACCESS_MASK DesiredAccess, ULONG Flags, ULONG JobMemberLevel, HANDLE DebugObjectHandle, ULONG CrossThreadFlags, PVOID ObjectCreationState);
EXTERN_C VOID     __fastcall    Fake_PspExitThread(NTSTATUS ExitStatus);
EXTERN_C VOID     __fastcall    Fake_PspCallThreadNotifyRoutines(PETHREAD Thread, BOOLEAN Create, BOOLEAN a3);

// ntos — Ob 对象管理器
EXTERN_C NTSTATUS __fastcall    Fake_ObReferenceObjectByHandleWithTag(HANDLE Handle, int a2, _OBJECT_TYPE* a3, char a4, int a5, PVOID* Object, __int64 a7);
EXTERN_C NTSTATUS __fastcall    Fake_ObpReferenceObjectByHandleWithTag(HANDLE Handle, ACCESS_MASK DesiredAccess, POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode, ULONG Tag, PVOID* Object, POBJECT_HANDLE_INFORMATION HandleInformation, __int64 a8);

// win32k.sys — 窗口/句柄保护
EXTERN_C PWND     __fastcall    Fake_ValidateHwnd(HWND hwnd);
EXTERN_C ULONG    NTAPI         Fake_NtUserFindWindowEx(HWND hwndParent, HWND hwndChild, PUNICODE_STRING pstrClassName, PUNICODE_STRING pstrWindowName, DWORD dwType);
EXTERN_C HWND     __fastcall    Fake_NtUserWindowFromPoint(POINT Point);


//----------------------------------------------------------------------
// DBVM EPT Hook 辅助宏
// HookFunction 的 trampoline_address 不能为空（内部会解引用），
// 不需要保存 trampoline 时用局部变量承接后丢弃
//----------------------------------------------------------------------
#define WEDEBUG_SETUP_HOOK(FUNC_NAME, FAKE_FUNC)                                                        \
    do {                                                                                                \
        if (Global::FUNC_NAME == nullptr) {                                                             \
            LOG_DEBUG("[-] " #FUNC_NAME " 指针为空，跳过 Hook\r\n");                                   \
            break;                                                                                      \
        }                                                                                               \
        PVOID _trampoline_##FUNC_NAME = nullptr;                                                        \
        if (VmxHelper::HookFunction(#FUNC_NAME,                                                        \
                (PVOID)(Global::FUNC_NAME),                                                             \
                (PVOID)(FAKE_FUNC),                                                                     \
                &_trampoline_##FUNC_NAME,                                                               \
                VmxHelper::jmp_type)) {                                                                 \
            LOG_DEBUG("[+] Hook " #FUNC_NAME " 成功\r\n");                                             \
        } else {                                                                                        \
            LOG_DEBUG("[-] Hook " #FUNC_NAME " 失败\r\n");                                             \
        }                                                                                               \
    } while (0)

#define WEDEBUG_SETUP_HOOK_TRAMPOLINE(FUNC_NAME, FAKE_FUNC, TRAMPOLINE)                                 \
    do {                                                                                                \
        if (Global::FUNC_NAME == nullptr) {                                                             \
            LOG_DEBUG("[-] " #FUNC_NAME " 指针为空，跳过 Hook\r\n");                                   \
            break;                                                                                      \
        }                                                                                               \
        if (VmxHelper::HookFunction(#FUNC_NAME,                                                        \
                (PVOID)(Global::FUNC_NAME),                                                             \
                (PVOID)(FAKE_FUNC),                                                                     \
                (PVOID*)(TRAMPOLINE),                                                                   \
                VmxHelper::jmp_type)) {                                                                 \
            LOG_DEBUG("[+] Hook " #FUNC_NAME " 成功（带 Trampoline）\r\n");                            \
        } else {                                                                                        \
            LOG_DEBUG("[-] Hook " #FUNC_NAME " 失败（带 Trampoline）\r\n");                            \
        }                                                                                               \
    } while (0)

#define WEDEBUG_UNHOOK(FUNC_NAME)                                                                       \
    do {                                                                                                \
        VmxHelper::UnHookFunction(#FUNC_NAME);                                                         \
        LOG_DEBUG("[+] UnHook " #FUNC_NAME "\r\n");                                                    \
    } while (0)


//----------------------------------------------------------------------
// 卸载所有 EPT 钩子
//----------------------------------------------------------------------
EXTERN_C
VOID UnEptHook()
{
    LOG_DEBUG("[+] 开始卸载所有 WeDebug EPT 钩子\r\n");

    // ntos — Nt 系统调用
    WEDEBUG_UNHOOK(NtCreateDebugObject);
    WEDEBUG_UNHOOK(PspInsertProcess);
    WEDEBUG_UNHOOK(NtSetInformationDebugObject);
    WEDEBUG_UNHOOK(NtRemoveProcessDebug);
    WEDEBUG_UNHOOK(NtDebugActiveProcess);
    WEDEBUG_UNHOOK(NtWaitForDebugEvent);
    WEDEBUG_UNHOOK(NtDebugContinue);

    // ntos — Dbgk 调试子系统
    WEDEBUG_UNHOOK(DbgkMapViewOfSection);
    WEDEBUG_UNHOOK(DbgkUnMapViewOfSection);
    WEDEBUG_UNHOOK(DbgkCreateThread);
    WEDEBUG_UNHOOK(DbgkExitThread);
    WEDEBUG_UNHOOK(DbgkExitProcess);
    WEDEBUG_UNHOOK(DbgkForwardException);
    WEDEBUG_UNHOOK(DbgkpQueueMessage);
    WEDEBUG_UNHOOK(DbgkOpenProcessDebugPort);

    // ntos — Ps
    WEDEBUG_UNHOOK(PspCallThreadNotifyRoutines);
    WEDEBUG_UNHOOK(PspExitThread);

    // ntos — Ob
    WEDEBUG_UNHOOK(ObReferenceObjectByHandleWithTag);
    WEDEBUG_UNHOOK(ObpReferenceObjectByHandleWithTag);

    // win32k.sys
    WEDEBUG_UNHOOK(ValidateHwnd);
    WEDEBUG_UNHOOK(NtUserFindWindowEx);
    WEDEBUG_UNHOOK(NtUserWindowFromPoint);

    LOG_DEBUG("[+] 所有 WeDebug EPT 钩子已卸载\r\n");
}


//----------------------------------------------------------------------
// 安装所有 EPT 钩子
// 注意：NtCreateDebugObject 必须最先挂钩，win32k 的 ValidateHwnd 也要优先
//----------------------------------------------------------------------
EXTERN_C
VOID SetupEptHook()
{
    if (!Global::g_IsInitSymbols)
    {
        LOG_DEBUG("[-] SetupEptHook 失败：符号尚未初始化\r\n");
        return;
    }

    LOG_DEBUG("[+] 开始安装 WeDebug EPT 钩子\r\n");

    // ① 调试对象创建 — 必须第一个挂钩
    WEDEBUG_SETUP_HOOK(NtCreateDebugObject, Fake_NtCreateDebugObject);

    //// ② Ps 进程插入
    //WEDEBUG_SETUP_HOOK(PspInsertProcess, Fake_PspInsertProcess);

    //// ③ 调试对象信息控制
    //WEDEBUG_SETUP_HOOK(NtSetInformationDebugObject, Fake_NtSetInformationDebugObject);

    //// ④ 移除进程调试
    //WEDEBUG_SETUP_HOOK(NtRemoveProcessDebug, Fake_NtRemoveProcessDebug);

    //// ⑤ 核心调试 API
    //WEDEBUG_SETUP_HOOK(NtDebugActiveProcess, Fake_NtDebugActiveProcess);
    //WEDEBUG_SETUP_HOOK(NtWaitForDebugEvent, Fake_NtWaitForDebugEvent);
    //WEDEBUG_SETUP_HOOK(NtDebugContinue, Fake_NtDebugContinue);

    //// ⑥ 模块映射/取消映射通知
    //WEDEBUG_SETUP_HOOK(DbgkMapViewOfSection, Fake_DbgkMapViewOfSection);
    //WEDEBUG_SETUP_HOOK(DbgkUnMapViewOfSection, Fake_DbgkUnMapViewOfSection);

    //// ⑦ 线程生命周期通知
    //WEDEBUG_SETUP_HOOK(DbgkCreateThread, Fake_DbgkCreateThread);
    //WEDEBUG_SETUP_HOOK(DbgkExitThread, Fake_DbgkExitThread);
    //WEDEBUG_SETUP_HOOK(DbgkExitProcess, Fake_DbgkExitProcess);

    //// ⑧ 异常转发 & 消息队列
    //WEDEBUG_SETUP_HOOK(DbgkForwardException, Fake_DbgkForwardException);
    //WEDEBUG_SETUP_HOOK(DbgkpQueueMessage, Fake_DbgkpQueueMessage);

    //// ⑨ Ps 线程通知 & 退出
    //WEDEBUG_SETUP_HOOK(PspCallThreadNotifyRoutines, Fake_PspCallThreadNotifyRoutines);
    //WEDEBUG_SETUP_HOOK(PspExitThread, Fake_PspExitThread);

    //// ⑩ Ob 句柄引用（ObReferenceObjectByHandleWithTag 需保存 Trampoline）
    //WEDEBUG_SETUP_HOOK_TRAMPOLINE(ObReferenceObjectByHandleWithTag,
    //    Fake_ObReferenceObjectByHandleWithTag,
    //    &Global::Original_ObReferenceObjectByHandleWithTag);
    //WEDEBUG_SETUP_HOOK(ObpReferenceObjectByHandleWithTag, Fake_ObpReferenceObjectByHandleWithTag);

    //// ⑪ win32k.sys — ValidateHwnd 必须在 Find/FromPoint 之前挂钩
    //WEDEBUG_SETUP_HOOK(ValidateHwnd, Fake_ValidateHwnd);
    //WEDEBUG_SETUP_HOOK(NtUserFindWindowEx, Fake_NtUserFindWindowEx);
    //WEDEBUG_SETUP_HOOK(NtUserWindowFromPoint, Fake_NtUserWindowFromPoint);

    LOG_DEBUG("[+] 所有 WeDebug EPT 钩子安装完毕\r\n");
}
