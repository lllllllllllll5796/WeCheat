#pragma once

namespace KernelCommon
{
	class Driver : public eastl::experimental::singleton<Driver>
	{
	public:
		Driver() 
		{
			m_count = 0;
		}

		PDRIVER_OBJECT GetNewDriver()
		{
			GUID Attach;
			wchar_t szGuid[MAX_PATH] = { 0 };
			m_lock.lock();
			auto lock_free = eastl::experimental::make_scope_exit([&]() {
				m_lock.unlock();
			});
			auto ns = ExUuidCreate(&Attach);
			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG("ExUuidCreate ns = %x\r\n", ns);
				return nullptr;
			}
#if !defined(HIDDEN)
			RtlStringCchPrintfW(szGuid, MAX_PATH,
				L"\\Driver\\{%08x-%04x-%04x-%02x-%02x-%02x-%02x}",
				Attach.Data1,
				Attach.Data2,
				Attach.Data3,
				Attach.Data4[0],
				Attach.Data4[1],
				Attach.Data4[2],
				Attach.Data4[3]);
#else
			_swprintf(szGuid,
				L"\\Driver\\{%08x-%04x-%04x-%02x-%02x-%02x-%02x}",
				Attach.Data1,
				Attach.Data2,
				Attach.Data3,
				Attach.Data4[0],
				Attach.Data4[1],
				Attach.Data4[2],
				Attach.Data4[3]);
#endif

			UNICODE_STRING nsAttachName;
			RtlInitUnicodeString(&nsAttachName, szGuid);
			ns = wdk::IoCreateDriver(&nsAttachName, (PDRIVER_INITIALIZE)Driver::NewDriverObject);
			if (NT_SUCCESS(ns))
			{
				if (!m_drvobj_list.empty())
				{
					return m_drvobj_list.back();
				}
			}
			return nullptr;
		}
		static
			NTSTATUS NTAPI
			NewDriverObject(
				IN PDRIVER_OBJECT driverObject,
				IN PUNICODE_STRING registryPath
			)
		{
			UNREFERENCED_PARAMETER(registryPath);
			Driver::get()->AddDriverObj(driverObject);
			return STATUS_SUCCESS;
		}
		void AddDriverObj(PDRIVER_OBJECT drv_obj)
		{
			drv_obj->DriverUnload = nullptr;
			m_drvobj_list.push_back(drv_obj);
			InterlockedIncrement(&m_count);
		}
		void DelDriverObj(PDRIVER_OBJECT drv_obj)
		{
			m_lock.lock();
			if (drv_obj->DriverUnload)
			{
				drv_obj->DriverUnload(drv_obj);
			}
			wdk::IoDeleteDriver(drv_obj);
			//ObMakeTemporaryObject(drv_obj); 这样子删除时，有一定几率爆炸
			m_lock.unlock();
		}
	private:
		LONG m_count;
		eastl::vector<PDRIVER_OBJECT>m_drvobj_list;
		mutexlock m_lock;
	};
}

