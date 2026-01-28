#pragma once

namespace VT_Util
{
	using namespace VT_InterruptHook;
	extern BOOL loadedbydbvm;
	extern int PTESize;
	extern UINT_PTR PAGE_SIZE_LARGE;
	extern UINT_PTR MAX_PDE_POS;
	extern UINT_PTR MAX_PTE_POS;

	extern int Cpu_Stepping;
	extern int Cpu_Model;
	extern int Cpu_FamilyID;
	extern int Cpu_Type;
	extern int Cpu_Ext_ModelID;
	extern int Cpu_Ext_FamilyID;

	extern int KernelCodeStepping;
	extern int KernelWritesIgnoreWP;

#if (NTDDI_VERSION >= NTDDI_VISTA)
	extern PVOID DRMHandle;
	extern PEPROCESS DRMProcess;
	extern PEPROCESS DRMProcess2;
#endif

	typedef enum _CPU_VENDOR
	{
		CPU_Other = 0,
		CPU_Intel,
		CPU_AMD
	} CPU_VENDOR;

	extern CPU_VENDOR Cpu_Vendor;

	typedef struct _CPUID
	{
		int eax;
		int ebx;
		int ecx;
		int edx;
	} CPUID, * PCPUID;

	typedef union _IA32_FEATURE_CONTROL_MSR
	{
		ULONG64 All;
		struct
		{
			ULONG64 Lock : 1;                // [0]
			ULONG64 EnableSMX : 1;           // [1]
			ULONG64 EnableVmxon : 1;         // [2]
			ULONG64 Reserved2 : 5;           // [3-7]
			ULONG64 EnableLocalSENTER : 7;   // [8-14]
			ULONG64 EnableGlobalSENTER : 1;  // [15]
			ULONG64 Reserved3a : 16;         //
			ULONG64 Reserved3b : 32;         // [16-63]
		} Fields;
	} IA32_FEATURE_CONTROL_MSR, * PIA32_FEATURE_CONTROL_MSR;

	struct PTEStruct
	{
		unsigned P : 1; // present (1 = present)
		unsigned RW : 1; // read/write
		unsigned US : 1; // user/supervisor
		unsigned PWT : 1; // page-level write-through
		unsigned PCD : 1; // page-level cache disabled
		unsigned A : 1; // accessed
		unsigned Reserved : 1; // dirty
		unsigned PS : 1; // page size (0 = 4-KB page)
		unsigned G : 1; // global page
		unsigned A1 : 1; // available 1 aka copy-on-write
		unsigned A2 : 1; // available 2/ is 1 when paged to disk
		unsigned A3 : 1; // available 3
		unsigned PFN : 20; // page-frame number
	};

	//typedef struct PTEStruct *PPDPTE;
	//typedef struct PTEStruct *PPDE;
	//typedef struct PTEStruct *PPTE;

	struct PTEStruct64
	{
		unsigned long long P : 1; // present (1 = present)
		unsigned long long RW : 1; // read/write
		unsigned long long US : 1; // user/supervisor
		unsigned long long PWT : 1; // page-level write-through
		unsigned long long PCD : 1; // page-level cache disabled
		unsigned long long A : 1; // accessed
		unsigned long long Reserved : 1; // dirty
		unsigned long long PS : 1; // page size (0 = 4-KB page)
		unsigned long long G : 1; // global page
		unsigned long long A1 : 1; // available 1 aka copy-on-write
		unsigned long long A2 : 1; // available 2/ is 1 when paged to disk
		unsigned long long A3 : 1; // available 3
		unsigned long long PFN : 52; // page-frame number
	};

	//typedef struct PTEStruct64 *PPDPTE_PAE;
	//typedef struct PTEStruct64 *PPDE_PAE;
	//typedef struct PTEStruct64 *PPTE_PAE;

	typedef struct tagDebugregs
	{
		ULONG DR0;
		ULONG DR1;
		ULONG DR2;
		ULONG DR3;
		ULONG DR5;
		ULONG DR6;
		ULONG DR7;
	} Debugregs;

	typedef struct
	{
		unsigned CF : 1; // 0
		unsigned reserved1 : 1; // 1
		unsigned PF : 1; // 2
		unsigned reserved2 : 1; // 3
		unsigned AF : 1; // 4
		unsigned reserved3 : 1; // 5
		unsigned ZF : 1; // 6
		unsigned SF : 1; // 7
		unsigned TF : 1; // 8
		unsigned IF : 1; // 9
		unsigned DF : 1; // 10
		unsigned OF : 1; // 11
		unsigned IOPL : 2; // 12+13
		unsigned NT : 1; // 14
		unsigned reserved4 : 1; // 15
		unsigned RF : 1; // 16
		unsigned VM : 1; // 17
		unsigned AC : 1; // 18
		unsigned VIF : 1; // 19
		unsigned VIP : 1; // 20
		unsigned ID : 1; // 21
		unsigned reserved5 : 10; // 22-31
#ifdef AMD64
		unsigned reserved6 : 8;
		unsigned reserved7 : 8;
		unsigned reserved8 : 8;
		unsigned reserved9 : 8;
#endif
	} EFLAGS, * PEFLAGS;


	typedef struct tagDebugReg7
	{
		unsigned L0 : 1; //			0
		unsigned G0 : 1; //			1
		unsigned L1 : 1; //			2
		unsigned G1 : 1; //			3
		unsigned L2 : 1; //			4
		unsigned G2 : 1; //			5
		unsigned L3 : 1; //			6
		unsigned G3 : 1; //			7
		unsigned GL : 1; //			8
		unsigned GE : 1; //			9
		unsigned undefined_1 : 1; //1       10
		unsigned RTM : 1; //        11
		unsigned undefined_0 : 1; //0       12
		unsigned GD : 1; //		   13
		unsigned undefined2 : 2; // 00 
		unsigned RW0 : 2;
		unsigned LEN0 : 2;
		unsigned RW1 : 2;
		unsigned LEN1 : 2;
		unsigned RW2 : 2;
		unsigned LEN2 : 2;
		unsigned RW3 : 2;
		unsigned LEN3 : 2;
#ifdef AMD64
		unsigned undefined3 : 8;
		unsigned undefined4 : 8;
		unsigned undefined5 : 8;
		unsigned undefined6 : 8;
#endif
	} DebugReg7;

	typedef struct DebugReg6
	{
		unsigned B0 : 1;
		unsigned B1 : 1;
		unsigned B2 : 1;
		unsigned B3 : 1;
		unsigned undefined1 : 9; // 011111111
		unsigned BD : 1;
		unsigned BS : 1;
		unsigned BT : 1;
		unsigned RTM : 1; //0=triggered
		unsigned undefined2 : 15; // 111111111111111
#ifdef AMD64
		unsigned undefined3 : 8;
		unsigned undefined4 : 8;
		unsigned undefined5 : 8;
		unsigned undefined6 : 8;
#endif
	} DebugReg6;

#pragma pack(2) //allignment of 2 bytes
	typedef struct tagGDT
	{
		WORD wLimit;
		PVOID vector;
	} GDT, * PGDT;
#pragma pack()

	typedef VOID F(UINT_PTR param);
	typedef F* PF;

	typedef VOID PREDPC_CALLBACK(CCHAR cpunr, PKDEFERRED_ROUTINE Dpc, PVOID DeferredContext, PVOID* SystemArgument1, PVOID* SystemArgument2);

	typedef PREDPC_CALLBACK* PPREDPC_CALLBACK;

	typedef struct _criticalSection
	{
		LONG locked;
		int cpunr; //unique id for a cpu
		int lockcount;
		int oldIFstate;
	} criticalSection, * PcriticalSection;



	void InitVT();
	void UnInitVT();

	BOOLEAN HvmIsSuported();

	void forEachCpuPassive(PF f, UINT_PTR param);
	void forOneCpu(CCHAR cpunr, PKDEFERRED_ROUTINE dpcfunction, PVOID DeferredContext, PVOID  SystemArgument1, PVOID  SystemArgument2, OPTIONAL PPREDPC_CALLBACK preDPCCallback);
	void forEachCpu(PKDEFERRED_ROUTINE dpcfunction, PVOID DeferredContext, PVOID  SystemArgument1, PVOID  SystemArgument2, OPTIONAL PPREDPC_CALLBACK preDPCCallback);
	void forEachCpuAsync(PKDEFERRED_ROUTINE dpcfunction, PVOID DeferredContext, PVOID  SystemArgument1, PVOID  SystemArgument2, OPTIONAL PPREDPC_CALLBACK preDPCCallback);

	NTSTATUS UtilForEachProcessor(NTSTATUS(*callback_routine)(void*), void* context);
	NTSTATUS UtilForEachProcessorDpc(PKDEFERRED_ROUTINE deferred_routine, void* context);

	int getCpuCount(void);

	int IsPrefix(unsigned char b);

	int cpunr(void);

	int GenerateCRC(unsigned char* ptr, int size);

	EFLAGS getEflags(void);

	UINT64 readMSR(DWORD msr);

	UINT64 getDR7(void);

	void setCR0(UINT64 newcr0);

	UINT64 getCR0(void);

	UINT64 getCR2(void);

	void setCR3(UINT64 newCR3);

	UINT64 getCR3(void);

	void setCR4(UINT64 newcr4);

	UINT64 getCR4(void);

	UINT64 getTSC(void);

	void GetIDT(PIDT pIdt);

	void enableInterrupts(void);

	void disableInterrupts(void);

	void csEnter(PcriticalSection CS);

	void csLeave(PcriticalSection CS);


#ifdef AMD64
	extern "C" WORD getCS(void);
	extern "C" WORD getSS(void);
	extern "C" WORD getDS(void);
	extern "C" WORD getES(void);
	extern "C" WORD getFS(void);
	extern "C" WORD getGS(void);
	extern "C" UINT64 getRSP(void);
	extern "C" UINT64 getRBP(void);
	extern "C" UINT64 getRAX(void);
	extern "C" UINT64 getRBX(void);
	extern "C" UINT64 getRCX(void);
	extern "C" UINT64 getRDX(void);
	extern "C" UINT64 getRSI(void);
	extern "C" UINT64 getRDI(void);
#else
	WORD getCS(void);
	WORD getSS(void);
	WORD getDS(void);
	WORD getES(void);
	WORD getFS(void);
	WORD getGS(void);
	ULONG GetRSP(void);
	ULONG GetRBP(void);
	ULONG GetRAX(void);
	ULONG GetRBX(void);
	ULONG GetRCX(void);
	ULONG GetRDX(void);
	ULONG GetRSI(void);
	ULONG GetRDI(void);
#endif

	extern "C" UINT64 getR8(void);
	extern "C" UINT64 getR9(void);
	extern "C" UINT64 getR10(void);
	extern "C" UINT64 getR11(void);
	extern "C" UINT64 getR12(void);
	extern "C" UINT64 getR13(void);
	extern "C" UINT64 getR14(void);
	extern "C" UINT64 getR15(void);

	extern "C" UINT64 getAccessRightsAsm(UINT64 segment);
	extern "C" UINT64 getSegmentLimitAsm(UINT64 segment);

#ifdef AMD64
	extern "C" void FxsaveAsm(volatile void*);
	extern "C" void GetGDT(PGDT pGdt);
	extern "C" WORD getLDT();
	extern "C" WORD getTR(void);
#else
	void GetGDT(PGDT pGdt);
	WORD getLDT();
	WORD getTR(void);
#endif

	void CreateRemoteAPC(ULONG threadid, PVOID addresstoexecute);

}

