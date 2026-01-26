#pragma once

namespace wdk
{
#pragma region Module
    typedef struct _RTL_PROCESS_MODULE_INFORMATION
    {
        HANDLE Section;
        PVOID MappedBase;
        PVOID ImageBase;
        ULONG ImageSize;
        ULONG Flags;
        USHORT LoadOrderIndex;
        USHORT InitOrderIndex;
        USHORT LoadCount;
        USHORT OffsetToFileName;
        UCHAR FullPathName[256];
    } RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

    typedef struct _RTL_PROCESS_MODULES
    {
        ULONG NumberOfModules;
        RTL_PROCESS_MODULE_INFORMATION Modules[1];
    } RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

    // private
    typedef struct _RTL_PROCESS_MODULE_INFORMATION_EX
    {
        USHORT NextOffset;
        RTL_PROCESS_MODULE_INFORMATION BaseInfo;
        ULONG ImageChecksum;
        ULONG TimeDateStamp;
        PVOID DefaultBase;
    } RTL_PROCESS_MODULE_INFORMATION_EX, *PRTL_PROCESS_MODULE_INFORMATION_EX;
#pragma endregion


#pragma region PEB-UserParameters
    template<typename T = PVOID>
    struct CURDIR_T
    {
        using UNICODE_STRING = T_STRING<T>;

        UNICODE_STRING DosPath;
        T Handle;
    }; /* size: 0x0018 */ /* size: 0x000c */

    using CURDIR    = CURDIR_T<PVOID>;
    using CURDIR32  = CURDIR_T<PVOID32>;
    using CURDIR64  = CURDIR_T<PVOID64>;

    using PCURDIR   = CURDIR * ;
    using PCURDIR32 = CURDIR32 * ;
    using PCURDIR64 = CURDIR64 * ;

    static_assert(sizeof(CURDIR32) == 0x000c);
    static_assert(sizeof(CURDIR64) == 0x0018);
    

    template<typename T = PVOID>
    struct RTL_DRIVE_LETTER_CURDIR_T
    {
        using UNICODE_STRING = T_STRING<T>;

        UINT16 Flags;
        UINT16 Length;
        UINT32 TimeStamp;
        UNICODE_STRING DosPath;
    }; /* size: 0x0018 */ /* size: 0x0010 */

    using RTL_DRIVE_LETTER_CURDIR   = RTL_DRIVE_LETTER_CURDIR_T<PVOID>;
    using RTL_DRIVE_LETTER_CURDIR32 = RTL_DRIVE_LETTER_CURDIR_T<PVOID32>;
    using RTL_DRIVE_LETTER_CURDIR64 = RTL_DRIVE_LETTER_CURDIR_T<PVOID64>;

    using PRTL_DRIVE_LETTER_CURDIR   = RTL_DRIVE_LETTER_CURDIR * ;
    using PRTL_DRIVE_LETTER_CURDIR32 = RTL_DRIVE_LETTER_CURDIR32 * ;
    using PRTL_DRIVE_LETTER_CURDIR64 = RTL_DRIVE_LETTER_CURDIR64 * ;

    static_assert(sizeof(RTL_DRIVE_LETTER_CURDIR32) == 0x0010);
    static_assert(sizeof(RTL_DRIVE_LETTER_CURDIR64) == 0x0018);


    enum : UINT32
    {
        RtlMaxDriveLetters = 32
    };


    template<typename T = PVOID, typename I = SIZE_T>
    struct RTL_USER_PROCESS_PARAMETERS_T
    {
        using CURDIR            = CURDIR_T<T>;
        using UNICODE_STRING    = T_STRING<T>;
        using CURDIR            = CURDIR_T<T>;
        using RTL_DRIVE_LETTER_CURDIR = RTL_DRIVE_LETTER_CURDIR_T<T>;

        UINT32 MaximumLength;
        UINT32 Length;
        UINT32 Flags;
        UINT32 DebugFlags;
        T      ConsoleHandle;
        UINT32 ConsoleFlags;
        T StandardInput;
        T StandardOutput;
        T StandardError;
        CURDIR CurrentDirectory;
        UNICODE_STRING DllPath;
        UNICODE_STRING ImagePathName;
        UNICODE_STRING CommandLine;
        T Environment;
        UINT32 StartingX;
        UINT32 StartingY;
        UINT32 CountX;
        UINT32 CountY;
        UINT32 CountCharsX;
        UINT32 CountCharsY;
        UINT32 FillAttribute;
        UINT32 WindowFlags;
        UINT32 ShowWindowFlags;
        UNICODE_STRING WindowTitle;
        UNICODE_STRING DesktopInfo;
        UNICODE_STRING ShellInfo;
        UNICODE_STRING RuntimeData;
        RTL_DRIVE_LETTER_CURDIR CurrentDirectores[RtlMaxDriveLetters];
        I EnvironmentSize;
        I EnvironmentVersion;
        T PackageDependencyData;
        UINT32 ProcessGroupId;
        UINT32 LoaderThreads;
        UNICODE_STRING RedirectionDllName;
    }; /* size: 0x0420 */ /* size: 0x02ac */

    using RTL_USER_PROCESS_PARAMETERS   = RTL_USER_PROCESS_PARAMETERS_T<PVOID, SIZE_T>;
    using RTL_USER_PROCESS_PARAMETERS32 = RTL_USER_PROCESS_PARAMETERS_T<PVOID32, UINT32>;
    using RTL_USER_PROCESS_PARAMETERS64 = RTL_USER_PROCESS_PARAMETERS_T<PVOID64, UINT64>;

    using PRTL_USER_PROCESS_PARAMETERS   = RTL_USER_PROCESS_PARAMETERS * ;
    using PRTL_USER_PROCESS_PARAMETERS32 = RTL_USER_PROCESS_PARAMETERS32 * ;
    using PRTL_USER_PROCESS_PARAMETERS64 = RTL_USER_PROCESS_PARAMETERS64 * ;

    static_assert(sizeof(RTL_USER_PROCESS_PARAMETERS32) == 0x02ac);
    static_assert(sizeof(RTL_USER_PROCESS_PARAMETERS64) == 0x0420);
#pragma endregion
}