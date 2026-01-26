#pragma once

class mutexlock
{
public:
	using native_handle_type = ::PKGUARDED_MUTEX;

	mutexlock();
	~mutexlock();

	void lock();
	bool try_lock();
	void unlock();
	native_handle_type native_handle();

private:
	native_handle_type m_GuardedMutex = nullptr;
};