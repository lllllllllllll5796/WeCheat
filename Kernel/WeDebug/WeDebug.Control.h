#pragma once

namespace Control
{
	typedef struct _DEVICE_EXTENSION {
		PDEVICE_OBJECT pDevice;
		UNICODE_STRING ustrDeviceName;	//设备名称
		UNICODE_STRING ustrSymLinkName;	//符号链接名
		PUCHAR buffer;//缓冲区
		ULONG file_length;//模拟的文件长度，必须小于MAX_FILE_LENGTH
	} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

	BOOL InitDeviceIoControl();
	VOID UnInitDeviceIoControl();

	NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriver_Object);
	VOID RemoveDevice(IN PDRIVER_OBJECT pDriver_Object);

	NTSTATUS InitDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
	NTSTATUS HandlerDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

	NTSTATUS HANDLE_WEDEBUG_Test(IN PUSER_DATA userData, IN PIRP pIrp);
	NTSTATUS HANDLE_WEDEBUG_GetProcessCr3(IN PUSER_DATA userData, IN PIRP pIrp);
	NTSTATUS HANDLE_WEDEBUG_InitSymbols(IN PUSER_DATA userData, IN PIRP pIrp);
	NTSTATUS HANDLE_WEDEBUG_DbgkInit(IN PUSER_DATA userData, IN PIRP pIrp);
}