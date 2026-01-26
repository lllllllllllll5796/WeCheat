#include "KernelCommon.pch.h"
#include "MapLoad.h"



bool CheckHeader(IN PVOID pBuffer, size_t& OffsetNtHeader, IMAGE_NT_HEADERS& ntHd)
{
	IMAGE_DOS_HEADER dosHd;
	GetBinaryData(pBuffer, 0, 0, dosHd);
	if (dosHd.e_magic != IMAGE_DOS_SIGNATURE)
	{
		//LOG_DEBUG("Invalid DOS signature\r\n");
		return false;
	}

	OffsetNtHeader = dosHd.e_lfanew;
	GetBinaryData(pBuffer, OffsetNtHeader, 0, ntHd);
	if (ntHd.Signature != IMAGE_NT_SIGNATURE)
	{
		//LOG_DEBUG("Invalid NT signature\r\n");
		return false;
	}

	if (!(ntHd.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE))
	{
		//LOG_DEBUG("File is not executable\r\n");
		return false;
	}

	if (ntHd.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		//LOG_DEBUG("File is not 64bit application\r\n");
		return false;
	}

	return true;
}

BOOLEAN IsMapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString)
{
	return pRegistryString->Length == 0;
}

NTSTATUS MapSelf(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryString)
{
	LDR_DATA_TABLE_ENTRY* v_fist_entry = nullptr;
	LDR_DATA_TABLE_ENTRY* v_target_entry = nullptr;
	auto v_self_entry = static_cast<LDR_DATA_TABLE_ENTRY*>(pDriverObject->DriverSection);
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

	ULONG64 KernelBase = (ULONG64)v_target_entry->DllBase;

	g_KernelBase = (PVOID)KernelBase;

	ImpSet(DbgPrint);
	ImpSet(DbgPrintEx);

	ImpSet(ExAllocatePool);
	ImpSet(ExAllocatePoolWithTag);
	ImpSet(ExFreePoolWithTag);
	ImpSet(ExFreePool);

	ImpSet(IoGetCurrentProcess);

	ImpSet(MmGetVirtualForPhysical);
	ImpSet(MmIsAddressValid);
	ImpSet(MmCopyVirtualMemory);

	ImpSet(ObfDereferenceObject);

	ImpSet(PsLookupProcessByProcessId);
	ImpSet(PsReferencePrimaryToken);
	ImpSet(PsDereferencePrimaryToken);
	ImpSet(PsInitialSystemProcess);
	ImpSet(PsGetProcessPeb);

	ImpSet(MmGetSystemRoutineAddress);
								 
	ImpSet(RtlLookupElementGenericTableAvl);
	ImpSet(RtlDeleteElementGenericTableAvl);
	ImpSet(RtlGetVersion);
	ImpSet(RtlInitUnicodeString);
	ImpSet(RtlPcToFileHeader);

	ImpSet(ZwCreateFile);
	ImpSet(ZwQueryInformationFile);
	ImpSet(ZwReadFile);
	ImpSet(ZwClose);
	ImpSet(ZwOpenFile);
	ImpSet(ZwCreateSection);
	ImpSet(ZwMapViewOfSection);
	ImpSet(ZwQuerySystemInformation);
	ImpSet(ZwQueryVirtualMemory);


	PVOID BaseAddr = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	PVOID JmpRdiAddress = FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("FF 27"));
	if (JmpRdiAddress == nullptr)
	{
		ImpCall(DbgPrintEx, DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("Not Found JmpRdiAddress\n"));
		return STATUS_NOT_SUPPORTED;
	}

	//ImpCall(DbgPrintEx, DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "跳板地址:0x%p\n", JmpRdiAddress);

	SetFuckStackStub(JmpRdiAddress);  //第一时间设置这个跳板

	BaseAddr = DumpDriverToMemory(pDriverObject);
	if (BaseAddr == NULL) {
		//KdPrint(("DumpDriverToMemory Failed..\n", BaseAddr));
		return status;
	}
	//KdPrint(("DumpDriverToMemory = %p\n", BaseAddr));

	BaseAddr = MapDriver(BaseAddr, v_target_entry->DllBase);

	if (BaseAddr == NULL)
	{
		ImpCall(DbgPrintEx, DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("MapDriver Failed BaseAddr = %p\n"), BaseAddr);
		return status;
	}

	//ImpCall(DbgPrintEx, DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "EntryPoint = %p\n", BaseAddr);


	//以此为标记，如果length为0，代表此驱动已被加载过
	pRegistryString->Length = 0;


	ULONG64 Encode_Addr = ((ULONG64)v_target_entry->DllBase & 0xFFFFFFFFFFF) / 0x1000 * 0x100000000;
	memcpy(PVOID((ULONG64)pRegistryString + 2 * sizeof(unsigned short)), &Encode_Addr, 8);

	MyEntry = (MyDriverEntry)BaseAddr;

	//ImpCall(DbgPrintEx, DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "调用 MyEntry = %p\n", MyEntry);

	status = MyEntry(pDriverObject, pRegistryString);

	return status;
}

HANDLE KLoadLibrary(const wchar_t* full_dll_path)
{
	HANDLE hSection, hFile;
	UNICODE_STRING dllName;
	PVOID BaseAddress = NULL;
	SIZE_T size = 0;
	NTSTATUS stat;
	OBJECT_ATTRIBUTES oa = { sizeof(oa), 0, &dllName, OBJ_CASE_INSENSITIVE };
	IO_STATUS_BLOCK iosb;


	ImpCall(RtlInitUnicodeString, &dllName, full_dll_path);

	//_asm int 3;
	stat = ImpCall(ZwOpenFile, &hFile, FILE_EXECUTE | SYNCHRONIZE, &oa, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);

	if (!NT_SUCCESS(stat)) {
		return 0;
	}

	oa.ObjectName = 0;

	stat = ImpCall(ZwCreateSection, &hSection, SECTION_ALL_ACCESS, &oa, 0, PAGE_EXECUTE, 0x1000000, hFile);

	if (!NT_SUCCESS(stat)) {
		return 0;
	}

	stat = ImpCall(ZwMapViewOfSection, hSection, NtCurrentProcess(), &BaseAddress, 0, 1000, 0,
		&size, (SECTION_INHERIT)1, MEM_TOP_DOWN, PAGE_READWRITE);


	if (!NT_SUCCESS(stat)) {
		return 0;
	}


	ImpCall(ZwClose, hSection);
	ImpCall(ZwClose, hFile);

	return BaseAddress;
}

PVOID DumpDriverToMemory(PDRIVER_OBJECT pDriverObject)
{
	INT64 BaseAddr = (INT64)((PLDR_DATA_TABLE_ENTRY64)(pDriverObject->DriverSection))->DllBase;

	PVOID pNewFileBuffer = NULL;
	INT64 NewFileBufferSize = 0;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)BaseAddr;
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((INT64)pDosHeader + (INT64)pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)((INT64)pNtHeader + sizeof(IMAGE_FILE_HEADER));
	PIMAGE_SECTION_HEADER pSectionGroup = (PIMAGE_SECTION_HEADER)((PCHAR)pNtHeader + sizeof(IMAGE_NT_HEADERS));

	//1、获取NewFileBuffer的内存大小
	NewFileBufferSize += pOptionalHeader->SizeOfHeaders;//PE头大小
	for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		NewFileBufferSize += pSectionGroup[i].SizeOfRawData;//节区大小
	}

	//2、为pNewFileBuffer分配内存空间
	pNewFileBuffer = ImpCall(ExAllocatePool, NonPagedPoolNx, NewFileBufferSize);//ImpCall(ExAllocatePool, NonPagedPool, NewFileBufferSize);
	if (pNewFileBuffer == NULL)
	{
		pNewFileBuffer = NULL;
		NewFileBufferSize = 0;
		//ImpCall(KeAttachProcess, NULL);
		//DbgPrint("Too Long CanNot Allocate 0x%lX\r\n", NewFileBufferSize);
		return 0;
	}
	MemZero(pNewFileBuffer, NewFileBufferSize);

	//3、将ImageBuffer的数据拷贝到NewFileBuffer中
	//		文件头直接拷贝
	MemCpy(pNewFileBuffer, (PVOID)BaseAddr, pOptionalHeader->SizeOfHeaders);

	//		节区循环拷贝
	for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		MemCpy((PVOID)((INT64)pNewFileBuffer + pSectionGroup[i].PointerToRawData),
			(PVOID)((INT64)BaseAddr + pSectionGroup[i].VirtualAddress),
			pSectionGroup[i].SizeOfRawData);
	}
	return pNewFileBuffer;
}

PVOID LoadFileToMemory(PWCHAR FullPath, PSIZE_T psBufferLength)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	UNICODE_STRING UstrDllPath = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK ioStatusBlock = { 0 };
	HANDLE hFile = NULL;
	PVOID pBuffer = NULL;
	FILE_STANDARD_INFORMATION fsi = { 0 };


	ImpCall(RtlInitUnicodeString, &UstrDllPath, FullPath);

	//参数校验
	if (psBufferLength == NULL)
	{
		//KdPrint(("%s %d: Parameter error\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//打开文件
	InitializeObjectAttributes(&objectAttributes, &UstrDllPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
	ntStatus = ImpCall(ZwCreateFile,
		&hFile,
		GENERIC_READ,
		&objectAttributes,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		//KdPrint(("%s %d: ZwCreateFile failed 0x%x\n", __FUNCTION__, __LINE__, ntStatus));
		goto End;
	}

	//获取文件大小*psBufferLength
	ntStatus = ImpCall(ZwQueryInformationFile,
		hFile,
		&ioStatusBlock,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation
	);
	if (!NT_SUCCESS(ntStatus))
	{
		//KdPrint(("%s %d: ZwQueryInformationFile failed 0x%x\n", __FUNCTION__, __LINE__, ntStatus));
		goto End;
	}
	*psBufferLength = (SIZE_T)fsi.EndOfFile.QuadPart;

	//分配内存

	pBuffer = ImpCall(ExAllocatePool, NonPagedPool, *psBufferLength);//ImpCall(ExAllocatePool, NonPagedPool, *psBufferLength);
	if (pBuffer == NULL)
	{
		//KdPrint(("%s %d: ExAllocatePool failed\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//将文件读入内存
	ntStatus = ImpCall(ZwReadFile,
		hFile,
		NULL,
		NULL,
		NULL,
		&ioStatusBlock,
		pBuffer,
		(ULONG)*psBufferLength,
		NULL,
		NULL
	);
	if (!NT_SUCCESS(ntStatus))
	{
		//KdPrint(("%s %d: ZwReadFile failed 0x%x\n", __FUNCTION__, __LINE__, ntStatus));
		ImpCall(ExFreePoolWithTag, pBuffer, 'skvp');
		goto End;
	}

End:
	//关闭文件句柄
	if (hFile != NULL)
	{
		ImpCall(ZwClose, hFile);
	}

	return pBuffer;
}

PVOID MapDriver(PVOID pBuffer, PVOID KernelBase)
{
	PVOID				pMemory = NULL;

	SIZE_T				tSize = 0;
	NTSTATUS			status = 0;

	// 	size_t OffsetNtHeader;
	// 	IMAGE_NT_HEADERS ntHd;
	// 
	// 	if (!CheckHeader(pBuffer, OffsetNtHeader, ntHd))
	// 		return NULL;

		//size_t BinaryImageSize = ntHd.OptionalHeader.SizeOfImage;
		//bool IsRelocatable = (ntHd.OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE);
		//LOG_DEBUG("文件大小:%d IsRelocatable:%d\r\n", BinaryImageSize, IsRelocatable);

		//------------------------------------------------

	g_pDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
	g_pNTHeader = (PIMAGE_NT_HEADERS64)((INT64)pBuffer + g_pDosHeader->e_lfanew); // PE头


	tSize = g_pNTHeader->OptionalHeader.SizeOfImage;


	//===============================
	//===============================
	//分配非分页内存
	//===============================
	//===============================

	//pMemory = KLoadLibrary(L"\\DosDevices\\C:\\Users\\Administrator\\Desktop\\JackRead.sys");

	//尝试性读内存，让他加载到内存上
	/*__try {
		RtlCopyMemory(pBuffer, pMemory, 0x5000);
	}
	__except(1){
	}*/

	// 这里有些系统会出现偶尔分配不到的情况 ，这里我们给他2次机会
	pMemory = GetMmAllocateIndependentPages(tSize);//ImpCall(ExAllocatePoolWithTag, NonPagedPool, tSize, 'skvp');//ImpCall(ExAllocatePool, NonPagedPool, tSize);//utils::GetMmAllocateIndependentPages(tSize); //ExAllocatePool(NonPagedPool, tSize);
	if (pMemory == NULL)
	{
		pMemory = GetMmAllocateIndependentPages(tSize);
	}
	// 那就挂掉吧
	if (pMemory == NULL) {
		//KdPrint(("AllocNonPagedPool failed..\n"));
		return NULL;
	}
	// 自映射初始化
	InitializePteBase(__readcr3());
	//ImpCall(DbgPrint, "===================== \r\n");
	//ImpCall(DbgPrint, "g_pxe_base: 0x%llX \r\n", g_pxe_base);
	//ImpCall(DbgPrint, "g_ppe_base: 0x%llX \r\n", g_ppe_base);
	//ImpCall(DbgPrint, "g_pde_base: 0x%llX \r\n", g_pde_base);
	//ImpCall(DbgPrint, "g_pte_base: 0x%llX \r\n", g_pte_base);
	//ImpCall(DbgPrint, "===================== \r\n");

	// 这里我们给可执行权限
	SetMemoryExecute((ULONG64)pMemory, tSize);

	//LOG_DEBUG("DriverWorking: 0x%llX size: 0x%llX\r\n", pMemory, tSize);

	//LOG_DEBUG("pBuffer %p , pMemory %p ,size = %llx\n", pBuffer, pMemory, tSize);

	//===============================
	//===============================
	//映射节区
	//===============================
	//===============================
	INT64 imgBase = 0;

	imgBase = (ULONG64)pMemory;

	g_pSectionHeader = (PIMAGE_SECTION_HEADER)((PCHAR)g_pNTHeader + sizeof(IMAGE_NT_HEADERS64));

	INT nMoveSize = g_pNTHeader->OptionalHeader.SizeOfHeaders +
		g_pNTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

	PVOID   pSectionAddress = NULL;
	//复制头和段信息
	MemCpy((PVOID)imgBase, pBuffer, nMoveSize);

	//KdPrint(("SectionName\t\tROV\t\tAddress\n"));
	for (int i = 0; i < g_pNTHeader->FileHeader.NumberOfSections; i++)
	{
		if (g_pSectionHeader[i].VirtualAddress == 0 || g_pSectionHeader[i].SizeOfRawData == 0)
		{
			continue;
		}
		//复制每个节
		// 定位该节在内存中的位置
		pSectionAddress = (PVOID)(imgBase + g_pSectionHeader[i].VirtualAddress);
		// 复制段数据到虚拟内存
		MemCpy(pSectionAddress, ((PCHAR)pBuffer +
			g_pSectionHeader[i].PointerToRawData), g_pSectionHeader[i].SizeOfRawData);


		//KdPrint(("%s\t\t%x\t\t%llx\n", g_pSectionHeader[i].Name, g_pSectionHeader[i].VirtualAddress,pSectionAddress));

	}

	/*
		用完之后把之前的内存释放掉
	*/
	ImpCall(ExFreePoolWithTag, pBuffer, 'skvp');


	//修正指针，指向新分配的内存
	//新的dos头
	g_pDosHeader = (PIMAGE_DOS_HEADER)imgBase;
	//新的pe头地址
	g_pNTHeader = (PIMAGE_NT_HEADERS)((PCHAR)imgBase + (g_pDosHeader->e_lfanew));
	//新的节表地址
	g_pSectionHeader = (PIMAGE_SECTION_HEADER)((PCHAR)g_pNTHeader + sizeof(IMAGE_NT_HEADERS64));


	//===============================
	//===============================
	//地址重定位
	//===============================
	//===============================

	INT64 offset = 0;
	INT64 relAddr = 0;
	LONG relNum = 0;
	LONG relType = 0;
	PINT64 tempPoint = 0;
	PUSHORT pReloc = NULL;
	PIMAGE_BASE_RELOCATION pRelocation = NULL;

	offset = imgBase - g_pNTHeader->OptionalHeader.ImageBase;

	PIMAGE_DATA_DIRECTORY pDirectory = (PIMAGE_DATA_DIRECTORY)&g_pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];


	//KdPrint(("\n\nOptionalHeader.ImageBase = %llx , RelocationOffset = %llx\n", imgBase - offset, offset));
	//KdPrint(("%-20s%-20s%s\n", "Point", "PointVal", "ReapirVal"));
	if (pDirectory->Size)
	{
		pRelocation = (PIMAGE_BASE_RELOCATION)(imgBase + pDirectory->VirtualAddress);

		while (pRelocation->SizeOfBlock)
		{
			relAddr = imgBase + pRelocation->VirtualAddress;

			//计算在当前块中的数据个数
			relNum = (pRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;

			//指向数据块
			pReloc = (PUSHORT)(PIMAGE_RELOC)((INT64)pRelocation + sizeof(IMAGE_BASE_RELOCATION));

			while (relNum--)
			{
				relType = *((PUSHORT)pReloc) >> 12;
				if (relType == IMAGE_REL_BASED_DIR64)
				{
					tempPoint = (PINT64)(relAddr + (*pReloc & 0x0fff));
					//KdPrint(("%-20llx%-20llx%llx\n", tempPoint, *tempPoint, *tempPoint + offset));
					*tempPoint += offset;
				}
				else if (relType == IMAGE_REL_BASED_HIGHLOW)
				{
					//LOG_DEBUG("IMAGE_REL_BASED_HIGHLOW\r\n");
				}
				else if (relType == IMAGE_REL_BASED_HIGH)
				{
					//LOG_DEBUG("IMAGE_REL_BASED_HIGH\r\n");
				}
				else if (relType == IMAGE_REL_BASED_LOW)
				{
					//LOG_DEBUG("IMAGE_REL_BASED_LOW\r\n");
				}
				else if (relType == IMAGE_REL_BASED_ABSOLUTE)
				{
					//LOG_DEBUG("IMAGE_REL_BASED_ABSOLUTE\r\n");
				}
				pReloc++;
			}
			pRelocation = (PIMAGE_BASE_RELOCATION)((INT64)pRelocation + pRelocation->SizeOfBlock);
		}
	}


	//===============================
	//===============================
	//修复导入表 没有导入表 自然不需要修复
	//===============================
	//===============================
	INT64 libAddr = 0;
	INT64 funcAddr = 0;
	PIMAGE_IMPORT_DESCRIPTOR	  pImportDes = NULL;
	PIMAGE_THUNK_DATA64           pRealIAT = NULL;
	PIMAGE_THUNK_DATA64           pFuncIAT = NULL;
	PIMAGE_THUNK_DATA64           pOriginalIAT = NULL;
	INT64 temp1 = (INT64) & (((PIMAGE_NT_HEADERS)((PCHAR)imgBase + (g_pDosHeader->e_lfanew)))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);

	//第一个导入表节点
	pImportDes = (PIMAGE_IMPORT_DESCRIPTOR)(imgBase + ((PIMAGE_DATA_DIRECTORY)temp1)->VirtualAddress);
	PCHAR pName = NULL;

	//LOG_DEBUG("\n\n%-40s%-35s%s\n", "ModuleName", "FuncName", "ReapirAddr");

	while (pImportDes->Name)
	{
		pName = (PCHAR)(imgBase + pImportDes->Name);

		if (!ImpCall(MmIsAddressValid, pName))
		{
			//LOG_DEBUG("Repail ImportTable Failed\n");
			return 0;
		}

		//获取驱动模块基址
		libAddr = (INT64)KernelBase;
		if (libAddr == 0)
		{
			//LOG_DEBUG("GetKernelModule %s Failed\n", pName);
			return 0;
		}

		pRealIAT = (PIMAGE_THUNK_DATA64)(imgBase + pImportDes->OriginalFirstThunk);
		pFuncIAT = (PIMAGE_THUNK_DATA64)(imgBase + pImportDes->FirstThunk);
		pOriginalIAT = (PIMAGE_THUNK_DATA64)(imgBase + pImportDes->FirstThunk);
		while (pRealIAT->u1.ForwarderString)
		{
			//得到PIMAGE_IMPORT_BY_NAME
			temp1 = imgBase + pRealIAT->u1.AddressOfData;
			//取得函数的地址
			//r3使用GetProcAddress可以直接获取函数地址
			//这里我们需要枚举导出表取得函数地址
			funcAddr = GetFuncAddrFromExportTable((PVOID)libAddr, ((PIMAGE_IMPORT_BY_NAME)temp1)->Name);
			if (funcAddr)
			{
				pFuncIAT->u1.Function = funcAddr;
				//LOG_DEBUG("%-40s%-35s%llx\n", pName, ((PIMAGE_IMPORT_BY_NAME)temp1)->Name, pFuncIAT->u1.Function);
			}
			else
			{
				//LOG_DEBUG("查找%s导入函数失败\r\n", ((PIMAGE_IMPORT_BY_NAME)temp1)->Name);
			}
			//
			//到下一个节点
			pRealIAT++;
			pFuncIAT++;
		}
		//到下一个导入表
		pImportDes++;
	}
	//======================================

	INT64 EntryPoint = imgBase + g_pNTHeader->OptionalHeader.AddressOfEntryPoint;

	//LOG_DEBUG("AddressOfEntryPoint %p\n", EntryPoint);

	//返回入口点 call一下
	return (PVOID)EntryPoint;
}

ULONG GetIndexFromExportTable(PVOID pBaseAddress, PCCHAR pszFunctionName)
{
	ULONG ulFunctionIndex = 0;
	// Dos Header
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
	// NT Header
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	// Export Table
	PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
	// 有名称的导出函数个数
	ULONG ulNumberOfNames = pExportTable->NumberOfNames;
	// 导出函数名称地址表
	PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpName = NULL;
	// 开始遍历导出表
	for (ULONG i = 0; i < ulNumberOfNames; i++)
	{
		lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
		// 判断是否查找的函数
		if (StrICmp(pszFunctionName, lpName, true))
		{
			// 获取导出函数地址
			USHORT uHint = *(USHORT*)((PUCHAR)pDosHeader + pExportTable->AddressOfNameOrdinals + 2 * i);
			ULONG ulFuncAddr = *(PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfFunctions + 4 * uHint);
			PVOID lpFuncAddr = (PVOID)((PUCHAR)pDosHeader + ulFuncAddr);
			// 获取 SSDT 函数 Index
			ulFunctionIndex = *(ULONG*)((PUCHAR)lpFuncAddr + 4);
			break;
		}
	}

	return ulFunctionIndex;
}

INT64 GetFuncAddrFromExportTable(PVOID pBaseAddress, PCCHAR pszFunctionName)
{
	ULONG ulFunctionIndex = 0;
	// Dos Header
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
	// NT Header
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	// Export Table
	PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
	// 有名称的导出函数个数
	ULONG ulNumberOfNames = pExportTable->NumberOfNames;
	// 导出函数名称地址表
	PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpName = NULL;
	// 开始遍历导出表
	for (ULONG i = 0; i < ulNumberOfNames; i++)
	{
		lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
		// 判断是否查找的函数
		//if (0 == _strnicmp(pszFunctionName, lpName, strlen(pszFunctionName)))
		if (StrICmp(pszFunctionName, lpName, true))
		{
			// 获取导出函数地址
			USHORT uHint = *(USHORT*)((PUCHAR)pDosHeader + pExportTable->AddressOfNameOrdinals + 2 * i);
			ULONG ulFuncAddr = *(PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfFunctions + 4 * uHint);
			PVOID lpFuncAddr = (PVOID)((PUCHAR)pDosHeader + ulFuncAddr);

			return (INT64)lpFuncAddr;
		}
	}

	return 0;
}

BOOLEAN xstricmp(LPCSTR s1, LPCSTR s2)
{
	ULONG i = 0;

	for (i = 0; 0 == ((s1[i] ^ s2[i]) & 0xDF); ++i)
	{
		if (0 == s1[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN InitFuncAddr(PDRIVER_OBJECT pDriverOjb)
{
	LdrEntryPoint = (PLDR_DATA_TABLE_ENTRY64)((PLDR_DATA_TABLE_ENTRY64)pDriverOjb->DriverSection)->InLoadOrderLinks.Flink;
	return TRUE;
}
