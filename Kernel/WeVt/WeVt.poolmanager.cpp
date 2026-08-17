#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"
#include "WeVt.AllocateMem.h"
#include "WeVt.Trace.h"
#include "WeVt.poolmanager.tmh"

namespace pool_manager
{
	bool request_pool_x(allocation_intention intention, unsigned __int64* address)
	{
		bool is_recycled = false;
		PLIST_ENTRY current = g_vmm_context.pool_manager->list_of_allocated_pools;

		spinlock::lock(&g_vmm_context.pool_manager->lock_for_reading_pool);

		// 遍历链表
		for (PLIST_ENTRY entry = current->Flink;
			entry != current;
			entry = entry->Flink)
		{
			__pool_table* pool_table = CONTAINING_RECORD(entry, __pool_table, pool_list);

			if (pool_table->intention == intention && !pool_table->is_busy)
			{
				pool_table->is_busy = true;
				is_recycled = pool_table->recycled;
				*address = (unsigned __int64)pool_table->address;
				break;
			}
		}

		spinlock::unlock(&g_vmm_context.pool_manager->lock_for_reading_pool);
		return is_recycled;
	}

	/// <summary>
	/// Allocate pools and add them to pool table
	/// 分配池并将其添加到池表中
	/// </summary>
	/// <param name="size">Size of pool</param>
	/// <param name="count">Number of pools to allocate</param>
	/// <param name="intention"></param>
	/// <returns></returns>
	bool allocate_pool(unsigned __int64 size, unsigned __int32 count, allocation_intention intention)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			__pool_table* single_pool = ::allocate_pool<__pool_table>();
			if (single_pool == nullptr)
			{
#if ENABLE_TRACE
				TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Memory allocation failed");
#endif
				return false;
			}
			RtlSecureZeroMemory(single_pool, sizeof(__pool_table));

			single_pool->address = allocate_aligned_pool<void*>(size);

			if (single_pool->address == nullptr)
			{
#if ENABLE_TRACE
				TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Memory allocation failed");
#endif
				return false;
			}
			RtlSecureZeroMemory(single_pool->address, size);

			single_pool->intention = intention;
			single_pool->is_busy = false;  //将内存标识为未使用
			single_pool->size = size;

			InsertTailList(g_vmm_context.pool_manager->list_of_allocated_pools, &(single_pool->pool_list));
		}

		return true;
	}

	/// <summary>
	/// Request allocation
	/// </summary>
	/// <param name="size">Size of pool</param>
	/// <param name="count">Number of pools to allocate</param>
	/// <param name="intention"></param>
	/// <returns></returns>
	bool request_allocation(unsigned __int64 size, unsigned __int32 count, allocation_intention intention)
	{
		spinlock::lock(&g_vmm_context.pool_manager->lock_for_request_allocation);

		for (unsigned __int64 i = 0; i < 10; i++)
		{
			if (g_vmm_context.pool_manager->allocation_requests->size[i] == 0)
			{
				g_vmm_context.pool_manager->allocation_requests->count[i] = count;
				g_vmm_context.pool_manager->allocation_requests->size[i] = size;
				g_vmm_context.pool_manager->allocation_requests->intention[i] = intention;
				g_vmm_context.pool_manager->is_request_for_allocation_recived = true;
				break;
			}
		}

		spinlock::unlock(&g_vmm_context.pool_manager->lock_for_request_allocation);
		return g_vmm_context.pool_manager->is_request_for_allocation_recived;
	}

	/// <summary>
	/// 分配所有请求的池
	/// </summary>
	/// <returns></returns>
	bool perform_allocation()
	{
		bool status = true;

		if (g_vmm_context.pool_manager->is_request_for_allocation_recived == false)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] No pending allocations");
#endif
			return status;
		}

		for (unsigned __int64 i = 0; i < 10; i++)
		{
			if (g_vmm_context.pool_manager->allocation_requests->size[i] != 0)
			{
				status = allocate_pool
				(
					g_vmm_context.pool_manager->allocation_requests->size[i],
					g_vmm_context.pool_manager->allocation_requests->count[i],
					g_vmm_context.pool_manager->allocation_requests->intention[i]
				);

				if (status == false)
				{
#if ENABLE_TRACE
					TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Pool manager allocate and add to pool table failed");
#endif
					break;
				}

				g_vmm_context.pool_manager->allocation_requests->size[i] = 0;
				g_vmm_context.pool_manager->allocation_requests->count[i] = 0;
				g_vmm_context.pool_manager->allocation_requests->intention[i] = INTENTION_NONE;
#if ENABLE_TRACE
				TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] Allocation successful");
#endif
			}
		}

		g_vmm_context.pool_manager->is_request_for_allocation_recived = false;

		return status;
	}

	/// <summary>
	/// Initalize pool manager struct and preallocate pools
	/// 初始化池管理器结构并预分配池
	/// </summary>
	/// <returns> status </returns>
	bool initialize()
	{
		g_vmm_context.pool_manager = ::allocate_pool<__pool_manager>();
		if (g_vmm_context.pool_manager == nullptr)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Pool manager allocation failed");
#endif
			return false;
		}
		RtlSecureZeroMemory(g_vmm_context.pool_manager, sizeof(__pool_manager));

		g_vmm_context.pool_manager->allocation_requests = ::allocate_pool<__request_new_allocation>();
		if (g_vmm_context.pool_manager->allocation_requests == nullptr)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Allacation requests allocation failed");
#endif
			return false;
		}
		RtlSecureZeroMemory(g_vmm_context.pool_manager->allocation_requests, sizeof(__request_new_allocation));

		g_vmm_context.pool_manager->list_of_allocated_pools = ::allocate_pool<LIST_ENTRY>();
		if (g_vmm_context.pool_manager->list_of_allocated_pools == nullptr)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] List of allocated pools allocation failed");
#endif
			return false;
		}
		RtlSecureZeroMemory(g_vmm_context.pool_manager->list_of_allocated_pools, sizeof(LIST_ENTRY));

		InitializeListHead(g_vmm_context.pool_manager->list_of_allocated_pools);

		unsigned __int64 buffer_count = g_vmm_context.processor_count * 100;

		if (request_allocation(sizeof(__ept_dynamic_split), buffer_count, INTENTION_SPLIT_PML2) == false)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Pool mangaer request allocation Failed");
#endif
			return false;
		}

		if (request_allocation(sizeof(__ept_hooked_page_info), buffer_count, INTENTION_TRACK_HOOKED_PAGES) == false)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Pool mangaer request allocation Failed");
#endif
			return false;
		}

		if (request_allocation(100, buffer_count, INTENTION_EXEC_TRAMPOLINE) == false)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Pool mangaer request allocation Failed");
#endif
			return false;
		}

		if (request_allocation(sizeof(__ept_hooked_function_info), buffer_count, INTENTION_TRACK_HOOKED_FUNCTIONS) == false)
		{
#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] Pool mangaer request allocation Failed");
#endif
			return false;
		}

		return perform_allocation();
	}
	/// <summary>
	/// Free all allocted pools
	/// 释放所有已分配的池
	/// </summary>
	void uninitialize()
	{
		PLIST_ENTRY ListHead, NextEntry, DelEntry;

		if (g_vmm_context.pool_manager->list_of_allocated_pools != nullptr)
		{
			ListHead = g_vmm_context.pool_manager->list_of_allocated_pools;
			NextEntry = ListHead->Flink;

			while (ListHead != NextEntry)
			{
				// Get the head of the record
				__pool_table* pool_table = (__pool_table*)CONTAINING_RECORD(NextEntry, __pool_table, pool_list);

				// Free the alloocated buffer
				free_aligned_pool(pool_table->address);

				DelEntry = NextEntry;
				/* Move to the next entry */
				NextEntry = NextEntry->Flink;

				RemoveEntryList(DelEntry);
				// Free the record itself
				free_pool(pool_table);
			}

			free_pool(g_vmm_context.pool_manager->list_of_allocated_pools);
		}

		if (g_vmm_context.pool_manager->allocation_requests != nullptr)
		{
			free_pool(g_vmm_context.pool_manager->allocation_requests);
		}
	}

	/// <summary>
	/// Set information that pool is no longer used anymore
	/// </summary>
	/// <param name="address"></param>
	void release_pool(void* address)
	{
		PLIST_ENTRY current = 0;
		current = g_vmm_context.pool_manager->list_of_allocated_pools;

		spinlock::lock(&g_vmm_context.pool_manager->lock_for_reading_pool);
		while (g_vmm_context.pool_manager->list_of_allocated_pools != current->Flink)
		{
			current = current->Flink;

			// Get the head of the record
			__pool_table* pool_table = (__pool_table*)CONTAINING_RECORD(current, __pool_table, pool_list);

			if (address == pool_table->address)
			{
				RtlSecureZeroMemory(address, pool_table->size);
				pool_table->is_busy = false;
				pool_table->recycled = true;
				break;
			}
		}

		spinlock::unlock(&g_vmm_context.pool_manager->lock_for_reading_pool);
	}

	inline const char* intention_to_string(allocation_intention intention)
	{
		switch (intention)
		{
		case INTENTION_NONE:   return "None";
		case INTENTION_TRACK_HOOKED_PAGES:   return "Track Hooked Pages";
		case INTENTION_EXEC_TRAMPOLINE: return "Trampoline";
		case INTENTION_SPLIT_PML2: return "Split Pml2";
		case INTENTION_TRACK_HOOKED_FUNCTIONS: return "Trace Hooked Functions";
		default:      return "Unknown";
		}
	}

	/// <summary>
	/// Writes all information about allocated pools
	/// </summary>
	void dump_pools_info()
	{
		PLIST_ENTRY current = g_vmm_context.pool_manager->list_of_allocated_pools;

		spinlock::lock(&g_vmm_context.pool_manager->lock_for_reading_pool);

#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] -----------------------------------POOL MANAGER DUMP-----------------------------------");
#endif

		while (g_vmm_context.pool_manager->list_of_allocated_pools != current->Flink)
		{
			current = current->Flink;

			// Get the head of the record
			__pool_table* pool_table = (__pool_table*)CONTAINING_RECORD(current, __pool_table, pool_list);

#if ENABLE_TRACE
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Address: 0x%p    Size: %llu    Intention: %s    Is Busy: %s    Recycled: %s",
				pool_table->address, 
				pool_table->size, 
				intention_to_string(pool_table->intention),
				pool_table->is_busy ? "Yes" : "No",
				pool_table->recycled ? "Yes" : "No");
#endif
		}

#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] -----------------------------------POOL MANAGER DUMP-----------------------------------");
#endif

		spinlock::unlock(&g_vmm_context.pool_manager->lock_for_reading_pool);
	}

}
