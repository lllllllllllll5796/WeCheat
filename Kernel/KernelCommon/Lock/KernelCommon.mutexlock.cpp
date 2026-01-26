#include "kernelCommon.pch.h"
#include "KernelCommon.mutexlock.h"

mutexlock::mutexlock()
{
	m_GuardedMutex = static_cast<::PKGUARDED_MUTEX>(ExAllocatePoolWithTag(NonPagedPool, sizeof(*m_GuardedMutex), 'tgmx'));
	if (!m_GuardedMutex)
	{
		// TODO
		__debugbreak();
	}

	KeInitializeGuardedMutex(m_GuardedMutex);
}

mutexlock::~mutexlock()
{
	if (m_GuardedMutex)
	{
		ExFreePoolWithTag(m_GuardedMutex, 'tgmx');
	}
}

void
mutexlock::lock()
{
	if (m_GuardedMutex)
	{
		KeAcquireGuardedMutex(m_GuardedMutex);
	}
}

bool
mutexlock::try_lock()
{
	if (!m_GuardedMutex)
	{
		return false;
	}

	return (KeTryToAcquireGuardedMutex(m_GuardedMutex) != FALSE ? true : false);
}

void
mutexlock::unlock()
{
	if (m_GuardedMutex)
	{
		KeReleaseGuardedMutex(m_GuardedMutex);
	}
}

mutexlock::native_handle_type
mutexlock::native_handle()
{
	return m_GuardedMutex;
}