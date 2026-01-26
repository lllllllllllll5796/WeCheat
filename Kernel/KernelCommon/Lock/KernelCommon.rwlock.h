#pragma once

class rwlock
{
private:
	ERESOURCE Resource;
	bool EnableUserAndNormalKernalAPC = false;

public:
	rwlock(const rwlock&) = delete;
	rwlock(rwlock&&) = delete;
	rwlock& operator=(const rwlock&) = delete;
	rwlock& operator=(rwlock&&) = delete;

public:
	void SetEnableUserAndNormalKernalAPC(bool set);
	_IRQL_requires_max_(DISPATCH_LEVEL)
		rwlock();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		NTSTATUS Reinitialize();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		~rwlock();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		BOOLEAN LockShared(BOOLEAN Wait = TRUE);

	_IRQL_requires_max_(DISPATCH_LEVEL)
		BOOLEAN LockExclusive(BOOLEAN Wait = TRUE);

	_IRQL_requires_max_(DISPATCH_LEVEL)
		VOID Unlock();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		ULONG GetOwnersCount();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		ULONG GetSharedOwnersCount();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		BOOLEAN IsAcquired();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		BOOLEAN IsAcquiredShared();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		BOOLEAN IsAcquiredExclusive();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		ULONG GetSharedWaiters();

	_IRQL_requires_max_(DISPATCH_LEVEL)
		ULONG GetExclusiveWaiters();

	// Converts exclusive lock to shared if current thread locked
	// ERESOURCE with exclusive access:
	_IRQL_requires_max_(APC_LEVEL)
		VOID ConvertExclusiveToShared();
};
