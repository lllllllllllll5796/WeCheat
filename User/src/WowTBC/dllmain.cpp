#include "pch.h"
#include "dllmain.h"

typedef int(*fnEnumVisibleObjects)(void* p, int filter);

int EnumObjectsCallbackProc(QWORD objectPtr, QWORD filter)
{
	if (objectPtr)
	{
		try
		{
			char strObj[50] = { 0 };
			sprintf_s(strObj, "\'0x%llX\'", objectPtr);

			LOG_DEBUG("%s\n",strObj);

			//g_strTmp_ObjectsTable = strObj;
			//g_strTmp_ObjectsTable += ",";
			//g_strBuf_ObjectsTable += g_strTmp_ObjectsTable;

			//g_ObjectsCount++;

			return 1;
		}
		catch (...)
		{
			return 0;
		}
	}
	return 0;
}

QWORD CallEnumVisibleObjects(void* CallbackFunc, QWORD Filter)
{
	ULONG64 ModuleBase = (ULONG64)GetModuleHandleA("WowClassic.exe");
	if (ModuleBase)
	{
		try
		{
			fnEnumVisibleObjects EnumVisibleObjects = (fnEnumVisibleObjects)(ModuleBase + 0x194D490);
			return EnumVisibleObjects(CallbackFunc, Filter);
		}
		catch (...)
		{
			OutputDebugStringEx("%s Error Line = %d\n", __FUNCTION__, __LINE__);
		}
	}
	 
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			LOG_DEBUG("111111");
			CallEnumVisibleObjects(EnumObjectsCallbackProc, 0);
		}
		break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}