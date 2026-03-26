#include "WeVt.pch.h"
#include "WeVt.Trace.h"
#include "WeVt.Main.tmh"
#include "WeVt.Hypervisor.h"
#include "WeVt.Init.Symbolic.h"

EXTERN_C
VOID DriverUnLoad(__in DRIVER_OBJECT* DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	//Hypervisor::DisableVT();

	Global::UnInitialize_Global();

	LogDestroy();

	_cexit();

#if ENABLE_TRACE
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] DriverUnLoad End");
#endif

	WPP_CLEANUP(DriverObject);
}

EXTERN_C
NTSTATUS
DriverEntry(__in DRIVER_OBJECT* DriverObject, __in UNICODE_STRING* RegistryPath)
{
	WPP_INIT_TRACING(DriverObject, RegistryPath);

#if ENABLE_TRACE
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, oxorany("[+] DriverEntry"));
#endif

	sLog("\n");

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	ULONG LogLevel = LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend;

	//
	// Make sure we are not running in safe mode!
	//
	if (*InitSafeBootMode != 0)
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, oxorany("[-] InitSafeBootMode"));
#endif
		return STATUS_NOT_SUPPORTED;
	}

#if EMPTY_DRIVER
	DriverObject->DriverUnload = DriverUnLoad;
#else
	if (RegistryPath)
	{
		DriverObject->DriverUnload = DriverUnLoad;

		LDR_DATA_TABLE_ENTRY* v_fist_entry = nullptr;
		LDR_DATA_TABLE_ENTRY* v_target_entry = nullptr;
		auto v_self_entry = static_cast<LDR_DATA_TABLE_ENTRY*>(DriverObject->DriverSection);


		Global::g_DriverObject = DriverObject;
		Global::g_DriverBase = (ULONG64)v_self_entry->DllBase;
		Global::g_DriverSize = v_self_entry->SizeOfImage;
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] g_DriverBase:0x%llX g_DriverSize:0x%X\r\n", Global::g_DriverBase, Global::g_DriverSize);
#endif
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


		Global::g_KernelBase = (ULONG64)v_target_entry->DllBase;
		Global::g_KernelSize = (ULONG32)v_target_entry->SizeOfImage;

#if ENABLE_LOG
		Status = LogInitialize(LogLevel, L"\\??\\C:\\WeVt_Log.log");
#endif

		if (_cinit() != 0)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] _cinit Error");
#endif
			return STATUS_APP_INIT_FAILURE;
		}

		Status = wdk::WdkInitSystem();

		if (!NT_SUCCESS(Status))
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] WdkInitSystem Error");
#endif
			return Status;
		}

		if (!Global::Initialize_Global())
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Initialize_Global Error");
#endif
			return STATUS_UNSUCCESSFUL;
		}

		if (InitNtoskrnlSymbolsTable())
		{

		}

		//---------------------------------------
		// Æô¶¯ VT ÐéÄâ»¯
//		if (!Hypervisor::EnableVT())
//		{
//#if ENABLE_TRACE
//			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] EnableVT failed");
//#endif
//			LOG_DEBUG("[-] EnableVT failed\r\n");
//			return STATUS_UNSUCCESSFUL;
//		}
		//--------------------------------------

	}
#endif  //EmptyDriver

    return STATUS_SUCCESS;
}