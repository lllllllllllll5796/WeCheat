#pragma once

#define MAGIC_NTSTATUS     0x80070000
#define STATUS_CUSTOM_STATUS(x) (MAGIC_NTSTATUS + x) //不出错误框
//需要管理员处理才能工作 STATUS_DOWNGRADE_DETECTED
//恶意软件通报 STATUS_VIRUS_INFECTED

#define WOW64_SIZE_OF_80387_REGISTERS 80
#define WOW64_MAXIMUM_SUPPORTED_EXTENSION 512

#define ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID    (0x00000001)
#define ACTCTX_FLAG_LANGID_VALID                    (0x00000002)
#define ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID        (0x00000004)
#define ACTCTX_FLAG_RESOURCE_NAME_VALID             (0x00000008)
#define ACTCTX_FLAG_SET_PROCESS_DEFAULT             (0x00000010)
#define ACTCTX_FLAG_APPLICATION_NAME_VALID          (0x00000020)
#define ACTCTX_FLAG_SOURCE_IS_ASSEMBLYREF           (0x00000040)
#define ACTCTX_FLAG_HMODULE_VALID                   (0x00000080)

#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED        0x00000001
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH      0x00000002
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER      0x00000004
#define THREAD_CREATE_FLAGS_BYPASS_PROCESS_FREEZE   0x40

#define PROCESS_SET_PORT 0x0800

#define CODEINTEGRITY_OPTION_ENABLED                        0x01
#define CODEINTEGRITY_OPTION_TESTSIGN                       0x02
#define CODEINTEGRITY_OPTION_UMCI_ENABLED                   0x04
#define CODEINTEGRITY_OPTION_UMCI_AUDITMODE_ENABLED         0x08
#define CODEINTEGRITY_OPTION_UMCI_EXCLUSIONPATHS_ENABLED    0x10
#define CODEINTEGRITY_OPTION_TEST_BUILD                     0x20
#define CODEINTEGRITY_OPTION_PREPRODUCTION_BUILD            0x40
#define CODEINTEGRITY_OPTION_DEBUGMODE_ENABLED              0x80
#define CODEINTEGRITY_OPTION_FLIGHT_BUILD                   0x100
#define CODEINTEGRITY_OPTION_FLIGHTING_ENABLED              0x200
#define CODEINTEGRITY_OPTION_HVCI_KMCI_ENABLED              0x400
#define CODEINTEGRITY_OPTION_HVCI_KMCI_AUDITMODE_ENABLED    0x800
#define CODEINTEGRITY_OPTION_HVCI_KMCI_STRICTMODE_ENABLED   0x1000
#define CODEINTEGRITY_OPTION_HVCI_IUM_ENABLED               0x2000

#define STATUS_PRIVILEGE_NOT_HELD ((NTSTATUS)0xC0000061L)
//#define STATUS_PORT_NOT_SET ((NTSTATUS)0xC0000353L)
#define STATUS_HANDLE_NOT_CLOSABLE ((NTSTATUS)0xC0000235L)
#define PROCESS_DEBUG_INHERIT    0x00000001 // default for a non-debugged process
#define PROCESS_NO_DEBUG_INHERIT 0x00000002 // default for a debugged process

#define PROCESS_QUERY_INFORMATION   0x0400

#define INVALID_HANDLE_VALUE  ((HANDLE)((LONG_PTR)-1))


//
// Define EFLAG bit masks and shift offsets.
//

#define EFLAGS_CF_MASK 0x00000001       // carry flag
#define EFLAGS_PF_MASK 0x00000004       // parity flag
#define EFLAGS_AF_MASK 0x00000010       // auxiliary carry flag
#define EFLAGS_ZF_MASK 0x00000040       // zero flag
#define EFLAGS_SF_MASK 0x00000080       // sign flag
#define EFLAGS_TF_MASK 0x00000100       // trap flag
#define EFLAGS_IF_MASK 0x00000200       // interrupt flag
#define EFLAGS_DF_MASK 0x00000400       // direction flag
#define EFLAGS_OF_MASK 0x00000800       // overflow flag
#define EFLAGS_IOPL_MASK 0x00003000     // I/O privilege level
#define EFLAGS_NT_MASK 0x00004000       // nested task
#define EFLAGS_RF_MASK 0x00010000       // resume flag
#define EFLAGS_VM_MASK 0x00020000       // virtual 8086 mode
#define EFLAGS_AC_MASK 0x00040000       // alignment check
#define EFLAGS_VIF_MASK 0x00080000      // virtual interrupt flag
#define EFLAGS_VIP_MASK 0x00100000      // virtual interrupt pending
#define EFLAGS_ID_MASK 0x00200000       // identification flag

#define EFLAGS_TF_SHIFT 8               // trap
#define EFLAGS_IF_SHIFT 9               // interrupt enable



#define KGDT64_NULL (0 * 16)            // NULL descriptor
#define KGDT64_R0_CODE (1 * 16)         // kernel mode 64-bit code
#define KGDT64_R0_DATA (1 * 16) + 8     // kernel mode 64-bit data (stack)
#define KGDT64_R3_CMCODE (2 * 16)       // user mode 32-bit code
#define KGDT64_R3_DATA (2 * 16) + 8     // user mode 32-bit data
#define KGDT64_R3_CODE (3 * 16)         // user mode 64-bit code
#define KGDT64_SYS_TSS (4 * 16)         // kernel mode system task state
#define KGDT64_R3_CMTEB (5 * 16)        // user mode 32-bit TEB
#define KGDT64_R0_CMCODE (6 * 16)       // kernel mode 32-bit code
#define KGDT64_LAST (7 * 16)            // last entry


#define MODE_MASK 1                     // ntosp
#define RPL_MASK  3