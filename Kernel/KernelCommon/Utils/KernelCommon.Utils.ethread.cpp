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

			if (ImpCall(PsLookupThreadByThreadId, ThreadId, &Thread) == STATUS_SUCCESS)
			{
				Result = Thread;
				ImpCall(ObfDereferenceObject, Thread);
			}
			return Result;
		}
	}
}