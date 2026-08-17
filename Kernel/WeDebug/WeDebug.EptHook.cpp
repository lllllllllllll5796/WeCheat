#include "WeDebug.pch.h"
#include "WeDebug.EptHook.h"
#include "WeDebug.DbgkApi.h"
#include "WeDebug.Process.h"

//----------------------------------------------------------------------
// WeVt EPT RIP Hook
//----------------------------------------------------------------------
#define WEDEBUG_SETUP_HOOK(FUNC_NAME, FAKE_FUNC)                                                        \
    do {                                                                                                \
        if (Global::FUNC_NAME == nullptr) {                                                             \
            LOG_DEBUG("[-] " #FUNC_NAME " 指针为空，跳过 Hook\r\n");                                   \
            break;                                                                                      \
        }                                                                                               \
        if (hvgt::hook_function((PVOID)(Global::FUNC_NAME), (PVOID)(FAKE_FUNC), nullptr)) {              \
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
        if (hvgt::hook_function((PVOID)(Global::FUNC_NAME), (PVOID)(FAKE_FUNC), (PVOID*)(TRAMPOLINE))) { \
            LOG_DEBUG("[+] Hook " #FUNC_NAME " 成功（带 Trampoline）\r\n");                            \
        } else {                                                                                        \
            LOG_DEBUG("[-] Hook " #FUNC_NAME " 失败（带 Trampoline）\r\n");                            \
        }                                                                                               \
    } while (0)


EXTERN_C
VOID UnEptHook()
{
    LOG_DEBUG("[+] 开始卸载所有 WeDebug EPT 钩子\r\n");

    if (hvgt::ept_unhook())
    {
        LOG_DEBUG("[+] 卸载所有 EPT 钩子成功\r\n");
    }
    else
    {
        LOG_DEBUG("[-] 卸载 EPT 钩子失败（WeVt 可能未运行）\r\n");
    }
}


EXTERN_C
VOID SetupEptHook()
{
    if (!Global::g_IsInitSymbols)
    {
        LOG_DEBUG("[-] SetupEptHook 失败：符号尚未初始化\r\n");
        return;
    }

    if (!hvgt::test_vmcall())
    {
        LOG_DEBUG("[-] SetupEptHook 失败：无法与 WeVt 通信\r\n");
        return;
    }

    LOG_DEBUG("[+] 开始安装 WeDebug EPT 钩子\r\n");

    WEDEBUG_SETUP_HOOK(NtCreateDebugObject, Fake_NtCreateDebugObject);
    WEDEBUG_SETUP_HOOK(PspInsertProcess, Fake_PspInsertProcess);
    WEDEBUG_SETUP_HOOK(NtSetInformationDebugObject, Fake_NtSetInformationDebugObject);
    WEDEBUG_SETUP_HOOK(NtRemoveProcessDebug, Fake_NtRemoveProcessDebug);
    WEDEBUG_SETUP_HOOK(NtDebugActiveProcess, Fake_NtDebugActiveProcess);
    WEDEBUG_SETUP_HOOK(NtWaitForDebugEvent, Fake_NtWaitForDebugEvent);
    WEDEBUG_SETUP_HOOK(NtDebugContinue, Fake_NtDebugContinue);
    WEDEBUG_SETUP_HOOK(DbgkMapViewOfSection, Fake_DbgkMapViewOfSection);
    WEDEBUG_SETUP_HOOK(DbgkUnMapViewOfSection, Fake_DbgkUnMapViewOfSection);
    WEDEBUG_SETUP_HOOK(DbgkCreateThread, Fake_DbgkCreateThread);
    WEDEBUG_SETUP_HOOK(DbgkExitThread, Fake_DbgkExitThread);
    WEDEBUG_SETUP_HOOK(DbgkExitProcess, Fake_DbgkExitProcess);
    WEDEBUG_SETUP_HOOK(DbgkForwardException, Fake_DbgkForwardException);
    WEDEBUG_SETUP_HOOK(DbgkpQueueMessage, Fake_DbgkpQueueMessage);
    WEDEBUG_SETUP_HOOK_TRAMPOLINE(PspExitThread, Fake_PspExitThread, &Global::Original_PspExitThread);
    WEDEBUG_SETUP_HOOK(DbgkOpenProcessDebugPort, Fake_DbgkOpenProcessDebugPort);

    LOG_DEBUG("[+] 所有 WeDebug EPT 钩子安装完毕\r\n");
}
