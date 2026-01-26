#pragma once

#pragma region Version

#define WINDOWS_7                     7600
#define WINDOWS_7_SP1                 7601
#define WINDOWS_8					  9200
#define WINDOWS_8_1				      9600
#define WINDOWS_10_VERSION_THRESHOLD1 10240
#define WINDOWS_10_VERSION_THRESHOLD2 10586
#define WINDOWS_10_VERSION_REDSTONE1  14393
#define WINDOWS_10_VERSION_REDSTONE2  15063
#define WINDOWS_10_VERSION_REDSTONE3  16299
#define WINDOWS_10_VERSION_REDSTONE4  17134
#define WINDOWS_10_VERSION_REDSTONE5  17763
#define WINDOWS_10_VERSION_19H1		  18362
#define WINDOWS_10_VERSION_19H2		  18363
#define WINDOWS_10_VERSION_20H1		  19041
#define WINDOWS_10_VERSION_20H2		  19042
#define WINDOWS_10_VERSION_21H1		  19043
#define WINDOWS_10_VERSION_21H2		  19044
#define WINDOWS_10_VERSION_22H1		  19045
#define	WINDOWS_10_VERSION_20H2_19569 19569
#define	WINDOWS_10_VERSION_21H1_20150 20150
#define	WINDOWS_10_VERSION_21H1_20180 20180
#define	WINDOWS_10_VERSION_XXXX_21292 21292
#define WINDOWS_11_VERSION_21H2		  22000
#define WINDOWS_11_VERSION_22H2		  22621
#define WINDOWS_11_VERSION_23H2		  22631

enum class SystemVersion : UINT32
{
	Unknown,                    // unknown

	WindowsXP,                  // 5.1.2600
	WindowsXP64,                // 5.2.3790

	WindowsVista,               // 6.0.6000
	WindowsVista_SP1,           // 6.0.6001
	WindowsVista_SP2,           // 6.0.6002

	Windows7,                   // 6.1.7600
	Windows7_SP1,               // 6.1.7601

	Windows8,                   // 6.2.9200
	Windows8_1,                 // 6.3.9600

	Windows10,                  // 10.0.10240
	Windows10_1507 = Windows10, // 10.0.10240
	Windows10_1511,             // 10.0.10586
	Windows10_1607,             // 10.0.14393
	Windows10_1703,             // 10.0.15063
	Windows10_1709,             // 10.0.16299
	Windows10_1803,             // 10.0.17134
	Windows10_1809,             // 10.0.17763
	Windows10_1903,             // 10.0.18362
	Windows10_1909,             // 10.0.18363
	Windows10_2004,             // 10.0.19041
	Windows10_20H2,             // 10.0.19042
	Windows10_21H1,             // 10.0.19043
	Windows10_21H2,             // 10.0.19044
	Windows10_22H2,             // 10.0.19045

	Windows10_20H2_19569,       // 10.0.19569
	Windows10_21H1_20150,       // 10.0.20150
	Windows10_21H1_20180,       // 10.0.20180
	Windows10_XXXX_21292,       // 10.0.21292

	Windows11,                  // 11.0.22000
	Windows11_21H2 = Windows11, // 11.0.22000
	Windows11_22H2,             // 11.0.22621
	Windows11_23H2,             // 11.0.22631
	WindowsMax,
};

__declspec(selectany) SystemVersion NtSystemVersion = SystemVersion::Unknown;

extern decltype(&RtlGetVersion) RtlGetVersionFn;

//
// Indicates the safe boot init value. Possible values are:
//  0   The operating system is not in Safe Mode.
//  1   SAFEBOOT_MINIMAL
//  2   SAFEBOOT_NETWORK
//  3*  SAFEBOOT_DSREPAIR
//
extern "C" PULONG InitSafeBootMode;

inline SystemVersion GetSystemVersion()
{
	NTSTATUS vStatus = STATUS_UNSUCCESSFUL;

	RTL_OSVERSIONINFOW  vVersion;

	vStatus = RtlGetVersionFn(&vVersion);

	if (STATUS_SUCCESS != vStatus)
	{
		//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "RtlGetVersion Error = 0x%X\n", vStatus);
		return SystemVersion::Unknown;
	}

	switch (vVersion.dwBuildNumber)
	{
	default:
		//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "System Version = %d\n",vVersion.dwBuildNumber);
		break;
	case 2600:
		NtSystemVersion = SystemVersion::WindowsXP;
		break;
	case 3790:
		NtSystemVersion = SystemVersion::WindowsXP64;
		break;
	case 6000:
		NtSystemVersion = SystemVersion::WindowsVista;
		break;
	case 6001:
		NtSystemVersion = SystemVersion::WindowsVista_SP1;
		break;
	case 6002:
		NtSystemVersion = SystemVersion::WindowsVista_SP2;
		break;
	case 7600:
		NtSystemVersion = SystemVersion::Windows7;
		break;
	case 7601:
		NtSystemVersion = SystemVersion::Windows7_SP1;
		break;
	case 9200:
		NtSystemVersion = SystemVersion::Windows8;
		break;
	case 9600:
		NtSystemVersion = SystemVersion::Windows8_1;
		break;
	case 10240:
		NtSystemVersion = SystemVersion::Windows10;
		break;
	case 10586:
		NtSystemVersion = SystemVersion::Windows10_1511;
		break;
	case 14393:
		NtSystemVersion = SystemVersion::Windows10_1607;
		break;
	case 15063:
		NtSystemVersion = SystemVersion::Windows10_1703;
		break;
	case 16299:
		NtSystemVersion = SystemVersion::Windows10_1709;
		break;
	case 17134:
		NtSystemVersion = SystemVersion::Windows10_1803;
		break;
	case 17763:
		NtSystemVersion = SystemVersion::Windows10_1809;
		break;
	case 18362:
		NtSystemVersion = SystemVersion::Windows10_1903;
		break;
	case 18363:
		NtSystemVersion = SystemVersion::Windows10_1909;
		break;
	case 19041:
		NtSystemVersion = SystemVersion::Windows10_2004;
		break;
	case 19042:
		NtSystemVersion = SystemVersion::Windows10_20H2;
		break;
	case 19043:
		NtSystemVersion = SystemVersion::Windows10_21H1;
		break;
	case 19044:
		NtSystemVersion = SystemVersion::Windows10_21H2;
		break;
	case 19045:
		NtSystemVersion = SystemVersion::Windows10_22H2;
		break;
	case 19569:
		NtSystemVersion = SystemVersion::Windows10_20H2_19569;
		break;
	case 20150:
		NtSystemVersion = SystemVersion::Windows10_21H1_20150;
		break;
	case 20180:
		NtSystemVersion = SystemVersion::Windows10_21H1_20180;
		break;
	case 21292:
		NtSystemVersion = SystemVersion::Windows10_XXXX_21292;
		break;
	case 22000:
		NtSystemVersion = SystemVersion::Windows11_21H2;
		break;
	case 22621:
		NtSystemVersion = SystemVersion::Windows11_22H2;
		break;
	case 22631:
		NtSystemVersion = SystemVersion::Windows11_23H2;
		break;
	}

	return NtSystemVersion;
}

#pragma endregion