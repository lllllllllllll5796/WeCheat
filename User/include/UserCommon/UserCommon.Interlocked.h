/******************************************************************************
Module:  Interlocked.h
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/

#pragma once
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////
// Instances of this class will be accessed by multiple threads. So,
// all members of this class (except the constructor and destructor)
// must be thread-safe.

class CInterlocked 
{
public:
	CInterlocked()  { 
		m_lGrdCnt = 0; InitializeCriticalSection(&m_cs);
	}
	~CInterlocked() { DeleteCriticalSection(&m_cs); }

	// IsGuarded is used for debugging
	BOOL IsGuarded() const { return(m_lGrdCnt > 0); }

public:
	class CGuard {
	public:
		CGuard(CInterlocked& rg) : m_rg(rg) { m_rg.Guard(); };
		~CGuard() { m_rg.Unguard(); }

	private:
		CInterlocked& m_rg;
	};

private:
	void Guard()   { EnterCriticalSection(&m_cs); m_lGrdCnt++; }
	void Unguard() { m_lGrdCnt--; LeaveCriticalSection(&m_cs); }

	// Guard/Unguard can only be accessed by the nested CGuard class.
	friend class CInterlocked::CGuard;

private:
	CRITICAL_SECTION m_cs;
	long m_lGrdCnt;   // # of EnterCriticalSection calls
};