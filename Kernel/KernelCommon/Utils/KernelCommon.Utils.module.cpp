#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.module.h"

namespace KernelCommon
{
	namespace Utils
	{
		PVOID GetUserModeModule(PEPROCESS TargetProcess, CONST WCHAR* ModuleName, BOOLEAN IsWow64)
		{
			if (TargetProcess == NULL)
				return NULL;

			::KAPC_STATE State;
			PVOID Address = NULL;
			KeStackAttachProcess((PRKPROCESS)TargetProcess, &State);

			UNICODE_STRING TargetModuleName;
			RtlCreateUnicodeString(&TargetModuleName, ModuleName);

			__try
			{
				do
				{
					if (IsWow64 == TRUE)
					{
						wdk::PPEB32 Peb32 = (wdk::PPEB32)PsGetProcessWow64Process(TargetProcess);

						for (PLIST_ENTRY32 ListEntry = (PLIST_ENTRY32)((wdk::PPEB_LDR_DATA32)Peb32->Ldr)->InLoadOrderModuleList.Flink;
							ListEntry != (PLIST_ENTRY32)(&((wdk::PPEB_LDR_DATA32)(Peb32->Ldr))->InLoadOrderModuleList);
							ListEntry = (PLIST_ENTRY32)ListEntry->Flink)
						{
							wdk::PLDR_DATA_TABLE_ENTRY32 Entry = CONTAINING_RECORD(ListEntry, wdk::LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

							UNICODE_STRING CurrentModuleName;
							RtlCreateUnicodeString(&CurrentModuleName, (PWCH)Entry->BaseDllName.Buffer);

							if (RtlCompareUnicodeString(&CurrentModuleName, &TargetModuleName, TRUE) == 0)
							{
								Address = (PVOID)Entry->DllBase;
								break;
							}
						}
					}
					else
					{
						wdk::PPEB64 Peb = (wdk::PPEB64)PsGetProcessPeb(TargetProcess);
						for (PLIST_ENTRY ListEntry = (PLIST_ENTRY)((wdk::PPEB_LDR_DATA64)Peb->Ldr)->InLoadOrderModuleList.Flink;
							ListEntry != (PLIST_ENTRY)(&((wdk::PPEB_LDR_DATA64)(Peb->Ldr))->InLoadOrderModuleList);
							ListEntry = ListEntry->Flink)
						{
							wdk::PLDR_DATA_TABLE_ENTRY64 Entry = CONTAINING_RECORD(ListEntry, wdk::LDR_DATA_TABLE_ENTRY64, InLoadOrderLinks);

							UNICODE_STRING CurrentModuleName;
							RtlCreateUnicodeString(&CurrentModuleName, (PCWSTR)(Entry->BaseDllName.Buffer));

							if (RtlCompareUnicodeString(&CurrentModuleName, &TargetModuleName, TRUE) == 0)
							{
								Address = Entry->DllBase;
								break;
							}
						}
					}

				} while (0);

			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{

			}

			KeUnstackDetachProcess(&State);
			return Address;
		}

		BOOL GetSectionData(CONST CHAR* ImageName, CONST CHAR* SectionName, ULONG64& SectionSize, PVOID& SectionBaseAddress)
		{
			ULONG64 ImageSize = 0;
			PVOID ImageBase = 0;

			if (GetProcessInfo(ImageName, ImageSize, ImageBase) == FALSE)
			{
				LOG_DEBUG("GetSectionData--->GetProcessInfo Error\r\n");
				return FALSE;
			}

			LOG_DEBUG("ImageBase:0x%p ImageSize:0x%p\r\n", ImageBase, ImageSize);

			if (ImageBase && ImageSize)
			{
				PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ImageBase;
				PIMAGE_NT_HEADERS64 NtHeader = (PIMAGE_NT_HEADERS64)(DosHeader->e_lfanew + (ULONG64)ImageBase);
				ULONG NumSections = NtHeader->FileHeader.NumberOfSections;
				PIMAGE_SECTION_HEADER Section = IMAGE_FIRST_SECTION(NtHeader);

				STRING TargetSectionName;
				RtlInitString(&TargetSectionName, SectionName);

				for (ULONG i = 0; i < NumSections; i++)
				{
					STRING CurrentSectionName;
					RtlInitString(&CurrentSectionName, (PCSZ)Section->Name);
					if (CurrentSectionName.Length > 8)
						CurrentSectionName.Length = 8;

					LOG_DEBUG("CurrentSectionName:%s\r\n", Section->Name);

					if (RtlCompareString(&CurrentSectionName, &TargetSectionName, FALSE) == 0)
					{
						SectionSize = Section->Misc.VirtualSize;
						SectionBaseAddress = (PVOID)((ULONG64)ImageBase + (ULONG64)Section->VirtualAddress);

						return TRUE;
					}
					Section++;
				}
			}

			return FALSE;
		}

		//��Ҫ�ҿ���ʹ��
		PVOID AttachedProcess_GetModuleBase_x86(PEPROCESS pEProcess, PWCHAR pModuleName, OUT ULONG* SizeOfImage)
		{
			PVOID DllBase = nullptr;
			NTSTATUS nStatus;
			wdk::PPEB32 pPeb = NULL;

			UNICODE_STRING usModuleName;

			RtlInitUnicodeString(&usModuleName, pModuleName);
			pPeb = (wdk::PPEB32)PsGetProcessWow64Process(pEProcess);

			if (pPeb == NULL || pPeb->Ldr == 0)
			{
				return NULL;
			}

			for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((wdk::PPEB_LDR_DATA32)pPeb->Ldr)->InLoadOrderModuleList.Flink;
				pListEntry != (PLIST_ENTRY32)(&((wdk::PPEB_LDR_DATA32)pPeb->Ldr)->InLoadOrderModuleList);
				pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
			{
				wdk::PLDR_DATA_TABLE_ENTRY32 LdrEntry = CONTAINING_RECORD(pListEntry, wdk::LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);
				if (LdrEntry->BaseDllName.Buffer == NULL)
				{
					continue;
				}

				UNICODE_STRING usCurrentName = { 0 };
				RtlInitUnicodeString(&usCurrentName, (PWCHAR)LdrEntry->BaseDllName.Buffer);
				if (RtlEqualUnicodeString(&usModuleName, &usCurrentName, TRUE))
				{
					DllBase = (PVOID)LdrEntry->DllBase;
					*SizeOfImage = LdrEntry->SizeOfImage;

					//LOG_DEBUG("DllBase:%p  Size:0x%X\r\n", DllBase, *SizeOfImage);
					return DllBase;
				}
			}

			return NULL;
		}

		//��Ҫ�ҿ���ʹ��
		PVOID AttachedProcess_GetModuleBase_x64(PEPROCESS pEProcess, PWCHAR pModuleName, OUT ULONG* SizeOfImage)
		{
			PVOID DllBase = nullptr;
			auto vPeb = (wdk::PPEB)PsGetProcessPeb(pEProcess);
			if (vPeb == nullptr)
			{
				LOG_DEBUG("PEB is null\r\n", vPeb);
				return NULL;
			}

			//LOG_DEBUG("PEB:%p\r\n", vPeb);

			auto vLdr = (wdk::PPEB_LDR_DATA)vPeb->Ldr;
			auto vLdrHead = (PLIST_ENTRY)&vLdr->InLoadOrderModuleList;

			for (auto vLdrNext = vLdrHead->Flink; vLdrNext != vLdrHead; vLdrNext = vLdrNext->Flink)
			{
				auto vLdrEntry = CONTAINING_RECORD(vLdrNext, wdk::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
				auto vDllName = (PUNICODE_STRING)&vLdrEntry->BaseDllName;
				//LOG_DEBUG("%S", vDllName->Buffer);
				//
				if (_wcsnicmp(vDllName->Buffer, pModuleName, vDllName->Length / sizeof(wchar_t)) == 0)
				{
					DllBase = vLdrEntry->DllBase;
					*SizeOfImage = vLdrEntry->SizeOfImage;
					//LOG_DEBUG("DllBase:%p  Size:0x%X\r\n", DllBase, *SizeOfImage);
					return DllBase;
				}
			}

			return NULL;
		}

		NTSTATUS GetProcessModuleInfo(UINT64 ProcessID, WCHAR* DllName, OUT PULONG64 ModuleBase, OUT PULONG ModuleSize)
		{
			NTSTATUS status = STATUS_UNSUCCESSFUL;

			ULONG64	 DllBaseAddr = 0;
			ULONG    DllSize = 0;

			if (ModuleBase == nullptr || ModuleSize == nullptr)
			{
				return STATUS_UNSUCCESSFUL;
			}

			//�õ�ģ����Ϣ(��ʼ��ַ,ģ���С)
			PEPROCESS Process = NULL;
			status = PsLookupProcessByProcessId((HANDLE)ProcessID, &Process);

			if (!NT_SUCCESS(status))
			{
				return STATUS_UNSUCCESSFUL;
			}
			if (Process == NULL)
			{
				LOG_DEBUG("Process is null\r\n");
				return STATUS_UNSUCCESSFUL;
			}

			auto Is_x86 = PsGetProcessWow64Process(Process);

			::KAPC_STATE KAPC;

			KeStackAttachProcess(Process, &KAPC);

			if (Is_x86)
			{
				DllBaseAddr = (ULONG64)AttachedProcess_GetModuleBase_x86(Process, DllName, &DllSize);
			}
			else
			{
				DllBaseAddr = (ULONG64)AttachedProcess_GetModuleBase_x64(Process, DllName, &DllSize);
			}

			if (DllBaseAddr == 0)
			{
				LOG_DEBUG("ModuleBase is null\r\n");
				status = STATUS_UNSUCCESSFUL;
				goto $EXIT;
			}

			*ModuleBase = DllBaseAddr;
			*ModuleSize = DllSize;

			status = STATUS_SUCCESS;

		$EXIT:
			KeUnstackDetachProcess(&KAPC);
			ObfDereferenceObject(Process);
			Process = NULL;

			return status;
		}

		//ͨ��PEPROCESS�����ƻ��Dll��ģ��
		PVOID GetModuleBaseBySystemApi(PEPROCESS TargetProcess, PWCHAR ModuleName)
		{
			//https://github.com/imgits/scdetective/blob/1625a7f30900305711057806608cd2867056726c/ScDetective_Driver/ScDetective/Process/Process.c#L802
			if (TargetProcess == nullptr)
			{
				return nullptr;
			}
			PVOID result(nullptr);
			ULONG_PTR HighUserAddress(0x7FFFFFFF0000/*MmUserProbeAddress*/);
			ULONG ulRet(0);

			ULONG_PTR ulBase(0);
			LPVOID Current = NULL;
			do
			{
				wdk::MEMORY_BASIC_INFORMATION mbi;

				NTSTATUS Status = ZwQueryVirtualMemory(ZwCurrentProcess(),
					(PVOID)ulBase,
					(::MEMORY_INFORMATION_CLASS)(0),  //wdk::MEMORY_INFORMATION_CLASS::MemoryBasicInformation
					&mbi,
					sizeof(wdk::MEMORY_BASIC_INFORMATION),
					(PSIZE_T)&ulRet);

				if (NT_SUCCESS(Status))
				{

					//�����Image �ٲ�ѯSectionName,��FileObject Name
					if (mbi.Type == SEC_IMAGE)
					{

						wdk::MEMORY_MAPPED_FILE_NAME_INFORMATION msn;

						Status = ZwQueryVirtualMemory(ZwCurrentProcess(),
							(PVOID)ulBase,
							(::MEMORY_INFORMATION_CLASS)(2),  //wdk::MEMORY_INFORMATION_CLASS::MemoryMappedFilenameInformation
							&msn,
							sizeof(wdk::MEMORY_MAPPED_FILE_NAME_INFORMATION),
							(PSIZE_T)&ulRet);

						if (NT_SUCCESS(Status))
						{
							LOG_DEBUG("SectionName:%wZ,base=%p\r\n", &(msn.Name), ulBase);  //�ſ����Կ����ܶ���Ϣ
						}

					}

					ulBase += mbi.RegionSize;
				}
				else ulBase += PAGE_SIZE;
			} while (ulBase < (ULONG_PTR)HighUserAddress);

			return result;
		}

		PVOID GetModuleBase(PEPROCESS TargetProcess, const eastl::wstring& DllName, BOOL IsX64)
		{
			if (TargetProcess == nullptr)
			{
				return nullptr;
			}

			//if (!ddk::Nt_MemUtil::MmIsAccessibleAddress(eProcess))
			//{
			//	return nullptr;
			//}

			PVOID result(nullptr);
			HANDLE hProcess(nullptr);
			ULONG_PTR HighUserAddress(0x7FFFFFFF0000/*MmUserProbeAddress*/);
			ULONG ulRet(0);
			//#ifdef _WIN64
			//
			//				HighUserAddress = 0x80000000000;
			//#else
			//				HighUserAddress = 0x80000000;
			//#endif
			auto Status = ObOpenObjectByPointer(TargetProcess,
				OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
				NULL,
				GENERIC_ALL,
				*PsProcessType,
				KernelMode,
				&hProcess
			);
			if (NT_SUCCESS(Status))
			{
				ULONG_PTR ulBase(0);
				do
				{
					wdk::MEMORY_BASIC_INFORMATION mbi;

					Status = ZwQueryVirtualMemory(hProcess,
						(PVOID)ulBase,
						(::MEMORY_INFORMATION_CLASS)(0),  //wdk::MEMORY_INFORMATION_CLASS::MemoryBasicInformation
						&mbi,
						sizeof(wdk::MEMORY_BASIC_INFORMATION),
						(PSIZE_T)&ulRet);

					if (NT_SUCCESS(Status))
					{
						//�����Image �ٲ�ѯSectionName,��FileObject Name

						if (mbi.Type == SEC_IMAGE)
						{
							wdk::MEMORY_MAPPED_FILE_NAME_INFORMATION msn;

							Status = ZwQueryVirtualMemory(hProcess,
								(PVOID)ulBase,
								(::MEMORY_INFORMATION_CLASS)(2),  //wdk::MEMORY_INFORMATION_CLASS::MemoryMappedFilenameInformation
								&msn,
								sizeof(wdk::MEMORY_MAPPED_FILE_NAME_INFORMATION),
								(PSIZE_T)&ulRet);

							if (NT_SUCCESS(Status))
							{
								/*
								SectionName:\Device\HarddiskVolume2\Windows\SysWOW64\ntdll.dll,base=0000000077BF0000
								*/
								//UNICODE_STRING DosName;
								//LOG_DEBUG("SectionName:%wZ,base=%p\r\n", &(msn.Name), ulBase);  //�ſ����Կ����ܶ���Ϣ
								eastl::wstring temp(msn.Name.Buffer);
								if (DllName == oxorany(L"ntdll.dll"))
								{
									// ntdll ���⴦��
									if (!IsX64)
									{
										//X86Ӧ���Ƿ���x86���Ǹ�\Windows\SysWOW64\ntdll.dll
										if (temp.find(oxorany(L"\\Windows\\SysWOW64\\ntdll.dll")) !=
											eastl::wstring::npos
											)
										{
											result = (PVOID)(ulBase);
											break;
										}
									}

									else
									{
										//x64
										if (temp.find(oxorany(L"\\Windows\\System32\\ntdll.dll")) !=
											eastl::wstring::npos
											)
										{
											result = (PVOID)(ulBase);
											break;
										}
									}
								}
								else
								{
									//��ͨ����
									if (temp.find(DllName) != eastl::wstring::npos)
									{
										result = (PVOID)(ulBase);
										break;
									}
								}

								//LOG_DEBUG("MoudleName:%wZ\r\n", &msn.Name);
								//RtlVolumeDeviceToDosName(&(msn.Name), &DosName);
								//LOG_DEBUG("SectionName:%wZ\r\n", &(DosName));
							}
						}
						ulBase += mbi.RegionSize;
					}
					else ulBase += PAGE_SIZE;
				} while (ulBase < (ULONG_PTR)HighUserAddress);

				if (hProcess)
				{
					ZwClose(hProcess);
				}
			}

			return result;
		}

		PVOID GetNtOsKernelBase()
		{
			PVOID pret = NULL;
			UNICODE_STRING apiname = { 0 };
			PVOID apiaddr = NULL;

			RtlInitUnicodeString(&apiname, L"NtCreateFile");

			apiaddr = MmGetSystemRoutineAddress(&apiname);

			RtlPcToFileHeader((PVOID)apiaddr, &pret);

			return pret;
		}

		PVOID GetKernelBase(PULONG pImageSize)
		{
			PVOID pModuleBase = NULL;
			PSYSTEM_MODULE_INFORMATION pSystemInfoBuffer = NULL;

			ULONG SystemInfoBufferSize = 0;

			NTSTATUS status = ZwQuerySystemInformation(wdk::SystemModuleInformation,
				&SystemInfoBufferSize,
				0,
				&SystemInfoBufferSize);

			if (!SystemInfoBufferSize)
			{
				LOG_DEBUG("ZwQuerySystemInformation (1) failed...\r\n");
				return NULL;
			}

			pSystemInfoBuffer = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(NonPagedPool, SystemInfoBufferSize * 2);

			if (!pSystemInfoBuffer)
			{
				LOG_DEBUG("ExAllocatePool failed...\r\n");
				return NULL;
			}

			memset(pSystemInfoBuffer, 0, SystemInfoBufferSize * 2);

			status = ZwQuerySystemInformation(wdk::SystemModuleInformation,
				pSystemInfoBuffer,
				SystemInfoBufferSize * 2,
				&SystemInfoBufferSize);

			if (NT_SUCCESS(status))
			{
				pModuleBase = pSystemInfoBuffer->Module[0].ImageBase;
				if (pImageSize)
					*pImageSize = pSystemInfoBuffer->Module[0].ImageSize;
			}
			else
			{
				LOG_DEBUG("ZwQuerySystemInformation (2) failed...\r\n");
			}

			ExFreePool(pSystemInfoBuffer);

			return pModuleBase;
		}

		VOID GetSystemModuleBase(CHAR* ModuleName, ULONG64* pModuleBase, ULONG* pBufferSize)
		{
			ULONG NeedSize, i, ModuleCount, BufferSize = 0x5000;
			PVOID pBuffer = nullptr;
			PCHAR pDrvName = nullptr;
			NTSTATUS v_ret_status = { STATUS_UNSUCCESSFUL };
			wdk::PRTL_PROCESS_MODULES v_modules;
			do
			{
				pBuffer = ExAllocatePool(PagedPool, BufferSize);
				if (pBuffer == nullptr)
					return;
				v_ret_status = ZwQuerySystemInformation(wdk::SystemModuleInformation, pBuffer, BufferSize, &NeedSize);
				if (v_ret_status == STATUS_INFO_LENGTH_MISMATCH)
				{
					ExFreePool(pBuffer);
					BufferSize *= 2;
				}
				else if (!NT_SUCCESS(v_ret_status))
				{
					ExFreePool(pBuffer);
					return;
				}
			} while (v_ret_status == STATUS_INFO_LENGTH_MISMATCH);
			v_modules = static_cast<wdk::PRTL_PROCESS_MODULES>(pBuffer);
			ModuleCount = v_modules->NumberOfModules;
			for (i = 0; i < ModuleCount; i++)
			{
				if (reinterpret_cast<ULONG64>(v_modules->Modules[i].ImageBase) > static_cast<ULONG64>(0x8000000000000000))
				{
					pDrvName = reinterpret_cast<char*>(v_modules->Modules[i].FullPathName);
					if (strstr(pDrvName, ModuleName))
					{

						*pModuleBase = reinterpret_cast<ULONG64>(v_modules->Modules[i].ImageBase);
						*pBufferSize = v_modules->Modules[i].ImageSize;

						goto exit_sub;
					}
				}
			}
		exit_sub:
			ExFreePool(pBuffer);
		}

		NTSTATUS GetModuleNameForAddress(IN PVOID ProcessVa, OUT PCHAR FileNameBuff)
		{
			NTSTATUS status = STATUS_SUCCESS;
			if (FileNameBuff == nullptr)
			{
				return STATUS_UNSUCCESSFUL;
			}
			if (NT_SUCCESS(status))
			{
				ULONG need = 0;
				ZwQuerySystemInformation(wdk::SYSTEM_INFORMATION_CLASS::SystemModuleInformation, 0, 0, &need);
				auto buff = reinterpret_cast<PSYSTEM_MODULE_INFORMATION>(ExAllocatePool(NonPagedPool, need));
				if (!buff)
				{
					status = STATUS_UNSUCCESSFUL;
					return status;
				}
				status = ZwQuerySystemInformation(wdk::SYSTEM_INFORMATION_CLASS::SystemModuleInformation, buff, need, &need);
				if (!NT_SUCCESS(status))
				{
					ExFreePool(buff);
				}

				PSYSTEM_MODULE_ENTRY next;
				next = buff->Module;
				for (size_t i = 0; i < buff->Count; i++)
				{
					if (ProcessVa >= next->ImageBase && (next->ImageSize + (ULONG64)next->ImageBase) >= (ULONG64)ProcessVa)
					{
						strcpy(FileNameBuff, (const char*)next->FullPathName);
						delete buff;
						return STATUS_SUCCESS;
					}
					next++;
				}
				FileNameBuff[0] = '\0';
				delete buff;
				return STATUS_UNSUCCESSFUL;
			}
			return status;
		}

		




	}
}