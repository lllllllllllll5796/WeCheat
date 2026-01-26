#pragma once

#include "UserCommon.DBVM.h"

#pragma pack(push, 1)
struct ModifyRegisterInfo
{
	struct
	{
		bool modify_RAX : 1;			//0
		bool modify_RBX : 1;			//1
		bool modify_RCX : 1;			//2
		bool modify_RDX : 1;            //3
		bool modify_RSI : 1;            //4
		bool modify_RDI : 1;            //5
		bool modify_RBP : 1;            //6
		bool modify_RSP : 1;            //7
		bool modify_RIP : 1;            //8
		bool modify_R8 : 1;             //9
		bool modify_R9 : 1;             //10
		bool modify_R10 : 1;            //11
		bool modify_R11 : 1;            //12
		bool modify_R12 : 1;            //13
		bool modify_R13 : 1;            //14
		bool modify_R14 : 1;            //15
		bool modify_R15 : 1;            //16
		bool modify_CF : 1;             //17
		bool modify_PF : 1;             //18
		bool modify_AF : 1;             //19
		bool modify_ZF : 1;             //20
		bool modify_SF : 1;             //21
		bool modify_OF : 1;             //22
		bool new_CF : 1;                //23
		bool new_PF : 1;                //24
		bool new_AF : 1;                //25
		bool new_ZF : 1;                //26
		bool new_SF : 1;                //27
		bool new_OF : 1;                //28
		bool reserved : 3;              //29,30,31
	};

	struct
	{
		bool modify_XMM0_0 : 1, modify_XMM0_1 : 1, modify_MM0_2 : 1, modify_XMM0_3 : 1;
		bool modify_XMM1_0 : 1, modify_XMM1_1 : 1, modify_MM1_2 : 1, modify_XMM1_3 : 1;
		bool modify_XMM2_0 : 1, modify_XMM2_1 : 1, modify_MM2_2 : 1, modify_XMM2_3 : 1;
		bool modify_XMM3_0 : 1, modify_XMM3_1 : 1, modify_MM3_2 : 1, modify_XMM3_3 : 1;
		bool modify_XMM4_0 : 1, modify_XMM4_1 : 1, modify_MM4_2 : 1, modify_XMM4_3 : 1;
		bool modify_XMM5_0 : 1, modify_XMM5_1 : 1, modify_MM5_2 : 1, modify_XMM5_3 : 1;
		bool modify_XMM6_0 : 1, modify_XMM6_1 : 1, modify_MM6_2 : 1, modify_XMM6_3 : 1;
		bool modify_XMM7_0 : 1, modify_XMM7_1 : 1, modify_MM7_2 : 1, modify_XMM7_3 : 1;
		bool modify_XMM8_0 : 1, modify_XMM8_1 : 1, modify_MM8_2 : 1, modify_XMM8_3 : 1;
		bool modify_XMM9_0 : 1, modify_XMM9_1 : 1, modify_MM9_2 : 1, modify_XMM9_3 : 1;
		bool modify_XMM10_0 : 1, modify_XMM10_1 : 1, modify_MM10_2 : 1, modify_XMM10_3 : 1;
		bool modify_XMM11_0 : 1, modify_XMM11_1 : 1, modify_MM11_2 : 1, modify_XMM11_3 : 1;
		bool modify_XMM12_0 : 1, modify_XMM12_1 : 1, modify_MM12_2 : 1, modify_XMM12_3 : 1;
		bool modify_XMM13_0 : 1, modify_XMM13_1 : 1, modify_MM13_2 : 1, modify_XMM13_3 : 1;
		bool modify_XMM14_0 : 1, modify_XMM14_1 : 1, modify_MM14_2 : 1, modify_XMM14_3 : 1;
		bool modify_XMM15_0 : 1, modify_XMM15_1 : 1, modify_MM15_2 : 1, modify_XMM15_3 : 1;
	};

	uint64_t modify_FP; //just one bit, each bit is a fpu field
	uint64_t new_RAX;
	uint64_t new_RBX;
	uint64_t new_RCX;
	uint64_t new_RDX;
	uint64_t new_RSI;
	uint64_t new_RDI;
	uint64_t new_RBP;
	uint64_t new_RSP;
	uint64_t new_RIP;
	uint64_t new_R8;
	uint64_t new_R9;
	uint64_t new_R10;
	uint64_t new_R11;
	uint64_t new_R12;
	uint64_t new_R13;
	uint64_t new_R14;
	uint64_t new_R15;
	uint64_t new_FP0;
	uint64_t new_FP0_H;
	uint64_t new_FP1;
	uint64_t new_FP1_H;
	uint64_t new_FP2;
	uint64_t new_FP2_H;
	uint64_t new_FP3;
	uint64_t new_FP3_H;
	uint64_t new_FP4;
	uint64_t new_FP4_H;
	uint64_t new_FP5;
	uint64_t new_FP5_H;
	uint64_t new_FP6;
	uint64_t new_FP6_H;
	uint64_t new_FP7;
	uint64_t new_FP7_H;
	union
	{
		struct { uint64_t uint64_0, uint64_1; };
		struct { double Double_0, Double_1; };
		struct { uint32_t uint32_0, uint32_1, uint32_2, uint32_3; };
		struct { float Float_0, Float_1, Float_2, Float_3; };
	} XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15;
};

class Hypervisor
{
public:
	Hypervisor();
	~Hypervisor();

	std::shared_ptr<DBVM> GetDbvm();
public:
	//需要初始化
	std::function<BOOL(IN HANDLE ProcessId, IN PVOID VirtualAddress, OUT ULONG64& PhysicalAddress)> GetPhysicalAddress;
	std::function<BOOL(IN HANDLE ProcessId, IN PVOID pRemoteAddress, IN size_t Size)> TriggerCOW_ByProcessId;
	std::function<BOOL(IN HANDLE ProcessId, IN OUT PVOID* BaseAddress, IN OUT PSIZE_T RegionSize, IN ULONG LockOption)> LockVirtualMemory;
	std::function<BOOL(IN HANDLE ProcessId, IN OUT PVOID* BaseAddress, IN OUT PSIZE_T RegionSize, IN ULONG LockOption)> UnLockVirtualMemory;
	std::function<BOOL(IN HANDLE ProcessId, OUT ULONG64& CR3)> GetCR3;
	std::function<BOOL(IN HANDLE ProcessId, IN PVOID BaseAddress, OUT PVOID	Buffer, IN SIZE_T NumberOfBytesToRead, OUT PSIZE_T NumberOfBytesReaded)> ReadMemoryByProcessId;

	bool ModifyRegisterOnExecute(HANDLE PID, uintptr_t VirtualAddress, ModifyRegisterInfo& Info, bool IsWinApi);
	bool UnModifyRegisterOnExecute(HANDLE PID, uintptr_t VirtualAddress);

	bool ModifyCodeOnExecute(HANDLE PID, uintptr_t VirtualAddress, const void* Buffer, size_t Size, bool IsWinApi);
	bool UnModifyCodeOnExecute(HANDLE PID, uintptr_t VirtualAddress, size_t Size);

	bool WPMHideWrapper(HANDLE PID, BOOL bRemoteAddressIsWinApi, void* pRemoteAddress, const void* pBuffer, size_t Size, std::vector<uint8_t>& OriginalBytes, std::function<void()> CallBackFunc) const;

private:
	std::shared_ptr<DBVM> m_Dbvm = nullptr;
	//std::unordered_map<uintptr_t, std::pair<ULONG64, bool>> Va2PaMap;
};

#pragma pack(pop)