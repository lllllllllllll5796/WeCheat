#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.ethread.h"

namespace KernelCommon
{
	namespace Utils
	{
		PETHREAD GetEThreadByThreadId(HANDLE ThreadId)
		{
			PETHREAD Thread = NULL;
			PETHREAD Result = NULL;

			if (PsLookupThreadByThreadId(ThreadId, &Thread) == STATUS_SUCCESS)
			{
				Result = Thread;
				ObfDereferenceObject(Thread);
			}
			return Result;
		}
	}
}