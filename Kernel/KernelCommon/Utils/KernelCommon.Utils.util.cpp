#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.util.h"

namespace KernelCommon
{
	namespace Utils
	{
		ULONG g_RandomSeed = 0;

		KIRQL RaiseIrql()
		{
			KIRQL irql = KeRaiseIrqlToDpcLevel();
			_disable();
			return irql;
		}

		VOID LowerIrql(KIRQL irql)
		{
			_enable();
			KeLowerIrql(irql);
		}

		KIRQL WPOFFx64()
		{
			KIRQL irql = KeRaiseIrqlToDpcLevel();
			UINT64 cr0 = __readcr0();
			cr0 &= 0xfffffffffffeffff;
			__writecr0(cr0);
			_disable();
			return irql;
		}

		VOID WPONx64(KIRQL irql)
		{
			UINT64 cr0 = __readcr0();
			cr0 |= 0x10000;
			_enable();
			__writecr0(cr0);
			KeLowerIrql(irql);
		}

		eastl::string GetCpuID()
		{
			eastl::string strResult;
			char szBuffer[MAX_PATH] = { 0 };
			int Array[4] = { 0 };
			__cpuid(Array, 1);
			for (int i = 0; i < 4; i++)
			{
				if (i == 1)
					continue;
				sprintf(szBuffer, "%0.8X", Array[i]);
				strResult += szBuffer;
			}

			//LOG_DEBUG("CPUID:%s\r\n", strResult.c_str());
			return strResult;
		}

		SIZE_T AlignSize(SIZE_T nSize, UINT nAlign)
		{
			if (nAlign == 0)
			{
				return nSize;
			}

			return ((nSize + nAlign - 1) / nAlign * nAlign);
		}

		BOOL IsKernelAddress(PVOID Address)
		{
#ifdef _M_AMD64
			ULONG_PTR kernelStarts = 0x800000000000;
#else
			ULONG_PTR kernelStarts = 0x80000000;
#endif

			if ((ULONG_PTR)Address <= kernelStarts)
				return FALSE;

			if (!MmIsAddressValid(Address))
				return FALSE;

			return TRUE;
		}

		BOOL ProbeUserAddress(PVOID addr, SIZE_T size, ULONG alignment)
		{
			if (size == 0) 
			{
				return TRUE;
			}

			ULONG_PTR current = (ULONG_PTR)addr;
			auto temp = ((ULONG_PTR)addr & (alignment - 1));

			if (((ULONG_PTR)addr & (alignment - 1)) != 0) {
				return FALSE;
			}

			ULONG_PTR last = current + size - 1;
			if ((last < current) || (last >= 0x7FFFFFFF0000/*MmUserProbeAddress*/)) 
			{
				return FALSE;
			}

			return TRUE;
		}

		VOID AdjustRelativePointers(PBYTE buffer, PBYTE target, SIZE_T size) 
		{
			if (size < sizeof(PVOID)) 
			{
				return;
			}

			for (SIZE_T i = 0; i <= size - sizeof(PVOID); i += sizeof(ULONG)) 
			{
				PVOID* ptr = (PVOID*)(buffer + i);
				SIZE_T offset = (PBYTE)*ptr - buffer;

				if (offset < size) 
				{
					*ptr = target + offset;
					i += sizeof(ULONG);
				}
			}
		}

		ULONG RtlNextRandom(ULONG Min, ULONG Max) // [Min,Max)
		{
			#define RANDOM_SEED_INIT 0x3AF84E05

			if (g_RandomSeed == 0)
			{
				g_RandomSeed = RANDOM_SEED_INIT;
			}

			if (g_RandomSeed == RANDOM_SEED_INIT)  // One-time seed initialisation. It doesn't have to be good, just not the same every time
				g_RandomSeed = static_cast<ULONG>(__rdtsc());

			// NB: In user mode, the correct scale for RtlUniform/RtlRandom/RtlRandomEx is different on Win 10+:
			// Scale = (RtlNtMajorVersion() >= 10 ? MAXUINT32 : MAXINT32) / (Max - Min);
			// The KM versions seem to have been unaffected by this change, at least up until RS3.
			// If this ever starts returning values >= Max, try the above scale instead
			const ULONG Scale = static_cast<ULONG>(MAXINT32) / (Max - Min);
			return RtlRandomEx(&g_RandomSeed) / Scale + Min;
		}

		ULONG GetPoolTag()
		{
			constexpr ULONG PoolTags[] =
			{
				' prI', // Allocated IRP packets
				'+prI', // I/O verifier allocated IRP packets
				'eliF', // File objects
				'atuM', // Mutant objects
				'sFtN', // ntfs.sys!StrucSup.c
				'ameS', // Semaphore objects
				'RwtE', // Etw KM RegEntry
				'nevE', // Event objects
				' daV', // Mm virtual address descriptors
				'sdaV', // Mm virtual address descriptors (short)
				'aCmM', // Mm control areas for mapped files
				'  oI', // I/O manager
				'tiaW', // WaitCompletion Packets
				'eSmM', // Mm secured VAD allocation
				'CPLA', // ALPC port objects
				'GwtE', // ETW GUID
				' ldM', // Memory Descriptor Lists
				'erhT', // Thread objects
				'cScC', // Cache Manager Shared Cache Map
				'KgxD', // Vista display driver support
			};

			constexpr ULONG NumPoolTags = ARRAYSIZE(PoolTags);
			const ULONG Index = RtlNextRandom(0, NumPoolTags);
			NT_ASSERT(Index <= NumPoolTags - 1);
			return PoolTags[Index];
		}

		PVOID FindSignature(PVOID Memory, ULONG64 Size, PCSZ Pattern, PCSZ Mask)
		{
			ULONG64 SigLength = strlen(Mask);
			if (SigLength > Size) return NULL;

			for (ULONG64 i = 0; i < Size - SigLength; i++)
			{
				BOOLEAN Found = TRUE;
				for (ULONG64 j = 0; j < SigLength; j++)
					Found &= Mask[j] == '?' || Pattern[j] == *((PCHAR)Memory + i + j);

				if (Found)
					return (PCHAR)Memory + i;
			}
			return NULL;
		}

		NTSTATUS BBSearchPattern(IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound)
		{
			ULONG_PTR i, j;
			if (ppFound == NULL || pattern == NULL || base == NULL)
				return STATUS_INVALID_PARAMETER;

			for (i = 0; i < size - len; i++)
			{
				BOOL found = TRUE;
				for (j = 0; j < len; j++)
				{
					if (pattern[j] != wildcard && pattern[j] != ((PCUCHAR)base)[i + j])
					{
						found = FALSE;
						break;
					}
				}

				if (found != FALSE)
				{
					*ppFound = (PUCHAR)base + i;
					return STATUS_SUCCESS;
				}
			}

			return STATUS_NOT_FOUND;
		}

		BOOLEAN CheckMask(PCHAR base, PCHAR pattern, PCHAR mask)
		{
			for (; *mask; ++base, ++pattern, ++mask)
			{
				if (*mask == 'x' && *base != *pattern)
				{
					return FALSE;
				}
			}
			return TRUE;
		}

		PVOID FindPattern(PCHAR base, ULONG length, PCHAR pattern, PCHAR mask)
		{
			length -= (ULONG)strlen(mask);
			for (ULONG i = 0; i <= length; ++i)
			{
				PVOID addr = &base[i];
				if (CheckMask((PCHAR)addr, pattern, mask))
				{
					return addr;
				}
			}

			return 0;
		}

		PVOID FindPatternImage(PCHAR base, PCHAR pattern, PCHAR mask)
		{
			PVOID match = NULL;
			if (base)
			{
				//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "FindPatternImage 1\r\n");

				PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
				PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);
				for (ULONG i = 0; i < headers->FileHeader.NumberOfSections; ++i)
				{
					PIMAGE_SECTION_HEADER section = &sections[i];
					if (memcmp(section->Name, ".text", 5) == 0)
					{
						match = FindPattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
						if (match)
						{
							break;
						}
					}
				}
			}

			return match;
		}

		VOID NewExInitializeFastMutex(
			_Out_ PFAST_MUTEX FastMutex
		)
		{
			WriteRaw(&FastMutex->Count, FM_LOCK_BIT);
			FastMutex->Owner = NULL;
			FastMutex->Contention = 0;
			KeInitializeEvent(&FastMutex->Event, SynchronizationEvent, FALSE);
			return;
		}

		PWCHAR DosFullPathToModuleNameW(PWCHAR FullPath)
		{
			PWCHAR FindStr = wcsrchr(FullPath, 0x5c);   //0x5c = '\'
			return FindStr == NULL ? NULL : FindStr + 1;
		}

		ULONG GetPreviousModeOffset()
		{
			auto PrevModeOffset = 0UL;

			PVOID fnExGetPreviousMode = ExGetPreviousMode;

			fnExGetPreviousMode = GetSystemRoutineAddress(oxorany("ExGetPreviousMode"));

			if (fnExGetPreviousMode)
			{
				//LOG_DEBUG("ExGetPreviousMode %p\r\n", fnExGetPreviousMode);
				UCHAR PreviousModePattern[] = "\x00\x00\xC3";
				auto pFound = MmMemSearch((const void*)fnExGetPreviousMode, 32, PreviousModePattern, sizeof(PreviousModePattern) - 1);
				if (pFound)
					PrevModeOffset = *(DWORD*)((PUCHAR)pFound - 2);
			}

			//LOG_DEBUG("PrevModeOffset:%d\r\n", PrevModeOffset);
			return PrevModeOffset;
		}

		KPROCESSOR_MODE KeSetPreviousMode(KPROCESSOR_MODE mode)
		{
			KPROCESSOR_MODE old = ExGetPreviousMode();
			*(KPROCESSOR_MODE*)((PBYTE)KeGetCurrentThread() + GetPreviousModeOffset()) = mode;
			return old;
		}

		NTSTATUS HidePCHDriver(PDRIVER_OBJECT a_self_driver_object)
		{
			NTSTATUS Status{ STATUS_SUCCESS };
			auto v_self_entry = static_cast<wdk::PKLDR_DATA_TABLE_ENTRY>(a_self_driver_object->DriverSection);
			wdk::PKLDR_DATA_TABLE_ENTRY	v_fist_entry = nullptr;
			UNICODE_STRING v_pch_sys_name = { 0 };
			wdk::PKLDR_DATA_TABLE_ENTRY v_target_entry{ nullptr };
			RtlInitUnicodeString(&v_pch_sys_name, L"PCHUNTER*");
			v_fist_entry = v_self_entry;

			__try
			{
				do
				{
					if (v_self_entry->BaseDllName.Buffer != nullptr)
					{
						if (FsRtlIsNameInExpression(&v_pch_sys_name, &v_self_entry->BaseDllName, TRUE, nullptr))
						{
							v_target_entry = v_self_entry;
							break;
						}
						v_self_entry = reinterpret_cast<wdk::PKLDR_DATA_TABLE_ENTRY>(v_self_entry->InLoadOrderLinks.Blink);
					}
				} while (v_self_entry->InLoadOrderLinks.Blink != reinterpret_cast<PLIST_ENTRY>(v_fist_entry));
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return NULL;
			}

			if (v_target_entry)
			{
				const PLIST_ENTRY v_target_entry_pointer = &(v_target_entry->InLoadOrderLinks);
				RemoveEntryList(v_target_entry_pointer);
				v_target_entry_pointer->Flink = nullptr;
				v_target_entry_pointer->Blink = nullptr;
			}

			return Status;
		}

		BOOL NtFileNameToDosFileName(IN PUNICODE_STRING us, OUT WCHAR* ws)
		{
			HANDLE hFile = NULL;
			OBJECT_ATTRIBUTES ObjectAttributes;
			IO_STATUS_BLOCK IoStatusBlock;
			PFILE_OBJECT FileObject = NULL;
			POBJECT_NAME_INFORMATION pObjectNameInfo = 0;
			BOOL boole = FALSE;

			InitializeObjectAttributes(&ObjectAttributes, us, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

			if (NT_SUCCESS(ZwOpenFile(&hFile, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &ObjectAttributes, &IoStatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT)))
			{
				if (NT_SUCCESS(ObReferenceObjectByHandle(hFile, FILE_READ_ATTRIBUTES, *IoFileObjectType, KernelMode, (PVOID*)&FileObject, NULL)))
				{
					if (NT_SUCCESS(IoQueryFileDosDeviceName(FileObject, &pObjectNameInfo)))
					{
						wcscpy(ws, pObjectNameInfo->Name.Buffer);
						boole = TRUE;
					}
				}
			}
			if (pObjectNameInfo)
			{
				ExFreePool(pObjectNameInfo);
			}

			if (FileObject)
			{
				ObfDereferenceObject(FileObject);
			}

			if (hFile)
			{
				ZwClose(hFile);
			}

			return boole;
		}

		ULONG GetSystemStartTime()
		{
			LARGE_INTEGER la;
			ULONG MyInc;
			MyInc = KeQueryTimeIncrement();
			KeQueryTickCount(&la);
			la.QuadPart *= MyInc;
			la.QuadPart /= 10000;
			return la.LowPart;
		}

		bool GetModuleFileName(OUT WCHAR* fileName, IN PUNICODE_STRING filePath)
		{
			if (filePath)
			{
				if (StrIsValid2(*filePath))
				{
					int Full_length = filePath->Length / sizeof(WCHAR);
					int i = Full_length - 1;

					while ((filePath->Buffer[i] != L'\\') && (i > 0))
					{
						i--;
					}

					if (filePath->Buffer[i] == L'\\')
					{
						int fileNameLen = Full_length - (i + 1);
						wcsncpy(fileName, &filePath->Buffer[i + 1], fileNameLen);
						return true;
					}
					else
					{
						wcsncpy(fileName, &filePath->Buffer[i], Full_length);
						return true;
					}
				}
			}
			return false;
		}

		//获取进程名
		NTSTATUS GetProcessName(IN PEPROCESS Process, OUT WCHAR* fileName)
		{
			NTSTATUS Status;
			PUNICODE_STRING ImageFileName;
			Status = SeLocateProcessImageName(Process, &ImageFileName);
			if (NT_SUCCESS(Status))
			{
				if (GetModuleFileName(fileName, ImageFileName))
				{
					Status = STATUS_SUCCESS;
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}

				if (ImageFileName)
				{
					ExFreePool(ImageFileName);
				}
			}
			return Status;
		}
	}
}
