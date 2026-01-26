#include "WeDebug.pch.h"
#include "WeDebug.Control.h"
#include "WeDebug.DbgkApi.h"

CODE_OBF_MFLT
EXTERN_C
VOID DriverUnLoad(__in DRIVER_OBJECT* DriverObject)
{
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("[+] WeDebug DriverUnLoad\r\n"));

	DbgkUnInitialize();

#if EMPTY_DRIVER

#else
	Control::UnInitDeviceIoControl();

	Global::UnInitialize_Global();

	LogDestroy();

	_cexit();
#endif
}

CODE_OBF_MFLT
EXTERN_C
NTSTATUS
DriverEntry(__in DRIVER_OBJECT* DriverObject, __in UNICODE_STRING* RegistryPath) 
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG LogLevel =  LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend;



	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("[+] WeDebug DriverEntry\r\n"));

	//
	// Make sure we are not running in safe mode!
	//
	if (*InitSafeBootMode != 0) 
	{
		return STATUS_NOT_SUPPORTED;
	}

#if EMPTY_DRIVER
	DriverObject->DriverUnload = DriverUnLoad;
#else
	if (RegistryPath)
	{
		//驱动无模块
#if DRIVER_NO_MODULE
		InitFuncAddr(reinterpret_cast<PDRIVER_OBJECT>(DriverObject));

		if (!IsMapSelf(reinterpret_cast<PDRIVER_OBJECT>(DriverObject), reinterpret_cast<PUNICODE_STRING>(RegistryPath)))
		{
			Status = MapSelf(reinterpret_cast<PDRIVER_OBJECT>(DriverObject), reinterpret_cast<PUNICODE_STRING>(RegistryPath));

			//清除一些驱动加载信息
			ClearDriverInstallMark(DriverObject);

			if (!NT_SUCCESS(Status))
			{
				bool SuccessFlag = true;
			}

			return STATUS_NOT_SUPPORTED;
		}

		Global::g_KernelBase = (((ULONG64)(reinterpret_cast<PUNICODE_STRING>(RegistryPath)->Buffer) & 0xFFFFFFFF) * 0x1000) | 0xFFFFF00000000000;

		KBase2 = (PVOID)Global::g_KernelBase;
#else
		DriverObject->DriverUnload = DriverUnLoad;

		LDR_DATA_TABLE_ENTRY* v_fist_entry = nullptr;
		LDR_DATA_TABLE_ENTRY* v_target_entry = nullptr;
		auto v_self_entry = static_cast<LDR_DATA_TABLE_ENTRY*>(DriverObject->DriverSection);

		//驱动自己的信息
		Global::g_DriverObject = DriverObject;
		Global::g_DriverBase = (ULONG64)v_self_entry->DllBase;
		Global::g_DriverSize = v_self_entry->SizeOfImage;

		//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "[+] g_DriverBase:0x%llX g_DriverSize:0x%X\r\n", Global::g_DriverBase, Global::g_DriverSize);

		v_fist_entry = v_self_entry;
		do
		{
			if (v_self_entry->BaseDllName.Buffer != nullptr)
			{
				if (StrStr((wchar_t*)(v_self_entry->BaseDllName.Buffer), oxorany(L"ntoskrnl")))
				{
					v_target_entry = v_self_entry;
					break;
				}
				v_self_entry = reinterpret_cast<LDR_DATA_TABLE_ENTRY*>(v_self_entry->InLoadOrderLinks.Blink);
			}
		} while (v_self_entry->InLoadOrderLinks.Blink != reinterpret_cast<PLIST_ENTRY>(v_fist_entry));

		//这玩意是系统内核的地址和大小
		Global::g_KernelBase = (ULONG64)v_target_entry->DllBase;
		Global::g_KernelSize = (ULONG32)v_target_entry->SizeOfImage;

		//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "[+] KernelBase:0x%llX KernelSize:0x%X\r\n", Global::g_KernelBase, Global::g_KernelSize);
#endif
		//初始化导入函数
		InitializeHideImport(Global::g_KernelBase);

		Status = LogInitialize(LogLevel, L"\\??\\C:\\Log.log");

		if (!NT_SUCCESS(Status)) {
			DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "[+] Failed to initialize logging interface with status 0x%08x\r\n", Status);
			return Status;
		}

		LOG_INFO("[+] ********************************************************\r\n");
		LOG_INFO("[+] *                www.woaidaima.com                     *\r\n");
		LOG_INFO("[+] *                  我爱代码论坛                        *\r\n");
		LOG_INFO("[+] *                    WeDebug                           *\r\n");
		LOG_INFO("[+] ********************************************************\r\n");

		if (_cinit() != 0)
		{
			LOG_ERROR("[-] _cinit\r\n");
			return STATUS_APP_INIT_FAILURE;
		}



		Status = wdk::WdkInitSystem();

		if (!NT_SUCCESS(Status))
		{
			LOG_ERROR("[-] [1] WdkInitSystem 初始化失败\r\n");
			return Status;
		}

		if (!Global::Initialize_Global())
		{
			LOG_ERROR("[-] [2] 全局初始化失败\r\n");
			return STATUS_UNSUCCESSFUL;
		}

		if (!Control::InitDeviceIoControl())
		{
			LOG_DEBUG("[-] [3] InitDeviceIoControl 失败\r\n");
			return STATUS_UNSUCCESSFUL;
		}
	}

#endif  //EmptyDriver

    return STATUS_SUCCESS;
}