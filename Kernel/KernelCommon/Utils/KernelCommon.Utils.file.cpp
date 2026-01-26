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
			ImpCall(RtlInitUnicodeString, &uniFileName, szFileName);
			InitializeObjectAttributes(&oba,
				&uniFileName,
				OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
				0,
				0);
			ns = ImpCall(IoCreateFile, &hListFile,
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
				LOG_DEBUG("IoCreateFile ß∞‹:%X\r\n", ns);
				return NULL;
			}
			ns = ImpCall(ZwQueryInformationFile, hListFile,
				&iosb,
				&filestandinfo,
				sizeof(FILE_STANDARD_INFORMATION),
				FileStandardInformation
			);

			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG("ZwQueryInformationFile ß∞‹%X\r\n", ns);
				ImpCall(ZwClose, hListFile);
				return NULL;
			}

			//get file len 


			dwSize = (ULONG)filestandinfo.AllocationSize.QuadPart;

			FilePool = ImpCall(ExAllocatePool, NonPagedPool,
				dwSize);

			if (!FilePool)
			{
				ImpCall(ZwClose, hListFile);
				return NULL;
			}

			//allocate pool for read file

			ns = ImpCall(ZwReadFile, hListFile,
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
				ImpCall(ExFreePool, FilePool);
				ImpCall(ZwClose, hListFile);
				return NULL;
			}
			//read file
			ImpCall(ZwClose, hListFile);
			return FilePool;
		}
	}
}