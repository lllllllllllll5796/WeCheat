#include "UserCommon.pch.h"
#include "UserCommon.DBVM.h"

inline void cpuid(int CPUInfo[4], int InfoType)
{
	__asm __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType));
}

inline void cpuidex(int CPUInfo[4], int InfoType, int ECXValue)
{
	__asm __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType), "c" (ECXValue));
}

inline void writecr3(unsigned __int64 __cr3_val)
{
	__asm __volatile__("mov %0, %%cr3" : : "r"(__cr3_val) : "memory");
}

inline unsigned __int64 readmsr(unsigned long __register)
{
	unsigned long __edx;
	unsigned long __eax;
	__asm __volatile__("rdmsr" : "=d"(__edx), "=a"(__eax) : "c"(__register) : "memory");
	return (((unsigned __int64)__edx) << 32) | (unsigned __int64)__eax;
}

inline void writemsr(unsigned long __register, unsigned __int64 val)
{
	unsigned long __edx = (unsigned long)(((unsigned __int64)val) << 32);
	unsigned long __eax = (unsigned long)val;
	__asm __volatile__("wrmsr" : : "a"(__eax), "d"(__edx), "c"(__register) : "memory");
	//return (((unsigned __int64)__edx) << 32) | (unsigned __int64)__eax;
}

DBVM::DBVM()
{

}

DBVM::~DBVM()
{

}

bool DBVM::IsIntel()
{
	int info[4];
	cpuid(info, 0);
	if (info[1] == 0x756e6547 && info[3] == 0x49656e69 && info[2] == 0x6c65746e)
		return true;
	return false;
}

bool DBVM::IsAMD()
{
	int info[4];
	cpuid(info, 0);
	if (info[1] == 0x68747541 && info[3] == 0x69746E65 && info[2] == 0x444D4163)
		return true;
	return false;
}

bool DBVM::IsCapable()
{
	int info[4];
	if (IsIntel())
	{
		cpuid(info, 1);
		int c = info[2];
		if ((c >> 5) & 1)
			return true;
	}
	else if (IsAMD())
	{
		cpuid(info, 0x80000001);
		int c = info[2];
		if ((c >> 2) & 1)
			return true;
	}
	return false;
}

uint32_t DBVM::GetVersion() const
{
	uint32_t Version = 0;
	Version = (uint32_t)VMCall(VMCALL_GETVERSION);

	if ((Version >> 24) != 0xCE)
		return 0;

	return Version & 0xFFFFFF;
}

uintptr_t DBVM::GetMemory() const
{
	return VMCall(VMCALL_GETMEM);
}

uintptr_t DBVM::SwitchToKernelMode(uintptr_t Rip, uintptr_t Param) const
{
	return VMCall(VMCALL_SWITCH_TO_KERNELMODE, (uint32_t)KernelCS, (uint64_t)Rip, (uint64_t)Param);
}

uint64_t DBVM::SwitchToKernelMode() const
{
	return VMCall(VMCALL_KERNELMODE, (uint16_t)KernelCS);
}

void DBVM::ReturnToUserMode() const
{
	VMCall(VMCALL_USERMODE);
}

CR3 DBVM::GetCR3() const
{
	return VMCall(VMCALL_GETCR3);
}

void DBVM::SetCR3(CR3 cr3) const
{
	SwitchToKernelMode();
	writecr3(cr3);
	ReturnToUserMode();
}

uint64_t DBVM::GetCR4() const
{
	return VMCall(VMCALL_GETCR4);
}

uint64_t DBVM::ReadMSR(uint32_t MSR) const
{
	SwitchToKernelMode();
	const uint64_t Result = readmsr(MSR);
	ReturnToUserMode();
	return Result;
	//return VMCall(VMCALL_READMSR, MSR, uint64_t(0));
}

void DBVM::WriteMSR(uint32_t MSR, uint64_t Value) const
{
	SwitchToKernelMode();
	writemsr(MSR, Value);
	ReturnToUserMode();
	//VMCall(VMCALL_WRITEMSR, MSR, Value);
}

bool DBVM::ChangeRegisterOnBP(PhysicalAddress PABase, const ChangeRegOnBPInfo& changeregonbpinfo) const
{
	//OutputDebugStringEx("ChangeRegisterOnBP\r\n");
	CloakActivate(PABase);
	return VMCall(VMCALL_CLOAK_CHANGEREGONBP, PABase, changeregonbpinfo) == 0;
}

bool DBVM::RemoveChangeRegisterOnBP(PhysicalAddress PABase) const
{
	return VMCall(VMCALL_CLOAK_REMOVECHANGEREGONBP, PABase) == 0;
}

bool DBVM::CloakWriteOriginal(PhysicalAddress PABase, const void* Src) const
{
	return VMCall(VMCALL_CLOAK_WRITEORIGINAL, PABase, Src) == 0;
}

bool DBVM::CloakReadOriginal(PhysicalAddress PABase, void* Dst) const
{
	return VMCall(VMCALL_CLOAK_READORIGINAL, PABase, Dst) == 0;
}

void DBVM::CloakActivate(PhysicalAddress PABase, uintptr_t Mode) const
{
	//1 already clocked
	//0 success
	VMCall(VMCALL_CLOAK_ACTIVATE, PABase, Mode);
}

void DBVM::CloakDeactivate(PhysicalAddress PABase) const
{
	//0 success
	VMCall(VMCALL_CLOAK_DEACTIVATE, PABase);
}

void DBVM::CloakReset() const
{
	VMCall(VMCALL_EPT_RESET);
}

void DBVM::HideDBVM() const
{
	VMCall(VMCALL_HIDEDBVMPHYSICALADDRESSESALL);
}

void DBVM::ChangePassword(uint64_t password1, uint32_t password2, uint64_t password3)
{
	VMCall(VMCALL_CHANGEPASSWORD, password1, password2, password3);
	SetPassword(password1, password2, password3);
}

void DBVM::SetPassword(uint64_t password1, uint32_t password2, uint64_t password3)
{
	current_password1 = password1;
	current_password2 = password2;
	current_password3 = password3;
}

void DBVM::GetPassword(uint64_t& password1, uint32_t& password2, uint64_t& password3) const
{
	password1 = current_password1;
	password2 = current_password2;
	password3 = current_password3;
}

void DBVM::SetDefaultPassword()
{
	current_password1 = default_password1;
	current_password2 = default_password2;
	current_password3 = default_password3;
}

PhysicalAddress DBVM::GetPTEAddress(uintptr_t VirtualAddress, CR3 cr3) const
{
	return PhysicalMemory::GetPTEAddress(VirtualAddress, cr3, ReadPhysicalMemory);
}

PhysicalAddress DBVM::GetPhysicalAddress(uintptr_t VirtualAddress, CR3 cr3) const
{
	return PhysicalMemory::GetPhysicalAddress(VirtualAddress, cr3, ReadPhysicalMemory);
}

bool DBVM::RPM(uintptr_t Address, void* Buffer, size_t Size, CR3 cr3) const
{
	return PhysicalMemory::ReadProcessMemory(Address, Buffer, Size, cr3, ReadPhysicalMemory);
}

bool DBVM::WPM(uintptr_t Address, const void* Buffer, size_t Size, CR3 cr3) const
{
	return PhysicalMemory::WriteProcessMemory(Address, Buffer, Size, cr3, ReadPhysicalMemory, WritePhysicalMemory);
}

bool DBVM::WPMCloak(uintptr_t VirtualAddress, const void* Buffer, size_t Size, CR3 cr3) const
{
	return PhysicalMemory::WriteProcessMemory(VirtualAddress, Buffer, Size, cr3, ReadPhysicalMemory,
		[&](PhysicalAddress PA, const void* Buffer, size_t Size)
		{
			PhysicalAddress PABase = PA & ~0xFFF;

			//OutputDebugStringEx("VA:0x%llX PA:0x%llX PABase:0x%llX\n", VirtualAddress, (uintptr_t)PA, (uintptr_t)PABase);

			CloakActivate(PABase);

			uint8_t buf[0x1000];               //正好一个页面
			if (!CloakReadOriginal(PABase, buf))
				return false;

			memcpy(buf + PA - PABase, Buffer, Size);

			if (!CloakWriteOriginal(PABase, buf))
				return false;

			return true;
		});
}

bool DBVM::RemoveCloak(uintptr_t VirtualAddress, size_t Size, CR3 cr3) const
{
	return PhysicalMemory::WriteProcessMemory(VirtualAddress, 0, Size, cr3, ReadPhysicalMemory,
		[&](PhysicalAddress PA, const void* Buffer, size_t Size)
		{
			PhysicalAddress PABase = PA & ~0xFFF;

			//OutputDebugStringEx("VA:0x%llX PA:0x%llX PABase:0x%llX\n", VirtualAddress, (uintptr_t)PA, (uintptr_t)PABase);

			CloakDeactivate(PABase);
			return true;
		});
}

bool DBVM::CloakWrapper(uintptr_t Address, const void* Buffer, size_t Size, CR3 cr3, auto f) const
{
	if (!WPMCloak(Address, Buffer, Size, cr3))
		return false;
	f();
	return RemoveCloak(Address, Size, cr3);
}



