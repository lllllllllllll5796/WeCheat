#pragma once
#include "Wdk.RtlStruct.inl"

namespace wdk
{
    extern"C"
    {

        PIMAGE_NT_HEADERS NTAPI 
            RtlImageNtHeader(
                _In_ PVOID Base
            );

        PVOID NTAPI
            RtlImageDirectoryEntryToData(
                _In_ PVOID Base,
                _In_ BOOLEAN MappedAsImage,
                _In_ USHORT DirectoryEntry,
                _Out_ PULONG Size
            );

        ULONG NTAPI
            RtlGetNtGlobalFlags(
                VOID
            );

        BOOLEAN NTAPI
            RtlIsSandboxedToken(
                _In_opt_ PSECURITY_SUBJECT_CONTEXT Context,
                _In_ KPROCESSOR_MODE PreviousMode
            );

//         NTSTATUS NTAPI
//             RtlConvertSidToUnicodeString(
//                 _Inout_ PUNICODE_STRING UnicodeString,
//                 _In_ PSID Sid,
//                 _In_ BOOLEAN AllocateDestinationString
//             );

        ULONG NTAPI
            RtlNtStatusToDosError(
                _In_ NTSTATUS Status
            );

        ULONG NTAPI
            RtlNtStatusToDosErrorNoTeb(
                _In_ NTSTATUS Status
            );

    }
}

namespace wdk
{
    extern"C"
    {
        
        inline constexpr auto Is64BitSystem() -> bool { return (sizeof(SIZE_T) == sizeof(UINT64)); };
        inline constexpr auto Is32BitSystem() -> bool { return !Is64BitSystem(); }

        inline bool IsWindowsXPOrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsXP)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsXPSP1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsXP)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsXPSP2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsXP)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsXPSP3OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsXP)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsVistaOrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsVista)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsVistaSP1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsVista_SP1)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsVistaSP2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::WindowsVista_SP2)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows7OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows7)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows7SP1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows7_SP1)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows8OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows8)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows8Point1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows8_1)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows10OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsThreshold1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1507)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsThreshold2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1511)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsRedstone1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1607)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsRedstone2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1703)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsRedstone3OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1709)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsRedstone4OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1803)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindowsRedstone5OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1809)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows19H1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1903)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows19H2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_1909)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows20H1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_2004)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows20H2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_20H2)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows21H1OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_21H1)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows21H2OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows10_21H2)
            {
                return true;
            }
            return false;
        }

        inline bool IsWindows11OrGreater()
        {
            if (GetSystemVersion() >= SystemVersion::Windows11)
            {
                return true;
            }
            return false;
        }

        inline auto RtlInitSystem() -> NTSTATUS
        {
            if (GetSystemVersion() == SystemVersion::Unknown)
            {
                return STATUS_NOT_SUPPORTED;
            }

            return STATUS_SUCCESS;
        }

    }
}
