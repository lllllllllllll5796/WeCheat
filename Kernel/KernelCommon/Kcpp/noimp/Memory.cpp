#include "KernelCommon.pch.h"
#include "Memory.h"

BOOL InitializePteBase(u64 dirbase)
{
	PHYSICAL_ADDRESS phAddr; u64 slot = 0;
	auto pfn = PTE{ dirbase }.PageFrameNumber;
	phAddr.QuadPart = pfn << PAGE_SHIFT;
	auto pml4 = (PTE*)MmGetVirtualForPhysical(phAddr);
	while (pml4[slot].PageFrameNumber != pfn) slot++;
	g_pte_base = (slot << 39) + 0xFFFF000000000000;
	g_pde_base = g_pte_base + (slot << 30);
	g_ppe_base = g_pde_base + (slot << 21);
	g_pxe_base = g_ppe_base + (slot << 12);

	if (g_pxe_base && g_ppe_base && g_pde_base && g_pte_base)
	{
// 		LOG_DEBUG("===================== \r\n");
// 		LOG_DEBUG("g_pxe_base: 0x%llX \r\n", g_pxe_base);
// 		LOG_DEBUG("g_ppe_base: 0x%llX \r\n", g_ppe_base);
// 		LOG_DEBUG("g_pde_base: 0x%llX \r\n", g_pde_base);
// 		LOG_DEBUG("g_pte_base: 0x%llX \r\n", g_pte_base);
// 		LOG_DEBUG("===================== \r\n");
		return TRUE;
	}

	return FALSE;
}

DECLSPEC_NOINLINE
bool FixPage(PVOID addr)
{
	typedef NTSTATUS(__fastcall* MmLoadFn)(
		IN  ULONG_PTR FaultStatus,
		IN  PVOID VirtualAddress,
		IN  KPROCESSOR_MODE PreviousMode,
		IN  PVOID TrapInformation);

	static MmLoadFn mmac = 0;
	if (!mmac) {
		mmac = (MmLoadFn)RVA(FindPatternSect2(g_KernelBase, oxorany(".text"), oxorany("E8 ? ? ? ? 8B D8 40 84 ? 74 0C")), 5);
	}

	//sp("fix");

	return NT_SUCCESS(mmac(FALSE, addr, KernelMode, nullptr));
}

bool IsAddrValid(u64 addr, bool fixPage /*= false*/)
{
	//get dirs ptrs
	auto pxe = MiGetPxeAddress(addr);
	auto ppe = MiGetPpeAddress(addr);
	auto pde = MiGetPdeAddress(addr);
	auto pte = MiGetPdeAddress(addr);

	//get, fix pml4e
	if (!pxe->Valid && (!fixPage || !FixPage(pxe)))
		return false;

	//get, fix pdpte
	if (!ppe->Valid && (!fixPage || !FixPage(ppe)))
		return false;

	//get, fix pde
	if (!pde->Valid && (!fixPage || !FixPage(pde)))
		return false;

	//large page //2MB
	if (pde->LargePage)
		return (bool)(!fixPage || /*fix page*/FixPage((pv64)addr));

	//get, fix pte
	if (!pte->Valid && (!fixPage || !FixPage(pte)))
		return false;

	//small page //4KB
	return (bool)(!fixPage || /*fix page*/FixPage((pv64)addr));
}

bool IsExec(u64 addr)
{
	if (!IsAddrValid(addr))
		return false;

	auto pte = MiGetPteAddress(addr);
	return !pte->NoExecute;
}

ULONG64 GetPXEAddress(PVOID addr)
{
	return (ULONG64)(((((ULONG64)addr & 0xffffffffffff) >> 39) << 3) + g_pxe_base);
}

ULONG64 GetPPEAddress(PVOID addr)
{
	return (ULONG64)(((((ULONG64)addr & 0xffffffffffff) >> 30) << 3) + g_ppe_base);
}

ULONG64 GetPDEAddress(PVOID addr)
{
	return (ULONG64)(((((ULONG64)addr & 0xffffffffffff) >> 21) << 3) + g_pde_base);
}

ULONG64 GetPTEAddress(PVOID addr)
{
	return (ULONG64)(((((ULONG64)addr & 0xffffffffffff) >> 12) << 3) + g_pte_base);
}

VOID SetMemoryExecute(ULONG64 Address, ULONG tSize)
{
	PMMPTE pte = NULL;
	for (ULONG_PTR pAdress = Address; pAdress < (ULONG_PTR)Address + tSize; pAdress += PAGE_SIZE) {
		//�޸�PET���ԣ����ڴ��ִ��
		PMMPTE pte = (PMMPTE)GetPDEAddress((PVOID)pAdress);
		if (!pte) {
			break;
		}
		if (!pte->u.Hard.LargePage) {
			pte = (PMMPTE)GetPTEAddress((PVOID)pAdress);
			if (!pte) { break; }
		}
		pte->u.Hard.NoExecute = 0;
	}
}

u64 ScanUserMemory(u64(*p)(u64))
{
	//scan pml4
	for (int a = 0; a < 512; ++a) {
		VIRT_ADDR gg{ 0 }; gg.s.pml4_index = a;
		auto pml4e = MiGetPxeAddress(gg.value);
		if (pml4e->Valid && pml4e->Owner) { //valid & only usermode
			//scan pdpt
			for (int b = 0; b < 512; ++b) {
				gg.s.pdpt_index = b;
				auto pdpte = MiGetPpeAddress(gg.value);
				if (pdpte->Valid && pdpte->Owner) { //valid & only usermode
					//scan pde
					for (int c = 0; c < 512; ++c) {
						gg.s.pd_index = c;
						auto pde = MiGetPdeAddress(gg.value);
						if (pde->Valid && pde->Owner) { //valid & only usermode
							if (pde->LargePage) {
								auto ret = p(gg.value);
								if (ret) return ret;
							}

							else {
								//scan pte
								for (int d = 0; d < 512; ++d) {
									gg.s.pt_index = d;
									auto pte = MiGetPteAddress(gg.value);
									if (pte->Valid && pte->Owner) { //valid & only usermode
										auto ret = p(gg.value);
										if (ret) return ret;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//not found
	return 0;
}
