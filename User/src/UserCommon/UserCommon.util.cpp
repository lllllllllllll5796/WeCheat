#include "UserCommon.pch.h"
#include "UserCommon.util.h"

int GetNumberOfProcessors()
{
	SYSTEM_INFO SysInfo = { 0 };
	GetSystemInfo(&SysInfo);
	return SysInfo.dwNumberOfProcessors;
}

void ReportSeriousError(LPCSTR lpText)
{
	MessageBoxA(NULL, lpText, oxorany("·¢Éú´íÎó:"), MB_ICONERROR | MB_SYSTEMMODAL);
}