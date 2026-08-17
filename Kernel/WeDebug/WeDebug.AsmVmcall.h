#pragma once

EXTERN_C
{
	bool __vm_call(unsigned __int64 vmcall_reason, unsigned __int64 rdx, unsigned __int64 r8, unsigned __int64 r9);
	bool __vm_call_ex(unsigned __int64 vmcall_reason, unsigned __int64 rdx, unsigned __int64 r8, unsigned __int64 r9,
		unsigned __int64 r10, unsigned __int64 r11, unsigned __int64 r12, unsigned __int64 r13,
		unsigned __int64 r14, unsigned __int64 r15);

	unsigned __int64 WeDebug_readcr3(void);
	void WeDebug_cli(void);
	void WeDebug_sti(void);
}
