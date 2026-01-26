#pragma once

typedef NTSTATUS(*MyDriverEntry)(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString);

__declspec(selectany) MyDriverEntry MyEntry = NULL;
__declspec(selectany) PIMAGE_DOS_HEADER		 g_pDosHeader = NULL;
__declspec(selectany) PIMAGE_NT_HEADERS64		 g_pNTHeader = NULL;
__declspec(selectany) PIMAGE_SECTION_HEADER	 g_pSectionHeader = NULL;
__declspec(selectany) PLDR_DATA_TABLE_ENTRY64  LdrEntryPoint = NULL;

EXTERN_C BOOLEAN IsMapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString);
EXTERN_C NTSTATUS MapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString);
//HANDLE KLoadLibrary(const wchar_t * full_dll_path);
PVOID DumpDriverToMemory(PDRIVER_OBJECT pDriverObject);
PVOID LoadFileToMemory(PWCHAR FullPath, PSIZE_T psBufferLength);
PVOID MapDriver(PVOID pBuffer,PVOID KernelBase);
ULONG GetIndexFromExportTable(PVOID pBaseAddress, PCHAR pszFunctionName);
INT64 GetFuncAddrFromExportTable(PVOID pBaseAddress, PCHAR pszFunctionName);
EXTERN_C BOOLEAN InitFuncAddr(PDRIVER_OBJECT pDriverOjb);

//
//NTSTATUS MapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString) {
//
//
//	PVOID BaseAddr = NULL;
//	NTSTATUS status = STATUS_UNSUCCESSFUL;
//
//	BaseAddr = DumpDriverToMemory(pDriverObject);
//	if (BaseAddr == NULL) {
//		//KdPrint(("DumpDriverToMemory Failed..\n", BaseAddr));
//		return status;
//	}
//	//KdPrint(("DumpDriverToMemory = %p\n", BaseAddr));
//
//
//	BaseAddr = MapDriver(BaseAddr);
//	if (BaseAddr == NULL) {
//		//KdPrint(("MapDriver Failed..\n", BaseAddr));
//		return status;
//	}
//
//	//以此为标记，如果length为0，代表此驱动已被加载过
//	pRegistryString->Length = 0;
//	LDR_DATA_TABLE_ENTRY* v_fist_entry = nullptr;
//	LDR_DATA_TABLE_ENTRY* v_target_entry = nullptr;
//	auto v_self_entry = static_cast<LDR_DATA_TABLE_ENTRY*>(pDriverObject->DriverSection);
//	v_fist_entry = v_self_entry;
//	do
//	{
//		if (v_self_entry->BaseDllName.Buffer != nullptr)
//		{
//			if (StrStr(v_self_entry->BaseDllName.Buffer, E(L"ntoskrnl")))
//			{
//				v_target_entry = v_self_entry;
//				break;
//			}
//			v_self_entry = reinterpret_cast<LDR_DATA_TABLE_ENTRY*>(v_self_entry->InLoadOrderLinks.Blink);
//		}
//	} while (v_self_entry->InLoadOrderLinks.Blink != reinterpret_cast<PLIST_ENTRY>(v_fist_entry));
//
//	ULONG64 Encode_Addr = ((ULONG64)v_target_entry->DllBase & 0xFFFFFFFFFFF) / 0x1000 * 0x100000000;
//	memcpy(PVOID((ULONG64)pRegistryString + 2 * sizeof(unsigned short)), &Encode_Addr, 8);
//	
//	MyEntry = (MyDriverEntry)BaseAddr;
//	status = MyEntry(pDriverObject, pRegistryString);
//
//
//	return status;
//
//
//
//}
//

template<class Type>
void GetBinaryData(IN PVOID pBuffer, uintptr_t Offset, size_t Index, Type& Data)
{
	RtlCopyMemory((void*)&Data, (PBYTE)pBuffer + Offset + sizeof(Type) * Index, sizeof(Type));
}

bool CheckHeader(IN PVOID pBuffer, size_t& OffsetNtHeader, IMAGE_NT_HEADERS& ntHd);

BOOLEAN IsMapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString);

NTSTATUS MapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString);

/*
	将指定路径的文件映射到内存
	这个应该只是简单的类似于读文件操作？没有节表的映射
*/
HANDLE KLoadLibrary(const wchar_t *full_dll_path);

/*
	内存需要手动释放
*/
PVOID DumpDriverToMemory(PDRIVER_OBJECT pDriverObject);


//将文件加载到内存，psBufferLength返回文件长度，函数返回值为文件加载地址
PVOID LoadFileToMemory(PWCHAR FullPath, PSIZE_T psBufferLength);


PVOID MapDriver(PVOID pBuffer, PVOID KernelBase);


// 根据导出表获取导出函数地址, 从而获取 SSDT 函数索引号
ULONG GetIndexFromExportTable(PVOID pBaseAddress, PCCHAR pszFunctionName);

// 根据导出表获取导出函数地址
INT64 GetFuncAddrFromExportTable(PVOID pBaseAddress, PCCHAR pszFunctionName);



//比较字符串
BOOLEAN xstricmp(LPCSTR s1, LPCSTR s2);


//获取驱动模块地址
//PVOID GetKernelModuleBaseByName(LPCSTR driverName)
//{
//
//	//NTSTATUS status = STATUS_SUCCESS;
//	//PLDR_DATA_TABLE_ENTRY64 FirstEntry = NULL;
//	//ANSI_STRING AnsiSec = { 0 };
//	//UNICODE_STRING DriverName = { 0 };
//
//	//PVOID imgBase = NULL;
//
//
//	//FirstEntry = (PLDR_DATA_TABLE_ENTRY64)LdrEntryPoint->InLoadOrderLinks.Flink;
//
//	//RtlInitAnsiString(&AnsiSec, driverName);
//	//status = RtlAnsiStringToUnicodeString(&DriverName, &AnsiSec, TRUE);
//	//if (!NT_SUCCESS(status)) {
//	//	return NULL;
//	//}
//
//
//	//do {
//	//	//__try
//	//	//{
//	//	if (RtlCompareUnicodeStringFn(&DriverName, (PUNICODE_STRING)&(FirstEntry->BaseDllName), TRUE) == 0)
//	//	{
//	//		imgBase = (PVOID)FirstEntry->DllBase;
//	//		break;
//	//	}
//	//	//}
//	//	//__except (1) {}
//
//
//	//	FirstEntry = (PLDR_DATA_TABLE_ENTRY64)FirstEntry->InLoadOrderLinks.Flink;
//
//
//	//} while ((ULONGLONG)LdrEntryPoint->InLoadOrderLinks.Flink != (ULONGLONG)FirstEntry);
//
//	//RtlFreeUnicodeString(&DriverName);
//
//	//return imgBase;
//	return 0;
//}



BOOLEAN InitFuncAddr(PDRIVER_OBJECT pDriverOjb);







