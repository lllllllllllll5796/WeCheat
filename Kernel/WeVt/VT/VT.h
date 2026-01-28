#pragma once

namespace VT
{
	BOOLEAN  SuportVT();

	BOOLEAN  EnableVT();

	BOOLEAN  StartVT(PCWSTR vmdiskPath, DWORD32 cpuid);
}

