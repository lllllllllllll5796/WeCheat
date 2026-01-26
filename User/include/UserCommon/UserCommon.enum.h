#pragma once

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
	WindowsMax,
};