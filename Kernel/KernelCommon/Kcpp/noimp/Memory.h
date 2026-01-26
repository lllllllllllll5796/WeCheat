#pragma once

#pragma pack(push, 1)
typedef union VIRT_ADDR_
{
	u64 value;
	void* pointer;

	struct {
		u64 offset : 12;
		u64 pt_index : 9;
		u64 pd_index : 9;
		u64 pdpt_index : 9;
		u64 pml4_index : 9;
		u64 reserved : 16;
	} s;

	struct {
		u64 offset : 21;
		u64 pd_index : 9;
		u64 pdpt_index : 9;
		u64 pml4_index : 9;
		u64 reserved : 16;
	} l;
} VIRT_ADDR;

typedef union PTE_
{
	u64 value;

	struct
	{
		u64 Valid : 1;                                           
		u64 Dirty1 : 1;                                          
		u64 Owner : 1;                                           
		u64 WriteThrough : 1;                                    
		u64 CacheDisable : 1;                                    
		u64 Accessed : 1;                                        
		u64 Dirty : 1;                                           
		u64 LargePage : 1;                                       
		u64 Global : 1;                                          
		u64 CopyOnWrite : 1;                                     
		u64 Unused : 1;                                          
		u64 Write : 1;                                           
		u64 PageFrameNumber : 36; 
		u64 ReservedForHardware : 4;
		u64 ReservedForSoftware : 4;                             
		u64 WsleAge : 4;                                         
		u64 WsleProtection : 3;
		u64 NoExecute : 1;                         
	};
} PTE;
#pragma pack(pop)

__declspec(selectany) u64 g_pxe_base;
__declspec(selectany) u64 g_ppe_base;
__declspec(selectany) u64 g_pde_base;
__declspec(selectany) u64 g_pte_base;

BOOL InitializePteBase(u64 dirbase);

#define PTE_SHIFT 3
#define PTI_SHIFT 12
#define PDI_SHIFT 21
#define PPI_SHIFT 30
#define PXI_SHIFT 39
#define PXE_PER_PAGE 512
#define VIRTUAL_ADDRESS_BITS 48
#define PXI_MASK (PXE_PER_PAGE - 1)
#define VIRTUAL_ADDRESS_MASK ((((ULONG_PTR)1) << VIRTUAL_ADDRESS_BITS) - 1)

#define MiGetPxeOffset(va) \
    ((ULONG)(((ULONG_PTR)(va) >> PXI_SHIFT) & PXI_MASK))

#define MiGetPxeAddress(va)   \
    ((PTE*)g_pxe_base + MiGetPxeOffset(va))

#define MiGetPpeAddress(va)   \
    ((PTE*)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + g_ppe_base))

#define MiGetPdeAddress(va) \
    ((PTE*)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + g_pde_base))

#define MiGetPteAddress(va) \
    ((PTE*)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + g_pte_base))


DECLSPEC_NOINLINE
bool FixPage(PVOID addr);

bool IsAddrValid(u64 addr, bool fixPage = false);

bool IsExec(u64 addr);

ULONG64 GetPXEAddress(PVOID addr);
ULONG64 GetPPEAddress(PVOID addr);
ULONG64 GetPDEAddress(PVOID addr);
ULONG64 GetPTEAddress(PVOID addr);

typedef struct _MMPTE_HARDWARE            // 18 elements, 0x8 bytes (sizeof)
{
	/*0x000*/     UINT64       Valid : 1;               // 0 BitPosition
	/*0x000*/     UINT64       Dirty1 : 1;              // 1 BitPosition
	/*0x000*/     UINT64       Owner : 1;               // 2 BitPosition
	/*0x000*/     UINT64       WriteThrough : 1;        // 3 BitPosition
	/*0x000*/     UINT64       CacheDisable : 1;        // 4 BitPosition
	/*0x000*/     UINT64       Accessed : 1;            // 5 BitPosition
	/*0x000*/     UINT64       Dirty : 1;               // 6 BitPosition
	/*0x000*/     UINT64       LargePage : 1;           // 7 BitPosition
	/*0x000*/     UINT64       Global : 1;              // 8 BitPosition
	/*0x000*/     UINT64       CopyOnWrite : 1;         // 9 BitPosition
	/*0x000*/     UINT64       Unused : 1;              // 10 BitPosition
	/*0x000*/     UINT64       Write : 1;               // 11 BitPosition
	/*0x000*/     UINT64       PageFrameNumber : 36;    // 12 BitPosition
	/*0x000*/     UINT64       ReservedForHardware : 4; // 48 BitPosition
	/*0x000*/     UINT64       ReservedForSoftware : 4; // 52 BitPosition
	/*0x000*/     UINT64       WsleAge : 4;             // 56 BitPosition
	/*0x000*/     UINT64       WsleProtection : 3;      // 60 BitPosition
	/*0x000*/     UINT64       NoExecute : 1;           // 63 BitPosition
}MMPTE_HARDWARE, * PMMPTE_HARDWARE;
typedef struct _MMPTE         // 1 elements, 0x8 bytes (sizeof)
{
	/*0x000*/     union {
		MMPTE_HARDWARE  Hard;
		ULONG64         Long;
	} u; // 9 elements, 0x8 bytes (sizeof)
}MMPTE, * PMMPTE;

VOID SetMemoryExecute(ULONG64 Address, ULONG tSize);

u64 ScanUserMemory(u64(*p)(u64));
