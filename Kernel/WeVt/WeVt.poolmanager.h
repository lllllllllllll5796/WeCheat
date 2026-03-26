#pragma once

namespace pool_manager
{
    enum allocation_intention
    {
        INTENTION_NONE,
        INTENTION_TRACK_HOOKED_PAGES,
        INTENTION_EXEC_TRAMPOLINE,
        INTENTION_SPLIT_PML2,
        INTENTION_TRACK_HOOKED_FUNCTIONS
    };

    struct __request_new_allocation
    {
        unsigned __int64 size[10];
        unsigned __int32 count[10];
        allocation_intention intention[10];
    };

    struct __pool_manager
    {
        __request_new_allocation* allocation_requests;
        PLIST_ENTRY list_of_allocated_pools;  //已分配池列表
        volatile long lock_for_request_allocation;
        volatile long lock_for_reading_pool;
        bool is_request_for_allocation_recived;
    };

    struct __pool_table
    {
        void* address;  //缓冲区指针
        unsigned __int64  size;  //缓冲区大小
        allocation_intention intention;  //该内存用于什么
        LIST_ENTRY pool_list;
        bool is_busy;  //内存是否已被使用
        bool recycled; //回收
    };

    /// <summary>
    /// Writes all information about allocated pools
    /// </summary>
    void dump_pools_info();

    /// <summary>
    /// Request allocation
    /// </summary>
    /// <param name="size">Size of pool</param>
    /// <param name="count">Number of pools to allocate</param>
    /// <param name="intention"></param>
    /// <returns></returns>
    bool request_allocation(unsigned __int64 size, unsigned __int32 count, allocation_intention intention);

    /// <summary>
    /// Initalize pool manager struct and preallocate pools
    /// </summary>
    /// <returns> status </returns>
    bool initialize();

    /// <summary>
    /// Free all allocted pools
    /// </summary>
    void uninitialize();

    /// <summary>
    /// Set information that pool is no longer used by anyone and mark as recycled
    /// </summary>
    /// <param name="address"></param>
    void release_pool(void* address);

    /// <summary>
    /// Allocate all requested pools
    /// </summary>
    /// <returns></returns>
    bool perform_allocation();

    void request_pool_x(allocation_intention intention, PLIST_ENTRY current, __pool_table* pool_table, void* address);

    /// <summary>
    /// Returns pre allocated pool and request new one for allocation
    /// </summary>
    /// <param name="intention">Indicates what will be pool used for</param>
    /// <param name="new_pool">If set new pool will (with same properties) be requested to allocate</param>
    /// <param name="size">Only if new_pool is true. Size of new pool</param>
    /// <returns></returns>
    template <typename T>
    T request_pool(allocation_intention intention, bool new_pool, unsigned __int64 size)
    {
        PLIST_ENTRY current = 0;
        void* address = 0;
        bool is_recycled = false;
        __pool_table* pool_table;

        is_recycled = request_pool_x(intention, current, pool_table, address);

        //
        // If pool which we got is recycled then we don't allocate
        // a new one because we don't want to overload memory, If there wasn't any preallocated pool
        // this function will send a request
        //
        if (new_pool == true && is_recycled == false)
            request_allocation(size, 1, intention);

        return (T)address;
    }
}
