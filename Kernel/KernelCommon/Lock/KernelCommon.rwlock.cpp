#include "kernelCommon.pch.h"
#include "kernelcommon.rwlock.h"

// ENTER_***_REGION are callable from IRQL <= APC_LEVEL

// Disables delivery of user and normal kernel APC's,
// except special kernel APCs:
#define ENTER_CRITICAL_REGION() KeEnterCriticalRegion()
#define LEAVE_CRITICAL_REGION() KeLeaveCriticalRegion()

// Disables delivery of all APCs, including special kernel:
#define ENTER_GUARDED_REGION() KeEnterGuardedRegion()
#define LEAVE_GUARDED_REGION() KeLeaveGuardedRegion()

// CRWLock
void
rwlock::SetEnableUserAndNormalKernalAPC(bool set)
{
	EnableUserAndNormalKernalAPC = set;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
rwlock::rwlock() : Resource({})
{
	ExInitializeResourceLite(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
rwlock::Reinitialize()
{
	return ExReinitializeResourceLite(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
rwlock::~rwlock()
{
	ExDeleteResourceLite(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
rwlock::LockShared(BOOLEAN Wait /*= TRUE*/)
{
	if (!EnableUserAndNormalKernalAPC)
	{
		ENTER_CRITICAL_REGION();
	}

	return ExAcquireResourceSharedLite(&Resource, Wait);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
rwlock::LockExclusive(BOOLEAN Wait /*= TRUE*/)
{
	if (!EnableUserAndNormalKernalAPC)
	{
		ENTER_CRITICAL_REGION();
	}

	return ExAcquireResourceExclusiveLite(&Resource, Wait);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
rwlock::Unlock()
{
	ExReleaseResourceLite(&Resource);
	if (!EnableUserAndNormalKernalAPC)
	{
		LEAVE_CRITICAL_REGION();
	}
}

_IRQL_requires_max_(DISPATCH_LEVEL)
ULONG
rwlock::GetOwnersCount()
{
	return ExIsResourceAcquiredLite(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
ULONG
rwlock::GetSharedOwnersCount()
{
	return ExIsResourceAcquiredSharedLite(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
rwlock::IsAcquired()
{
	return static_cast<BOOLEAN>(GetOwnersCount() > 0);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
rwlock::IsAcquiredShared()
{
	return static_cast<BOOLEAN>(GetSharedOwnersCount() > 0);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
rwlock::IsAcquiredExclusive()
{
	return ExIsResourceAcquiredExclusiveLite(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
ULONG
rwlock::GetSharedWaiters()
{
	return ExGetSharedWaiterCount(&Resource);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
ULONG
rwlock::GetExclusiveWaiters()
{
	return ExGetExclusiveWaiterCount(&Resource);
}

// Converts exclusive lock to shared if current thread locked
// ERESOURCE with exclusive access:
_IRQL_requires_max_(APC_LEVEL)
VOID
rwlock::ConvertExclusiveToShared()
{
	ExConvertExclusiveToSharedLite(&Resource);
}