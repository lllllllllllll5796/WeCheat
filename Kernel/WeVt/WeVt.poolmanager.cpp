#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"

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

}
