#pragma once

extern "C" uintptr_t vmcall_intel(uint64_t Password3, uint64_t Password1, void* pVMCallInfo);
extern "C" uintptr_t vmcall_amd(uint64_t Password3, uint64_t Password1, void* pVMCallInfo);
extern "C" void RunWithKernelStack(void* pThis, void* pFunc);

#pragma pack(push, 1)


struct ChangeRegOnBPInfo
{
	struct
	{
		bool changeRAX : 1;			//0
		bool changeRBX : 1;			//1
		bool changeRCX : 1;			//2
		bool changeRDX : 1;         //3
		bool changeRSI : 1;         //4
		bool changeRDI : 1;         //5
		bool changeRBP : 1;         //6
		bool changeRSP : 1;         //7
		bool changeRIP : 1;         //8
		bool changeR8 : 1;          //9
		bool changeR9 : 1;          //10
		bool changeR10 : 1;         //11
		bool changeR11 : 1;         //12
		bool changeR12 : 1;         //13
		bool changeR13 : 1;         //14
		bool changeR14 : 1;         //15
		bool changeR15 : 1;         //16
		bool changeCF : 1;          //17
		bool changePF : 1;          //18
		bool changeAF : 1;          //19
		bool changeZF : 1;          //20
		bool changeSF : 1;          //21
		bool changeOF : 1;          //22
		bool newCF : 1;             //23
		bool newPF : 1;             //24
		bool newAF : 1;             //25
		bool newZF : 1;             //26
		bool newSF : 1;             //27
		bool newOF : 1;             //28
		bool reserved : 3;          //29,30,31
	};

	struct
	{
		bool changeXMM0_0 : 1, changeXMM0_1 : 1, changeXMM0_2 : 1, changeXMM0_3 : 1;
		bool changeXMM1_0 : 1, changeXMM1_1 : 1, changeXMM1_2 : 1, changeXMM1_3 : 1;
		bool changeXMM2_0 : 1, changeXMM2_1 : 1, changeXMM2_2 : 1, changeXMM2_3 : 1;
		bool changeXMM3_0 : 1, changeXMM3_1 : 1, changeXMM3_2 : 1, changeXMM3_3 : 1;
		bool changeXMM4_0 : 1, changeXMM4_1 : 1, changeXMM4_2 : 1, changeXMM4_3 : 1;
		bool changeXMM5_0 : 1, changeXMM5_1 : 1, changeXMM5_2 : 1, changeXMM5_3 : 1;
		bool changeXMM6_0 : 1, changeXMM6_1 : 1, changeXMM6_2 : 1, changeXMM6_3 : 1;
		bool changeXMM7_0 : 1, changeXMM7_1 : 1, changeXMM7_2 : 1, changeXMM7_3 : 1;
		bool changeXMM8_0 : 1, changeXMM8_1 : 1, changeXMM8_2 : 1, changeXMM8_3 : 1;
		bool changeXMM9_0 : 1, changeXMM9_1 : 1, changeXMM9_2 : 1, changeXMM9_3 : 1;
		bool changeXMM10_0 : 1, changeXMM10_1 : 1, changeXMM10_2 : 1, changeXMM10_3 : 1;
		bool changeXMM11_0 : 1, changeXMM11_1 : 1, changeXMM11_2 : 1, changeXMM11_3 : 1;
		bool changeXMM12_0 : 1, changeXMM12_1 : 1, changeXMM12_2 : 1, changeXMM12_3 : 1;
		bool changeXMM13_0 : 1, changeXMM13_1 : 1, changeXMM13_2 : 1, changeXMM13_3 : 1;
		bool changeXMM14_0 : 1, changeXMM14_1 : 1, changeXMM14_2 : 1, changeXMM14_3 : 1;
		bool changeXMM15_0 : 1, changeXMM15_1 : 1, changeXMM15_2 : 1, changeXMM15_3 : 1;
	};

	uint64_t changeFP; //just one bit, each bit is a fpu field
	uint64_t newRAX;
	uint64_t newRBX;
	uint64_t newRCX;
	uint64_t newRDX;
	uint64_t newRSI;
	uint64_t newRDI;
	uint64_t newRBP;
	uint64_t newRSP;
	uint64_t newRIP;
	uint64_t newR8;
	uint64_t newR9;
	uint64_t newR10;
	uint64_t newR11;
	uint64_t newR12;
	uint64_t newR13;
	uint64_t newR14;
	uint64_t newR15;
	uint64_t newFP0;
	uint64_t newFP0_H;
	uint64_t newFP1;
	uint64_t newFP1_H;
	uint64_t newFP2;
	uint64_t newFP2_H;
	uint64_t newFP3;
	uint64_t newFP3_H;
	uint64_t newFP4;
	uint64_t newFP4_H;
	uint64_t newFP5;
	uint64_t newFP5_H;
	uint64_t newFP6;
	uint64_t newFP6_H;
	uint64_t newFP7;
	uint64_t newFP7_H;
	union
	{
		struct { uint64_t uint64_0, uint64_1; };
		struct { double Double_0, Double_1; };
		struct { uint32_t uint32_0, uint32_1, uint32_2, uint32_3; };
		struct { float Float_0, Float_1, Float_2, Float_3; };
	} XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15;
};

class DBVM
{
public:
	DBVM();
	~DBVM();
public:
	//需要初始化
	std::function<BOOL(IN HANDLE ProcessId, IN PVOID VirtualAddress, OUT ULONG64& PhysicalAddress)> GetPhysicalAddress;
	std::function<BOOL(IN HANDLE ProcessId, IN PVOID pRemoteAddress, IN size_t Size)> TriggerCOW_ByProcessId;
	std::function<BOOL(IN HANDLE ProcessId, IN OUT PVOID* BaseAddress, IN OUT PSIZE_T RegionSize, IN ULONG LockOption)> LockVirtualMemory;
	std::function<BOOL(IN HANDLE ProcessId, IN OUT PVOID* BaseAddress, IN OUT PSIZE_T RegionSize, IN ULONG LockOption)> UnLockVirtualMemory;
	std::function<BOOL(IN HANDLE ProcessId, OUT ULONG64& CR3)> GetCR3;
	std::function<BOOL(IN HANDLE ProcessId, IN PVOID BaseAddress, OUT PVOID	Buffer, IN SIZE_T NumberOfBytesToRead, OUT PSIZE_T NumberOfBytesReaded)> ReadMemoryByProcessId;

	bool IsIntel() const;

	bool IsAMD() const;

	//此函数会抛异常,需要Try住
	uint32_t GetVersion() const;

	bool ModifyRegisterOnExecute(HANDLE PID, uintptr_t VirtualAddress, ModifyRegisterInfo& Info, bool IsWinApi);
	bool UnModifyRegisterOnExecute(HANDLE PID, uintptr_t VirtualAddress);

	bool ModifyCodeOnExecute(HANDLE PID, uintptr_t VirtualAddress, const void* Buffer, size_t Size, bool IsWinApi);
	bool UnModifyCodeOnExecute(HANDLE PID, uintptr_t VirtualAddress, size_t Size);

	bool WPMHideWrapper(HANDLE PID, BOOL bRemoteAddressIsWinApi, void* pRemoteAddress, const void* pBuffer, size_t Size, std::vector<uint8_t>& OriginalBytes, std::function<void()> CallBackFunc) const;

private:
	//std::unordered_map<uintptr_t, std::pair<ULONG64, bool>> Va2PaMap;
};

#pragma pack(pop)