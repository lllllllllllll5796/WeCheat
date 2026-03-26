#include "WeVt.pch.h"
#include "WeVt.poolmanager.h"
#include "WeVt.HypervisorGlobals.h"

namespace pool_manager
{
	void request_pool_x(allocation_intention intention, PLIST_ENTRY current, __pool_table* pool_table, void* address)
	{
		bool is_recycled = false;

		current = g_vmm_context.pool_manager->list_of_allocated_pools;

		spinlock::lock(&g_vmm_context.pool_manager->lock_for_reading_pool);

		while (g_vmm_context.pool_manager->list_of_allocated_pools != current->Flink)
		{
			current = current->Flink;

			// Get the head of the record
			pool_table = (__pool_table*)CONTAINING_RECORD(current, __pool_table, pool_list);

			if (pool_table->intention == intention && pool_table->is_busy == false)
			{
				pool_table->is_busy = true;
				is_recycled = pool_table->recycled;
				address = pool_table->address;
				break;
			}
		}

		spinlock::unlock(&g_vmm_context.pool_manager->lock_for_reading_pool);
	}

}
