#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.objs.h"

namespace KernelCommon
{
	namespace Utils
	{
#define NUMBER_HASH_BUCKETS 37

		BOOL ObjectTypesSnapShot(eastl::unordered_map<eastl::wstring, POBJECT_TYPE>& ObjTypes)
		{
			BOOL bRet = FALSE;
			OBJECT_ATTRIBUTES oa;
			UNICODE_STRING nsObjectTypes;
			RtlInitUnicodeString(&nsObjectTypes, L"\\ObjectTypes");
			InitializeObjectAttributes(&oa,
				&nsObjectTypes,
				OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
				nullptr,
				nullptr);
			HANDLE DirectoryHandle = nullptr;
			auto ns = ZwOpenDirectoryObject(&DirectoryHandle, DIRECTORY_QUERY, &oa);
			if (NT_SUCCESS(ns))
			{
				PVOID dir_object = nullptr;
				ns = ObReferenceObjectByHandle(DirectoryHandle,
					0,
					nullptr,
					KernelMode,
					&dir_object,
					nullptr);
				if (NT_SUCCESS(ns))
				{
					//LOG_DEBUG("Get Objecttypes Diretory OK\r\n");
					auto pDirectoryObject = reinterpret_cast<POBJECT_DIRECTORY>(dir_object);
					for (auto Bucket = 0; Bucket < NUMBER_HASH_BUCKETS; Bucket++)
					{
						auto DirectoryEntry = pDirectoryObject->HashBuckets[Bucket];
						while (DirectoryEntry != NULL)
						{
							wchar_t buffer[MAX_PATH * 2] = { 0 };
							auto d_size = 0UL;
							POBJECT_NAME_INFORMATION wcName = (POBJECT_NAME_INFORMATION)buffer;
							auto ns = ObQueryNameString(DirectoryEntry->Object,
								(POBJECT_NAME_INFORMATION)wcName,
								sizeof(buffer),
								&d_size);
							if (NT_SUCCESS(ns))
							{
								//LOG_DEBUG("ObjectTypes:%wZ\r\n", &wcName->Name);
								ObjTypes[eastl::wstring(wcName->Name.Buffer)] = reinterpret_cast<POBJECT_TYPE>(DirectoryEntry->Object);
							}
							DirectoryEntry = DirectoryEntry->ChainLink;
						}
					}
					ObfDereferenceObject(dir_object);
					bRet = TRUE;
				}
				ZwClose(DirectoryHandle);
			}

			return bRet;
		}

		BOOL RPCControlSnapShot(eastl::unordered_map<eastl::wstring, PVOID>& RPCObjs)
		{
			BOOL bRet = FALSE;
			OBJECT_ATTRIBUTES oa;
			UNICODE_STRING nsRPCControls;
			RtlInitUnicodeString(&nsRPCControls, L"\\RPC Control");
			InitializeObjectAttributes(&oa,
				&nsRPCControls,
				OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
				nullptr,
				nullptr);
			HANDLE DirectoryHandle = nullptr;
			auto ns = ZwOpenDirectoryObject(&DirectoryHandle, DIRECTORY_QUERY, &oa);
			if (NT_SUCCESS(ns))
			{
				PVOID dir_object = nullptr;
				ns = ObReferenceObjectByHandle(DirectoryHandle,
					0,
					nullptr,
					KernelMode,
					&dir_object,
					nullptr);
				if (NT_SUCCESS(ns))
				{
					//LOG_DEBUG("Get Objecttypes Diretory OK\r\n");
					auto pDirectoryObject = reinterpret_cast<POBJECT_DIRECTORY>(dir_object);
					for (auto Bucket = 0; Bucket < NUMBER_HASH_BUCKETS; Bucket++)
					{
						auto DirectoryEntry = pDirectoryObject->HashBuckets[Bucket];
						while (DirectoryEntry != NULL)
						{
							wchar_t buffer[MAX_PATH * 2] = { 0 };
							auto d_size = 0UL;
							POBJECT_NAME_INFORMATION wcName = (POBJECT_NAME_INFORMATION)buffer;
							auto ns = ObQueryNameString(DirectoryEntry->Object,
								(POBJECT_NAME_INFORMATION)wcName,
								sizeof(buffer),
								&d_size);
							if (NT_SUCCESS(ns))
							{
								//LOG_DEBUG("ObjectTypes:%wZ\r\n", &wcName->Name);
								RPCObjs[eastl::wstring(wcName->Name.Buffer)] = reinterpret_cast<PVOID>(DirectoryEntry->Object);
							}
							DirectoryEntry = DirectoryEntry->ChainLink;
						}
					}
					ObfDereferenceObject(dir_object);
					bRet = TRUE;
				}
				ZwClose(DirectoryHandle);
			}

			return bRet;
		}

		POBJECT_TYPE GetObjectType(const eastl::wstring& Name)
		{
			eastl::unordered_map<eastl::wstring, POBJECT_TYPE> ObjTypes;
			if (ObjectTypesSnapShot(ObjTypes))
			{
				if (ObjTypes.find(Name) != ObjTypes.end())
				{
					return ObjTypes[Name];
				}
			}
			return nullptr;
		}

		PVOID GetRPCControl(const eastl::wstring& Name)
		{
			eastl::unordered_map<eastl::wstring, PVOID> RPCObjs;
			if (RPCControlSnapShot(RPCObjs))
			{
				if (RPCObjs.find(Name) != RPCObjs.end())
				{
					return RPCObjs[Name];
				}
			}
			return nullptr;
		}
	}
}