#pragma once

namespace VT
{
	BOOL SuportVT();

	BOOL EnableVT();

	BOOL StartVT(PCWSTR vmdiskPath, DWORD32 cpuid);
}

