#include "WeDebug.pch.h"
#include "WeDebug.Control.h"
#include "WeDebug.Symbolic.h"
#include "WeDebug.DbgkApi.h"

namespace Control
{
	BOOL InitDeviceIoControl()
	{
		LOG_DEBUG("[+] InitDeviceIoControl\r\n");

		NTSTATUS ntStatus = CreateDevice(Global::g_DriverObject);

		if (!NT_SUCCESS(ntStatus))
		{
			LOG_DEBUG("[-] 创建设备对象失败\n");
			return FALSE;
		}

		LOG_DEBUG("[+] 创建设备对象成功\n");

		Global::g_DriverObject->MajorFunction[IRP_MJ_CREATE] = InitDispatchRoutine;
		Global::g_DriverObject->MajorFunction[IRP_MJ_CLOSE] = InitDispatchRoutine;
		Global::g_DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandlerDispatchRoutine;

		return TRUE;
	}

	VOID UnInitDeviceIoControl()
	{
		RemoveDevice(Global::g_DriverObject);
	}

	//创建设备 符号链接等
	NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriver_Object)
	{
		NTSTATUS ntStatus;
		PDEVICE_OBJECT pDevObj;
		PDEVICE_EXTENSION pDevExt;

		//创建设备名称
		UNICODE_STRING devName;
		RtlInitUnicodeString(&devName, (L"\\Device\\WeDebug"));

		//创建设备
		ntStatus = IoCreateDevice(pDriver_Object,
			sizeof(DEVICE_EXTENSION),
			&devName,
			FILE_DEVICE_UNKNOWN,
			0,
			FALSE,
			&pDevObj);

		if (!NT_SUCCESS(ntStatus))
		{
			return ntStatus;
		}

		//创建直接读写设备
		pDevObj->Flags |= DO_BUFFERED_IO;
		pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
		pDevExt->pDevice = pDevObj;
		pDevExt->ustrDeviceName = devName;

		//创建符号链接
		UNICODE_STRING symLinkName;
		RtlInitUnicodeString(&symLinkName, (L"\\??\\WeDebug"));
		pDevExt->ustrSymLinkName = symLinkName;
		ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);
		if (!NT_SUCCESS(ntStatus))
		{
			IoDeleteDevice(pDevObj);  //创建失败删除设备
			return ntStatus;
		}
		return STATUS_SUCCESS;
	}

	//删除设备
	VOID RemoveDevice(IN PDRIVER_OBJECT pDriver_Object)
	{
	    PDEVICE_OBJECT	pNextObj;
	    pNextObj = pDriver_Object->DeviceObject;
	    while (pNextObj != NULL)
	    {
	        PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextObj->DeviceExtension;
	
	        //删除符号链接
	        UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
			LOG_DEBUG("[+] RemoveDevice:%wZ\n", pLinkName);

	        IoDeleteSymbolicLink(&pLinkName);
	        pNextObj = pNextObj->NextDevice;
	        IoDeleteDevice(pDevExt->pDevice);
	    }
	}

	NTSTATUS InitDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
	{
		NTSTATUS ntStatus = STATUS_SUCCESS;
		//得到当前堆栈
		PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
		//得到输入缓冲区大小
		ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
		//得到输出缓冲区大小
		ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
		//得到IOCTL码
		ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

		pIrp->IoStatus.Status = ntStatus;
		pIrp->IoStatus.Information = cbin;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return ntStatus;
	}

	NTSTATUS HandlerDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
	{
		NTSTATUS ntStatus = STATUS_SUCCESS;
		//得到当前堆栈
		PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
		//得到输入缓冲区大小
		ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
		//得到输出缓冲区大小
		ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
		//得到IOCTL码
		ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

		switch (code)
		{
			case IOCTL_WEDEBUG_Test:
			{
				LOG_DEBUG("[+] IOCTL_WEDEBUG_Test\n");
				//调用线程来自gui线程
 				PUSER_DATA userData = (PUSER_DATA)pIrp->AssociatedIrp.SystemBuffer;
 				HANDLE_WEDEBUG_Test(userData, pIrp);
				break;
			}
			case IOCTL_WEDEBUG_GetProcessCr3:
			{
				LOG_DEBUG("[+] IOCTL_WEDEBUG_GetProcessCr3\n");
				PUSER_DATA userData = (PUSER_DATA)pIrp->AssociatedIrp.SystemBuffer;
				HANDLE_WEDEBUG_GetProcessCr3(userData, pIrp);
				break;
			}
			case IOCTL_WEDEBUG_InitSymbols:
			{
				LOG_DEBUG("[+] IOCTL_WEDEBUG_InitSymbols\n");
				PUSER_DATA userData = (PUSER_DATA)pIrp->AssociatedIrp.SystemBuffer;
				HANDLE_WEDEBUG_InitSymbols(userData, pIrp);
				break;
			}
			case IOCTL_WEDEBUG_DbgkInit:
			{
				LOG_DEBUG("[+] IOCTL_WEDEBUG_DbgkInit\n");
				PUSER_DATA userData = (PUSER_DATA)pIrp->AssociatedIrp.SystemBuffer;
				HANDLE_WEDEBUG_DbgkInit(userData, pIrp);
				break;
			}
			default:
			{
				ntStatus = STATUS_INVALID_VARIANT;
				break;
			}
		}

		pIrp->IoStatus.Status = ntStatus;
		pIrp->IoStatus.Information = cbout;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return ntStatus;
	}

	USER_DATA GetUserData(PUSER_DATA userData)
	{
		USER_DATA user = { 0 };
		if (userData)
		{
			user.Count = userData->Count;
			user.uSize = userData->uSize;
			user.pUserData = userData->pUserData;
		}
		return user;
	}

	NTSTATUS HANDLE_WEDEBUG_Test(IN PUSER_DATA userData, IN PIRP pIrp)
	{
		LOG_DEBUG("[+] HANDLE_WEDEBUG_Test\r\n");

		USER_DATA user = GetUserData(userData);

		//分配明文缓存区
		BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
		DecryptData((PVOID)user.pUserData, aucPlainText);

		// 计算明文缓冲区中的结构体数量
		size_t numElements = user.uSize / sizeof(REQUEST_Test);

		// 遍历明文缓冲区中的结构体
		for (size_t i = 0; i < numElements; i++)
		{
			REQUEST_Test* pTest = reinterpret_cast<REQUEST_Test*>(aucPlainText + i * sizeof(REQUEST_Test));

			LOG_DEBUG("[+] InValue:0x%p\r\n", pTest->InValue);
		}

		REQUEST_Test* output = (REQUEST_Test*)pIrp->AssociatedIrp.SystemBuffer;  //内核的缓冲区，输入输出都用的这个
		RtlZeroMemory(output, sizeof(REQUEST_Test));
		
		output->OutValue = 0x666;

		return STATUS_SUCCESS;
	}

	NTSTATUS HANDLE_WEDEBUG_InitSymbols(IN PUSER_DATA userData, IN PIRP pIrp)
	{
		LOG_DEBUG("[+] HANDLE_WEDEBUG_InitSymbols\r\n");

		if (Global::g_IsInitSymbols == FALSE)
		{
			if (!InitNtoskrnlSymbolsTable())
			{
				return STATUS_UNSUCCESSFUL;
			}

			if (!InitWin32kBaseSymbolsTable())
			{
				return STATUS_UNSUCCESSFUL;
			}

			if (!InitWin32kFullSymbolsTable())
			{
				return STATUS_UNSUCCESSFUL;
			}

			CheckFunctionPointers();

			Global::g_IsInitSymbols = TRUE;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS HANDLE_WEDEBUG_DbgkInit(IN PUSER_DATA userData, IN PIRP pIrp)
	{
		LOG_DEBUG("[+] HANDLE_WEDEBUG_DbgkInit\r\n");

		if (Global::g_DbgkInitialized == FALSE)
		{
			NTSTATUS ntStatus = STATUS_SUCCESS;

			ntStatus = DbgkInitialize();

			if (NT_SUCCESS(ntStatus))
			{
				Global::g_DbgkInitialized = TRUE;

			}
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS HANDLE_WEDEBUG_GetProcessCr3(IN PUSER_DATA userData, IN PIRP pIrp)
	{
		LOG_DEBUG("[+] HANDLE_WEDEBUG_GetProcessCr3\r\n");

// 		USER_DATA user = GetUserData(userData);
// 
// 		//分配明文缓存区
// 		BYTE* aucPlainText = allocate_pool<BYTE*>(user.uSize);
// 		DecryptData((PVOID)user.pUserData, aucPlainText);
// 
// 		// 计算明文缓冲区中的结构体数量
// 		size_t numElements = user.uSize / sizeof(RING3_PROCESS_CR3);
// 
// 		// 遍历明文缓冲区中的结构体
// 		for (size_t i = 0; i < numElements; i++)
// 		{
// 			RING3_PROCESS_CR3* pInfo = reinterpret_cast<RING3_PROCESS_CR3*>(aucPlainText + i * sizeof(RING3_PROCESS_CR3));
// 
// 			if (pInfo->ProcessHandle)
// 			{
// 				PEPROCESS Process;
// 
// 				NTSTATUS Status = ObReferenceObjectByHandle((HANDLE)pInfo->ProcessHandle,
// 					PROCESS_ALL_ACCESS,
// 					*PsProcessType,
// 					KernelMode,
// 					(PVOID*)&Process,
// 					NULL);
// 
// 				if (!NT_SUCCESS(Status)) 
// 				{
// 					break;
// 				}
// 
// 				size_t ptr_DirectoryTableBase = (size_t)Process + kprocess_offset::DirectoryTableBase;
// 				if (ptr_DirectoryTableBase)
// 				{
// 					output->cr3 = *(size_t*)ptr_DirectoryTableBase;
// 				}
// 
// 				ObDereferenceObject(Process);
// 			}
// 		}
// 
// 		free_pool(aucPlainText);

		return STATUS_SUCCESS;
	}


}