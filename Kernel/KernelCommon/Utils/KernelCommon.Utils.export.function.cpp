#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.export.function.h"

namespace KernelCommon
{
	namespace Utils
	{
		//---------------3��----------------
		PVOID GetExportedFunctionAddress(PEPROCESS TargetProcess, PVOID ModuleBase, CONST CHAR* ExportedFunctionName)
		{
			::KAPC_STATE State;
			PVOID FunctionAddress = 0;
			if (TargetProcess != NULL)
				KeStackAttachProcess((PRKPROCESS)TargetProcess, &State);

			do
			{
				PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ModuleBase;
				PIMAGE_NT_HEADERS64 NtHeader = (PIMAGE_NT_HEADERS64)(DosHeader->e_lfanew + (ULONG64)ModuleBase);
				IMAGE_DATA_DIRECTORY ImageDataDirectory = NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

				if (ImageDataDirectory.Size == 0 || ImageDataDirectory.VirtualAddress == 0)
					break;

				PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG64)ModuleBase + ImageDataDirectory.VirtualAddress);
				ULONG* Address = (ULONG*)((ULONG64)ModuleBase + ExportDirectory->AddressOfFunctions);
				ULONG* Name = (ULONG*)((ULONG64)ModuleBase + ExportDirectory->AddressOfNames);
				USHORT* Ordinal = (USHORT*)((ULONG64)ModuleBase + ExportDirectory->AddressOfNameOrdinals);

				STRING TargetExportedFunctionName;
				RtlInitString(&TargetExportedFunctionName, ExportedFunctionName);

				for (size_t i = 0; ExportDirectory->NumberOfFunctions; i++)
				{
					STRING CurrentExportedFunctionName;
					RtlInitString(&CurrentExportedFunctionName, (PCHAR)ModuleBase + Name[i]);

					if (RtlCompareString(&TargetExportedFunctionName, &CurrentExportedFunctionName, TRUE) == 0)
					{
						FunctionAddress = (PVOID)((ULONG64)ModuleBase + Address[Ordinal[i]]);
						break;
					}
				}

			} while (0);

			if (TargetProcess != NULL)
				KeUnstackDetachProcess(&State);

			return FunctionAddress;
		}

		ULONG_PTR GetProcAddressFromPEExport(PVOID Image, const eastl::string& FunctionName)
		{
			//__try
			{
				auto RVATOVA = [](auto _base_, auto _offset_)
				{
					return ((PUCHAR)(_base_)+(ULONG)(_offset_));
				};

				PIMAGE_EXPORT_DIRECTORY pExport = NULL;

				PIMAGE_NT_HEADERS pHeaders = (PIMAGE_NT_HEADERS)
					((PUCHAR)Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);

				if (pHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
				{
					//LOG_DEBUG("32λ\r\n");
					// 32-bit image
					if (pHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
					{
						pExport = (PIMAGE_EXPORT_DIRECTORY)RVATOVA(
							Image,
							pHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
						);
					}
				}
				else if (pHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
				{
					//LOG_DEBUG("64λ\r\n");
					// 64-bit image
					PIMAGE_NT_HEADERS64 pHeaders64 = (PIMAGE_NT_HEADERS64)
						((PUCHAR)Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);

					if (pHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
					{
						pExport = (PIMAGE_EXPORT_DIRECTORY)RVATOVA(
							Image,
							pHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
						);
					}
				}

				if (pExport)
				{
					//LOG_DEBUG("�ҵ���������\r\n");
					PULONG AddressOfFunctions = (PULONG)RVATOVA(Image, pExport->AddressOfFunctions);
					PSHORT AddrOfOrdinals = (PSHORT)RVATOVA(Image, pExport->AddressOfNameOrdinals);
					PULONG AddressOfNames = (PULONG)RVATOVA(Image, pExport->AddressOfNames);
					ULONG i = 0;

					for (i = 0; i < pExport->NumberOfFunctions; i++)
					{
						auto Func_Name = eastl::string((char*)RVATOVA(Image, AddressOfNames[i]));

						//LOG_DEBUG("WTF:%s\r\n", (char*)RVATOVA(Image, AddressOfNames[i]));

						if (!strcmp(Func_Name.c_str(), FunctionName.c_str()))
						{
							//LOG_DEBUG("AddrOfOrdinals:%llX   i:%d\r\n", AddrOfOrdinals, i);
							//LOG_DEBUG("AddressOfFunctions:%llX   j:%d\r\n", AddressOfFunctions, AddrOfOrdinals[i]);
							return AddressOfFunctions[AddrOfOrdinals[i]];
						}
					}
				}
			}

			//__except (EXCEPTION_EXECUTE_HANDLER) {}
			return 0;
		}


		ULONG_PTR GetProcAddressR(ULONG_PTR hModule, const char* lpProcName, BOOL x64Module)
		{
#define DEREF( name )*(UINT_PTR *)(name)
#define DEREF_64( name )*(unsigned __int64 *)(name)
#define DEREF_32( name )*(unsigned long *)(name)
#define DEREF_16( name )*(unsigned short *)(name)
#define DEREF_8( name )*(UCHAR *)(name)
			UINT_PTR uiLibraryAddress = 0;
			ULONG_PTR fpResult = NULL;

			if (hModule == NULL)
				return NULL;

			// a module handle is really its base address
			uiLibraryAddress = (UINT_PTR)hModule;

			__try
			{
				UINT_PTR uiAddressArray = 0;
				UINT_PTR uiNameArray = 0;
				UINT_PTR uiNameOrdinals = 0;
				PIMAGE_NT_HEADERS32 pNtHeaders32 = NULL;
				PIMAGE_NT_HEADERS64 pNtHeaders64 = NULL;
				PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
				PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;

				// get the VA of the modules NT Header
				pNtHeaders32 = (PIMAGE_NT_HEADERS32)(uiLibraryAddress + ((PIMAGE_DOS_HEADER)uiLibraryAddress)->e_lfanew);
				pNtHeaders64 = (PIMAGE_NT_HEADERS64)(uiLibraryAddress + ((PIMAGE_DOS_HEADER)uiLibraryAddress)->e_lfanew);
				if (x64Module)
				{
					pDataDirectory = (PIMAGE_DATA_DIRECTORY)&pNtHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
				}
				else
				{
					pDataDirectory = (PIMAGE_DATA_DIRECTORY)&pNtHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
				}


				// get the VA of the export directory
				pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(uiLibraryAddress + pDataDirectory->VirtualAddress);

				// get the VA for the array of addresses
				uiAddressArray = (uiLibraryAddress + pExportDirectory->AddressOfFunctions);

				// get the VA for the array of name pointers
				uiNameArray = (uiLibraryAddress + pExportDirectory->AddressOfNames);

				// get the VA for the array of name ordinals
				uiNameOrdinals = (uiLibraryAddress + pExportDirectory->AddressOfNameOrdinals);

				// test if we are importing by name or by ordinal...
				if ((PtrToUlong(lpProcName) & 0xFFFF0000) == 0x00000000)
				{
					// import by ordinal...

					// use the import ordinal (- export ordinal base) as an index into the array of addresses
					uiAddressArray += ((IMAGE_ORDINAL(PtrToUlong(lpProcName)) - pExportDirectory->Base) * sizeof(unsigned long));

					// resolve the address for this imported function
					fpResult = (ULONG_PTR)(uiLibraryAddress + DEREF_32(uiAddressArray));
				}
				else
				{
					// import by name...
					unsigned long dwCounter = pExportDirectory->NumberOfNames;
					while (dwCounter--)
					{
						char* cpExportedFunctionName = (char*)(uiLibraryAddress + DEREF_32(uiNameArray));

						// test if we have a match...
						if (strcmp(cpExportedFunctionName, lpProcName) == 0)
						{
							// use the functions name ordinal as an index into the array of name pointers
							uiAddressArray += (DEREF_16(uiNameOrdinals) * sizeof(unsigned long));

							// calculate the virtual address for the function
							fpResult = (ULONG_PTR)(uiLibraryAddress + DEREF_32(uiAddressArray));

							// finish...
							break;
						}

						// get the next exported function name
						uiNameArray += sizeof(unsigned long);

						// get the next exported function name ordinal
						uiNameOrdinals += sizeof(unsigned short);
					}
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				fpResult = NULL;
			}

			return fpResult;
		}

		ULONG64 GetEProcessFunctionAddress(PEPROCESS PEProcess, PWCHAR MoudleName, PCHAR Function)
		{
			LOG_DEBUG("PEProcess:0x%p", PEProcess);

			PVOID LibAddr = (PVOID)GetModuleBaseBySystemApi(PEProcess, MoudleName);

			if (!LibAddr)
			{
				LOG_DEBUG("GetProcessModules %s Failed\n", MoudleName);
				return 0;
			}

			LOG_DEBUG("Modules:0x%p\r\n", LibAddr);

			PVOID MsgBoxAddr = (PCHAR)GetFuncAddrFromExportTable(LibAddr, Function);
			
			if (!MsgBoxAddr)
			{
				LOG_DEBUG("GetFuncAddr %s Failed\n", Function);
				return 0;
			}

			return (ULONG64)MsgBoxAddr;
		}

		/*
		��ȡϵͳ���������ĵ�ַ
		*/
		ULONG_PTR GetSystemRoutineAddressByName(PWCHAR funcNameStr)
		{
			ULONG_PTR				addr = 0;
			UNICODE_STRING		funcName = { 0 };
			RtlInitUnicodeString(&funcName, funcNameStr);
			addr = (ULONG_PTR)MmGetSystemRoutineAddress(&funcName);
			return addr;
		}

		PVOID GetSystemRoutineAddress(const eastl::string& FunctionName)
		{
			ANSI_STRING asName;
			UNICODE_STRING usName;
			NTSTATUS Status;

			RtlInitAnsiString(&asName, FunctionName.c_str());
			Status = RtlAnsiStringToUnicodeString(&usName, &asName, TRUE);
			if (NT_SUCCESS(Status))
			{
				//LOG_DEBUG("%wZ\r\n", &usName);
				auto pfn = MmGetSystemRoutineAddress(&usName);

				if (pfn)
				{
					RtlFreeUnicodeString(&usName);
					return pfn;
				}

				RtlFreeUnicodeString(&usName);
			}

			return nullptr;
		}
	}
}