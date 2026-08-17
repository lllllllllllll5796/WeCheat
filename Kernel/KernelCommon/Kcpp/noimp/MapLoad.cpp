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

	
	PVOID BaseAddr = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	PVOID JmpRdiAddress = FindPatternSect(g_KernelBase, oxorany(".text"), oxorany("FF 27"));
	if (JmpRdiAddress == nullptr)
	{
		DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("Not Found JmpRdiAddress\n"));
		return STATUS_NOT_SUPPORTED;
	}

	//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "�����ַ:0x%p\n", JmpRdiAddress);

	SetFuckStackStub(JmpRdiAddress);  //��һʱ�������������

	BaseAddr = DumpDriverToMemory(pDriverObject);
	if (BaseAddr == NULL) {
		//KdPrint(("DumpDriverToMemory Failed..\n", BaseAddr));
		return status;
	}
	//KdPrint(("DumpDriverToMemory = %p\n", BaseAddr));

	BaseAddr = MapDriver(BaseAddr, v_target_entry->DllBase);

	if (BaseAddr == NULL)
	{
		DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, oxorany("MapDriver Failed BaseAddr = %p\n"), BaseAddr);
		return status;
	}

	//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "EntryPoint = %p\n", BaseAddr);


	//�Դ�Ϊ��ǣ����lengthΪ0�������������ѱ����ع�
	pRegistryString->Length = 0;


	ULONG64 Encode_Addr = ((ULONG64)v_target_entry->DllBase & 0xFFFFFFFFFFF) / 0x1000 * 0x100000000;
	memcpy(PVOID((ULONG64)pRegistryString + 2 * sizeof(unsigned short)), &Encode_Addr, 8);

	MyEntry = (MyDriverEntry)BaseAddr;

	//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "���� MyEntry = %p\n", MyEntry);

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


	RtlInitUnicodeString(&dllName, full_dll_path);

	//_asm int 3;
	stat = ZwOpenFile(&hFile, FILE_EXECUTE | SYNCHRONIZE, &oa, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);

	if (!NT_SUCCESS(stat)) {
		return 0;
	}

	oa.ObjectName = 0;

	stat = ZwCreateSection(&hSection, SECTION_ALL_ACCESS, &oa, 0, PAGE_EXECUTE, 0x1000000, hFile);

	if (!NT_SUCCESS(stat)) {
		return 0;
	}

	stat = ZwMapViewOfSection(hSection, NtCurrentProcess(), &BaseAddress, 0, 1000, 0,
		&size, (SECTION_INHERIT)1, MEM_TOP_DOWN, PAGE_READWRITE);


	if (!NT_SUCCESS(stat)) {
		return 0;
	}


	ZwClose(hSection);
	ZwClose(hFile);

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

	//1����ȡNewFileBuffer���ڴ��С
	NewFileBufferSize += pOptionalHeader->SizeOfHeaders;//PEͷ��С
	for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		NewFileBufferSize += pSectionGroup[i].SizeOfRawData;//������С
	}

	//2��ΪpNewFileBuffer�����ڴ�ռ�
	pNewFileBuffer = ExAllocatePool(NonPagedPoolNx, NewFileBufferSize);//ExAllocatePool(NonPagedPool, NewFileBufferSize);
	if (pNewFileBuffer == NULL)
	{
		pNewFileBuffer = NULL;
		NewFileBufferSize = 0;
		//KeAttachProcess(NULL);
		//DbgPrint("Too Long CanNot Allocate 0x%lX\r\n", NewFileBufferSize);
		return 0;
	}
	MemZero(pNewFileBuffer, NewFileBufferSize);

	//3����ImageBuffer�����ݿ�����NewFileBuffer��
	//		�ļ�ͷֱ�ӿ���
	MemCpy(pNewFileBuffer, (PVOID)BaseAddr, pOptionalHeader->SizeOfHeaders);

	//		����ѭ������
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


	RtlInitUnicodeString(&UstrDllPath, FullPath);

	//����У��
	if (psBufferLength == NULL)
	{
		//KdPrint(("%s %d: Parameter error\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//���ļ�
	InitializeObjectAttributes(&objectAttributes, &UstrDllPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
	ntStatus = ZwCreateFile(&hFile,
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

	//��ȡ�ļ���С*psBufferLength
	ntStatus = ZwQueryInformationFile(hFile,
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

	//�����ڴ�

	pBuffer = ExAllocatePool(NonPagedPool, *psBufferLength);//ExAllocatePool(NonPagedPool, *psBufferLength);
	if (pBuffer == NULL)
	{
		//KdPrint(("%s %d: ExAllocatePool failed\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//���ļ������ڴ�
	ntStatus = ZwReadFile(hFile,
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
		ExFreePoolWithTag(pBuffer, 'skvp');
		goto End;
	}

End:
	//�ر��ļ����
	if (hFile != NULL)
	{
		ZwClose(hFile);
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
		//LOG_DEBUG("�ļ���С:%d IsRelocatable:%d\r\n", BinaryImageSize, IsRelocatable);

		//------------------------------------------------

	g_pDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
	g_pNTHeader = (PIMAGE_NT_HEADERS64)((INT64)pBuffer + g_pDosHeader->e_lfanew); // PEͷ


	tSize = g_pNTHeader->OptionalHeader.SizeOfImage;


	//===============================
	//===============================
	//����Ƿ�ҳ�ڴ�
	//===============================
	//===============================

	//pMemory = KLoadLibrary(L"\\DosDevices\\C:\\Users\\Administrator\\Desktop\\JackRead.sys");

	//�����Զ��ڴ棬�������ص��ڴ���
	/*__try {
		RtlCopyMemory(pBuffer, pMemory, 0x5000);
	}
	__except(1){
	}*/

	// ������Щϵͳ�����ż�����䲻������� ���������Ǹ���2�λ���
	pMemory = GetMmAllocateIndependentPages(tSize);//ExAllocatePoolWithTag(NonPagedPool, tSize, 'skvp');//ExAllocatePool(NonPagedPool, tSize);//utils::GetMmAllocateIndependentPages(tSize); //ExAllocatePool(NonPagedPool, tSize);
	if (pMemory == NULL)
	{
		pMemory = GetMmAllocateIndependentPages(tSize);
	}
	// �Ǿ͹ҵ���
	if (pMemory == NULL) {
		//KdPrint(("AllocNonPagedPool failed..\n"));
		return NULL;
	}
	// ��ӳ���ʼ��
	InitializePteBase(__readcr3());
	//DbgPrint("===================== \r\n");
	//DbgPrint("g_pxe_base: 0x%llX \r\n", g_pxe_base);
	//DbgPrint("g_ppe_base: 0x%llX \r\n", g_ppe_base);
	//DbgPrint("g_pde_base: 0x%llX \r\n", g_pde_base);
	//DbgPrint("g_pte_base: 0x%llX \r\n", g_pte_base);
	//DbgPrint("===================== \r\n");

	// �������Ǹ���ִ��Ȩ��
	SetMemoryExecute((ULONG64)pMemory, tSize);

	//LOG_DEBUG("DriverWorking: 0x%llX size: 0x%llX\r\n", pMemory, tSize);

	//LOG_DEBUG("pBuffer %p , pMemory %p ,size = %llx\n", pBuffer, pMemory, tSize);

	//===============================
	//===============================
	//ӳ�����
	//===============================
	//===============================
	INT64 imgBase = 0;

	imgBase = (ULONG64)pMemory;

	g_pSectionHeader = (PIMAGE_SECTION_HEADER)((PCHAR)g_pNTHeader + sizeof(IMAGE_NT_HEADERS64));

	INT nMoveSize = g_pNTHeader->OptionalHeader.SizeOfHeaders +
		g_pNTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

	PVOID   pSectionAddress = NULL;
	//����ͷ�Ͷ���Ϣ
	MemCpy((PVOID)imgBase, pBuffer, nMoveSize);

	//KdPrint(("SectionName\t\tROV\t\tAddress\n"));
	for (int i = 0; i < g_pNTHeader->FileHeader.NumberOfSections; i++)
	{
		if (g_pSectionHeader[i].VirtualAddress == 0 || g_pSectionHeader[i].SizeOfRawData == 0)
		{
			continue;
		}
		//����ÿ����
		// ��λ�ý����ڴ��е�λ��
		pSectionAddress = (PVOID)(imgBase + g_pSectionHeader[i].VirtualAddress);
		// ���ƶ����ݵ������ڴ�
		MemCpy(pSectionAddress, ((PCHAR)pBuffer +
			g_pSectionHeader[i].PointerToRawData), g_pSectionHeader[i].SizeOfRawData);


		//KdPrint(("%s\t\t%x\t\t%llx\n", g_pSectionHeader[i].Name, g_pSectionHeader[i].VirtualAddress,pSectionAddress));

	}

	/*
		����֮���֮ǰ���ڴ��ͷŵ�
	*/
	ExFreePoolWithTag(pBuffer, 'skvp');


	//����ָ�룬ָ���·�����ڴ�
	//�µ�dosͷ
	g_pDosHeader = (PIMAGE_DOS_HEADER)imgBase;
	//�µ�peͷ��ַ
	g_pNTHeader = (PIMAGE_NT_HEADERS)((PCHAR)imgBase + (g_pDosHeader->e_lfanew));
	//�µĽڱ���ַ
	g_pSectionHeader = (PIMAGE_SECTION_HEADER)((PCHAR)g_pNTHeader + sizeof(IMAGE_NT_HEADERS64));


	//===============================
	//===============================
	//��ַ�ض�λ
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

			//�����ڵ�ǰ���е����ݸ���
			relNum = (pRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;

			//ָ�����ݿ�
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
	//�޸������ û�е���� ��Ȼ����Ҫ�޸�
	//===============================
	//===============================
	INT64 libAddr = 0;
	INT64 funcAddr = 0;
	PIMAGE_IMPORT_DESCRIPTOR	  pImportDes = NULL;
	PIMAGE_THUNK_DATA64           pRealIAT = NULL;
	PIMAGE_THUNK_DATA64           pFuncIAT = NULL;
	PIMAGE_THUNK_DATA64           pOriginalIAT = NULL;
	INT64 temp1 = (INT64) & (((PIMAGE_NT_HEADERS)((PCHAR)imgBase + (g_pDosHeader->e_lfanew)))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);

	//��һ��������ڵ�
	pImportDes = (PIMAGE_IMPORT_DESCRIPTOR)(imgBase + ((PIMAGE_DATA_DIRECTORY)temp1)->VirtualAddress);
	PCHAR pName = NULL;

	//LOG_DEBUG("\n\n%-40s%-35s%s\n", "ModuleName", "FuncName", "ReapirAddr");

	while (pImportDes->Name)
	{
		pName = (PCHAR)(imgBase + pImportDes->Name);

		if (!MmIsAddressValid(pName))
		{
			//LOG_DEBUG("Repail ImportTable Failed\n");
			return 0;
		}

		//��ȡ����ģ���ַ
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
			//�õ�PIMAGE_IMPORT_BY_NAME
			temp1 = imgBase + pRealIAT->u1.AddressOfData;
			//ȡ�ú����ĵ�ַ
			//r3ʹ��GetProcAddress����ֱ�ӻ�ȡ������ַ
			//����������Ҫö�ٵ�����ȡ�ú�����ַ
			funcAddr = GetFuncAddrFromExportTable((PVOID)libAddr, ((PIMAGE_IMPORT_BY_NAME)temp1)->Name);
			if (funcAddr)
			{
				pFuncIAT->u1.Function = funcAddr;
				//LOG_DEBUG("%-40s%-35s%llx\n", pName, ((PIMAGE_IMPORT_BY_NAME)temp1)->Name, pFuncIAT->u1.Function);
			}
			else
			{
				//LOG_DEBUG("����%s���뺯��ʧ��\r\n", ((PIMAGE_IMPORT_BY_NAME)temp1)->Name);
			}
			//
			//����һ���ڵ�
			pRealIAT++;
			pFuncIAT++;
		}
		//����һ�������
		pImportDes++;
	}
	//======================================

	INT64 EntryPoint = imgBase + g_pNTHeader->OptionalHeader.AddressOfEntryPoint;

	//LOG_DEBUG("AddressOfEntryPoint %p\n", EntryPoint);

	//������ڵ� callһ��
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
	// �����Ƶĵ�����������
	ULONG ulNumberOfNames = pExportTable->NumberOfNames;
	// �����������Ƶ�ַ��
	PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpName = NULL;
	// ��ʼ����������
	for (ULONG i = 0; i < ulNumberOfNames; i++)
	{
		lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
		// �ж��Ƿ���ҵĺ���
		if (StrICmp(pszFunctionName, lpName, true))
		{
			// ��ȡ����������ַ
			USHORT uHint = *(USHORT*)((PUCHAR)pDosHeader + pExportTable->AddressOfNameOrdinals + 2 * i);
			ULONG ulFuncAddr = *(PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfFunctions + 4 * uHint);
			PVOID lpFuncAddr = (PVOID)((PUCHAR)pDosHeader + ulFuncAddr);
			// ��ȡ SSDT ���� Index
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
	// �����Ƶĵ�����������
	ULONG ulNumberOfNames = pExportTable->NumberOfNames;
	// �����������Ƶ�ַ��
	PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpName = NULL;
	// ��ʼ����������
	for (ULONG i = 0; i < ulNumberOfNames; i++)
	{
		lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
		// �ж��Ƿ���ҵĺ���
		//if (0 == _strnicmp(pszFunctionName, lpName, strlen(pszFunctionName)))
		if (StrICmp(pszFunctionName, lpName, true))
		{
			// ��ȡ����������ַ
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
