#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.dll.h"

namespace KernelCommon
{
	namespace Utils
	{
		extern "C" POBJECT_TYPE * MmSectionObjectType;

		_IRQL_requires_max_(PASSIVE_LEVEL)
			NTSTATUS NTAPI RtlGetKnownDllExtents(
				_In_ PUNICODE_STRING SectionName,
				_Out_ PVOID* BaseAddress,
				_Out_ SIZE_T* ImageSize
			)
		{
			NTSTATUS Status = STATUS_SUCCESS;
			HANDLE   SectionHandle = nullptr;
			PVOID    SectionObject = nullptr;
			PVOID    MappedBase = nullptr;
			SIZE_T   MappedSize = 0u;

			do
			{
				//NT_ASSERT(PsInitialSystemProcess == PsGetCurrentProcess());

				OBJECT_ATTRIBUTES ObjectAttributes;
				InitializeObjectAttributes(
					&ObjectAttributes,
					SectionName,
					OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
					nullptr,
					nullptr);

				Status = ImpCall(ZwOpenSection,
					&SectionHandle,
					SECTION_MAP_READ | SECTION_QUERY,
					&ObjectAttributes);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				SECTION_IMAGE_INFORMATION SectionImageInfo;

				Status = ImpCall(ZwQuerySection,
					SectionHandle,
					SectionImageInformation,
					&SectionImageInfo,
					sizeof(SectionImageInfo),
					nullptr);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				//
				// 21H2 no longer maps ntdll as an image in System. Querying the transfer
				// address to get the extents in this context will fail. Rather than rely
				// on ZwQueryVirtualMemory at all, we'll map it into system space to get
				// the extents.
				//
				// Note, in the future if Microsoft decides to relocate KnownDLLs in
				// every process we'll need to revisit this. That will likely involve
				// retrieving the module extents per-process, in our case this is used
				// for KPH communications validation so we'll need to the ntdll module
				// extents out of PH.
				//

				Status = ImpCall(ObReferenceObjectByHandle,
					SectionHandle,
					SECTION_MAP_READ | SECTION_QUERY,
					*MmSectionObjectType,
					KernelMode,
					&SectionObject,
					nullptr);
				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = ImpCall(MmMapViewInSystemSpace, SectionObject, &MappedBase, &MappedSize);
				if (!NT_SUCCESS(Status))
				{
					break;
				}

				*BaseAddress = SectionImageInfo.TransferAddress;
				*ImageSize = MappedSize;

			} while (false);

			if (MappedBase)
			{
				ImpCall(MmUnmapViewInSystemSpace, MappedBase);
			}

			if (SectionObject)
			{
				ImpCall(ObfDereferenceObject, SectionObject);
			}

			if (SectionHandle)
			{
				ImpCall(ZwClose, SectionHandle);
			}

			return Status;
		}
	

		PVOID Load_Dll(eastl::wstring FileName)
		{
			HANDLE hSection, hFile;
			UNICODE_STRING dllName;
			PVOID BaseAddress = NULL;
			SIZE_T size = 0;
			NTSTATUS stat;
			OBJECT_ATTRIBUTES oa = { sizeof(oa), 0, &dllName, OBJ_CASE_INSENSITIVE };
			IO_STATUS_BLOCK iosb;
			auto full_dll_path = FileName.c_str();

			KIRQL CurrentIrql = ImpCall(KeGetCurrentIrql);

			if (CurrentIrql != (KIRQL)0)
			{
				LOG_DEBUG("DBG: ABout to load %ws at IRQL %d\n", full_dll_path, CurrentIrql);
				__debugbreak();
			}

			ImpCall(RtlInitUnicodeString, &dllName, full_dll_path);

			stat = ImpCall(ZwOpenFile, &hFile, FILE_EXECUTE | SYNCHRONIZE, &oa, &iosb,
				FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);

			if (!NT_SUCCESS(stat))
			{
				LOG_DEBUG("WRN: Can't open %ws: %x\n", full_dll_path, stat);
				return 0;
			}

			oa.ObjectName = 0;

			stat = ImpCall(ZwCreateSection, &hSection, SECTION_ALL_ACCESS, &oa, 0, PAGE_EXECUTE,
				0x01000000, hFile);

			if (!NT_SUCCESS(stat))
			{
				LOG_DEBUG("WRN: Can't create section %ws: %x\n", full_dll_path, stat);
				return 0;
			}

			stat = ImpCall(ZwMapViewOfSection, hSection, NtCurrentProcess(), &BaseAddress, 0,
				1000, 0, &size, (SECTION_INHERIT)1, MEM_TOP_DOWN, PAGE_READWRITE);

			if (!NT_SUCCESS(stat))
			{
				LOG_DEBUG("WRN: Can't map section %ws: %x\n", full_dll_path, stat);
				return 0;
			}

			ImpCall(ZwClose, hSection);
			ImpCall(ZwClose, hFile);

			//LOG_DEBUG("DBG: Successfully loaded %ws\n", full_dll_path);
			return BaseAddress;
		}

		VOID Free_Dll(HANDLE hMod)
		{
			ImpCall(ZwUnmapViewOfSection, NtCurrentProcess(), hMod);
		}

		ULONG LdrGetImageSize(PVOID ImageBase)
		{
			auto size = 0UL;

			if (ImageBase == NULL)
			{
				return 0;
			}

			do
			{
				__try
				{
					auto dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
					if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
					{
						break;
					}
					auto ntheader = (PIMAGE_NT_HEADERS)((PUCHAR)ImageBase + dosHeader->e_lfanew);
					if (ntheader->Signature != IMAGE_NT_SIGNATURE)
					{
						break;
					}
					if (ntheader)
					{
					}
					size = (ntheader->OptionalHeader.SizeOfCode);
				}
				__except (1)
				{
					return 0UL;
				}
			} while (0);

			return size;
		}
	}
}
