#include "WeDebug.pch.h"
#include "WeDebug.AsmVmcall.h"

// clang-cl x64：用 GNU 内联汇编发出 VMCALL，避免 WeDebug 的 LLVM 工具集走 MASM。
// 标识符必须与 WeVt.AsmCallset.asm 中的 0xBF5587567C4C830F 一致。

extern "C" bool __vm_call(unsigned __int64 vmcall_reason, unsigned __int64 rdx_val,
	unsigned __int64 r8_val, unsigned __int64 r9_val)
{
	unsigned __int64 result = 0;
	register unsigned __int64 r8 asm("r8") = r8_val;
	register unsigned __int64 r9 asm("r9") = r9_val;

	__asm__ __volatile__(
		"movabs $0xBF5587567C4C830F, %%rax\n\t"
		"vmcall\n\t"
		: "=a"(result)
		: "c"(vmcall_reason), "d"(rdx_val), "r"(r8), "r"(r9)
		: "memory", "cc"
	);

	return result != 0;
}

extern "C" bool __vm_call_ex(unsigned __int64 vmcall_reason, unsigned __int64 rdx_val,
	unsigned __int64 r8_val, unsigned __int64 r9_val,
	unsigned __int64 r10_val, unsigned __int64 r11_val, unsigned __int64 r12_val,
	unsigned __int64 r13_val, unsigned __int64 r14_val, unsigned __int64 r15_val)
{
	unsigned __int64 result = 0;
	register unsigned __int64 r8 asm("r8") = r8_val;
	register unsigned __int64 r9 asm("r9") = r9_val;
	register unsigned __int64 r10 asm("r10") = r10_val;
	register unsigned __int64 r11 asm("r11") = r11_val;
	register unsigned __int64 r12 asm("r12") = r12_val;
	register unsigned __int64 r13 asm("r13") = r13_val;
	register unsigned __int64 r14 asm("r14") = r14_val;
	register unsigned __int64 r15 asm("r15") = r15_val;

	__asm__ __volatile__(
		"movabs $0xBF5587567C4C830F, %%rax\n\t"
		"vmcall\n\t"
		: "=a"(result)
		: "c"(vmcall_reason), "d"(rdx_val), "r"(r8), "r"(r9),
		  "r"(r10), "r"(r11), "r"(r12), "r"(r13), "r"(r14), "r"(r15)
		: "memory", "cc"
	);

	return result != 0;
}

// clang-cl + OLLVM 会把 MSVC intrinsic 变成未解析外部符号，这里用 GNU 汇编实现。
CODE_NOTOBF
extern "C" unsigned __int64 WeDebug_readcr3(void)
{
	unsigned __int64 value = 0;
	__asm__ __volatile__("mov %%cr3, %0" : "=r"(value));
	return value;
}

CODE_NOTOBF
extern "C" void WeDebug_cli(void)
{
	__asm__ __volatile__("cli" ::: "memory");
}

CODE_NOTOBF
extern "C" void WeDebug_sti(void)
{
	__asm__ __volatile__("sti" ::: "memory");
}
