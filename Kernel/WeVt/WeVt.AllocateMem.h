#pragma once

#ifndef VMM_TAG
#define VMM_TAG 'vtmm'
#endif

// VMXON/VMCS/EPT paging structures require 4KB alignment.
// WeCheat kernel malloc() is 16-byte aligned and cannot satisfy this.
// ExAllocatePoolWithTag page-aligns allocations of PAGE_SIZE or greater.
template <typename T>
inline T allocate_aligned_pool(unsigned __int64 size)
{
	if (size < PAGE_SIZE)
	{
		size = PAGE_SIZE;
	}

	return (T)ExAllocatePoolWithTag(NonPagedPool, size, VMM_TAG);
}

template <typename T>
inline T* allocate_aligned_pool()
{
	return allocate_aligned_pool<T*>(sizeof(T));
}

inline void free_aligned_pool(void* pool_address)
{
	if (pool_address != nullptr)
	{
		ExFreePoolWithTag(pool_address, VMM_TAG);
	}
}
