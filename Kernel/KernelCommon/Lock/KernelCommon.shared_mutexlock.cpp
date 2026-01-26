#include "kernelCommon.pch.h"
#include "kernelcommon.shared_mutexlock.h"

shared_mutex::shared_mutex() noexcept
{
	FltInitializePushLock(&m_PushLock);
}

shared_mutex::~shared_mutex() noexcept
{
	FltDeletePushLock(&m_PushLock);
}

void
shared_mutex::lock() noexcept
{
	//注意这个API 是WIN10的 TODO：修复WIN7
	FltAcquirePushLockExclusiveEx(&m_PushLock, 0);
}

bool
shared_mutex::try_lock() noexcept
{
	lock();
	return true;
}

void
shared_mutex::unlock() noexcept
{
	FltReleasePushLockEx(&m_PushLock, 0);
}

void
shared_mutex::lock_shared() noexcept
{
	FltAcquirePushLockSharedEx(&m_PushLock, 0);
}

bool
shared_mutex::try_lock_shared() noexcept
{
	lock();
	return true;
}

void
shared_mutex::unlock_shared() noexcept
{
	FltReleasePushLockEx(&m_PushLock, 0);
}

shared_mutex::native_handle_type
shared_mutex::native_handle() noexcept
{
	return &m_PushLock;
}