#pragma once

namespace NoExceptions
{
	typedef struct
	{
		KIRQL entryIRQL;
		VT_InterruptHook::PINT_VECTOR NoExceptionVectorList; //list pointing to an idt table with hooked ints
		VT_InterruptHook::IDT OriginalIDT;
		VT_InterruptHook::IDT ModdedIDT;
	} CPUSTATE, * PCPUSTATE;

	BOOL NoExceptions_Enter();

	int  NoExceptions_CopyMemory(PVOID Destination, PVOID Source, int size);

	void NoExceptions_Leave();

	void NoExceptions_Cleanup();

	//void NoException14_ErrorHandler();
}

