#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.file.h"

namespace KernelCommon
{
	namespace Utils
	{
		PVOID LoadAndReadFile(WCHAR* szFileName)
		{
			HANDLE hListFile = NULL;
			NTSTATUS ns;
			IO_STATUS_BLOCK	iosb;
			LARGE_INTEGER fileoffset;
			UNICODE_STRING uniFileName;
			OBJECT_ATTRIBUTES oba;
			FILE_STANDARD_INFORMATION filestandinfo;
			PVOID FilePool;
			DWORD dwSize = 0;
			BOOL bRet = FALSE;
			RtlInitUnicodeString(&uniFileName, szFileName);
			InitializeObjectAttributes(&oba,
				&uniFileName,
				OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
				0,
				0);
			ns = IoCreateFile(&hListFile,
				GENERIC_READ | SYNCHRONIZE,
				&oba,
				&iosb,
				0,
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ,
				FILE_OPEN,
				FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
				0,
				0,
				CreateFileTypeNone,
				0,
				IO_NO_PARAMETER_CHECKING
			);

			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG("IoCreateFileʧ��:%X\r\n", ns);
				return NULL;
			}
			ns = ZwQueryInformationFile(hListFile,
				&iosb,
				&filestandinfo,
				sizeof(FILE_STANDARD_INFORMATION),
				FileStandardInformation
			);

			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG("ZwQueryInformationFileʧ��%X\r\n", ns);
				ZwClose(hListFile);
				return NULL;
			}

			//get file len 


			dwSize = (ULONG)filestandinfo.AllocationSize.QuadPart;

			FilePool = ExAllocatePool(NonPagedPool,
				dwSize);

			if (!FilePool)
			{
				ZwClose(hListFile);
				return NULL;
			}

			//allocate pool for read file

			ns = ZwReadFile(hListFile,
				NULL,
				NULL,
				NULL,
				&iosb,
				FilePool,
				dwSize,
				NULL,
				NULL
			);

			if (!NT_SUCCESS(ns))
			{
				ExFreePool(FilePool);
				ZwClose(hListFile);
				return NULL;
			}
			//read file
			ZwClose(hListFile);
			return FilePool;
		}
	}
}