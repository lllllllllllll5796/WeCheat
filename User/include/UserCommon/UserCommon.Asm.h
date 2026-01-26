#pragma once

extern "C" uintptr_t vmcall_intel(uint64_t Password3, uint64_t Password1, void* pVMCallInfo);
extern "C" uintptr_t vmcall_amd(uint64_t Password3, uint64_t Password1, void* pVMCallInfo);
extern "C" void RunWithKernelStack(void* pThis, void* pFunc);
extern "C" void* FuckStack_Stub();
extern "C" void Asm_Vf_Pending_DB(bool& HvCheck);