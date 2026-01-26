#include "pch.h"
#include "Ioctl.h"
#include "Global.h"

//派遣数据到驱动
BOOL DispatchDataToDriver(DWORD dwIoControlCode,
	PUSER_DATA userData,
	PVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	if (Global::g_hGeneralDriverDevice != INVALID_HANDLE_VALUE)
	{
		LOG_DEBUG("DispatchDataToDriver dwIoControlCode:0x%X", dwIoControlCode);

		bRet = DeviceIoControl(Global::g_hGeneralDriverDevice,
			dwIoControlCode,
			userData,
			sizeof(USER_DATA),
			lpOutBuffer,
			nOutBufferSize,
			lpBytesReturned,
			NULL);

		DWORD dwError = GetLastError();

		LOG_DEBUG("DispatchDataToDriver dwError:0x%X", dwError);
	}
	return bRet;
}

BOOL SendUserDataToDriver(DWORD dwIoControlCode,
	PVOID source,
	SIZE_T size,
	PVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned)
{
	BOOL bRet;
	std::string encodeData;
	USER_DATA userData = { 0 };
	userData.uSize = size;  //记录明文长度
	if (source)
	{
		encodeData = EncryptData((const char*)source, size, KEY);
		userData.pUserData = (ULONG64)encodeData.c_str();
	}

	bRet = DispatchDataToDriver(dwIoControlCode,
		&userData,
		lpOutBuffer,
		nOutBufferSize,
		lpBytesReturned);

	return bRet;
}

void Test()
{
	DWORD Returned = DWORD(0);

	REQUEST_Test Data;
	Data.InValue = 0x999;

	BOOL OK = SendUserDataToDriver(IOCTL_WEDEBUG_Test,
		&Data,
		sizeof(REQUEST_Test),
		&Data,
		sizeof(REQUEST_Test),
		&Returned);

	if (!OK)
	{
		LOG_DEBUG("[-] 派遣通信失败 设备句柄:0x%p\r\n", Global::g_hGeneralDriverDevice);
		return;
	}

	LOG_DEBUG("OutValue:0x%p", Data.OutValue);
}

void InitSymbols()
{
	DWORD Returned = DWORD(0);

	BOOL OK = SendUserDataToDriver(IOCTL_WEDEBUG_InitSymbols,
		NULL,
		0,
		NULL,
		0,
		&Returned);

	if (!OK)
	{
		LOG_DEBUG("[-] 派遣通信失败 设备句柄:0x%p\r\n", Global::g_hGeneralDriverDevice);
		return;
	}
}

void InitDebug()
{
	DWORD Returned = DWORD(0);
	BOOL OK = SendUserDataToDriver(IOCTL_WEDEBUG_DbgkInit,
		NULL,
		0,
		NULL,
		0,
		&Returned);
	if (!OK)
	{
		LOG_DEBUG("[-] 派遣通信失败 设备句柄:0x%p\r\n", Global::g_hGeneralDriverDevice);
		return;
	}
}
