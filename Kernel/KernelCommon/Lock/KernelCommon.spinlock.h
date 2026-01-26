#pragma once

namespace spinlock
{
	bool try_lock(volatile long* lock);
	void lock(volatile long* lock);
	void unlock(volatile long* lock);
}