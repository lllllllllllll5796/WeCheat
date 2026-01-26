#pragma once

class shared_mutex
{
public:
	using native_handle_type = PEX_PUSH_LOCK;

	shared_mutex() noexcept;
	~shared_mutex() noexcept;
	void lock() noexcept;
	bool try_lock() noexcept;
	void unlock() noexcept;
	void lock_shared() noexcept;
	bool try_lock_shared() noexcept;
	void unlock_shared() noexcept;
	native_handle_type native_handle() noexcept;

private:
	EX_PUSH_LOCK m_PushLock;
};

template <typename T>
using shared_lock = std::shared_lock<T>;

template <typename T>
using unique_lock = std::unique_lock<T>;