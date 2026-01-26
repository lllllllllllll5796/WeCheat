#include "UserCommon.pch.h"
#include "UserCommon.Hypervisor.h"

#define LOCK_VM_IN_RAM 2
#define LOCK_VM_IN_WORKING_SET 1

Hypervisor::Hypervisor()
{
	m_Dbvm = std::make_shared<DBVM>();
}

Hypervisor::~Hypervisor()
{

}

std::shared_ptr<DBVM> Hypervisor::GetDbvm()
{
	return m_Dbvm;
}


bool Hypervisor::ModifyRegisterOnExecute(HANDLE PID, uintptr_t VirtualAddress, ModifyRegisterInfo& Info, bool IsWinApi)
{
	bool bResult = false;

	if (PID && VirtualAddress)
	{
		if (IsWinApi)
		{
			//触发写拷贝
			if (!TriggerCOW_ByProcessId(PID, (PVOID)VirtualAddress, 0x10))
			{
				return false;
			}
		}

		ULONG64 PhysicalAddress = 0;

		if (GetPhysicalAddress(PID, (PVOID)VirtualAddress, PhysicalAddress))
		{
			LOG_DEBUG("ModifyRegisterOnExecute--->目标地址VA:0x%llX 目标地址PA:%llX\r\n", VirtualAddress, PhysicalAddress);

			if (PhysicalAddress)
			{
				ChangeRegOnBPInfo changeregonbpinfo{};

				memcpy(&changeregonbpinfo, &Info, sizeof(ChangeRegOnBPInfo));

				ULONG64 PABase = PhysicalAddress & ~0xFFF;

				bResult = m_Dbvm->ChangeRegisterOnBP(PABase, changeregonbpinfo);

				return bResult;
			}
		}
	}

	return false;
}

bool Hypervisor::UnModifyRegisterOnExecute(HANDLE PID, uintptr_t VirtualAddress)
{
	bool bResult = false;

	if (PID && VirtualAddress)
	{
		ULONG64 PhysicalAddress = 0;

		if (GetPhysicalAddress(PID, (PVOID)VirtualAddress, PhysicalAddress))
		{
			LOG_DEBUG("UnModifyCodeOnExecute--->目标地址VA:0x%llX 目标地址PA:0x%llX\r\n", VirtualAddress, PhysicalAddress);

			ULONG64 Cr3 = 0;
			if (GetCR3((HANDLE)PID, Cr3))
			{
				ULONG64 PABase = PhysicalAddress & ~0xFFF;

				bResult = m_Dbvm->RemoveChangeRegisterOnBP(PABase);

				return bResult;
			}
		}
	}

	return false;
}

bool Hypervisor::ModifyCodeOnExecute(HANDLE PID, uintptr_t VirtualAddress, const void* Buffer, size_t Size, bool IsWinApi)
{
	bool bResult = false;

	if (PID && VirtualAddress)
	{
		if (IsWinApi)
		{
			//触发写拷贝
			if (!TriggerCOW_ByProcessId(PID, (PVOID)VirtualAddress, Size))
				return false;
		}

		ULONG64 PhysicalAddress = 0;

		PVOID BaseAddress = (PVOID)((ULONG_PTR)VirtualAddress & ~0xFFF);

		SIZE_T RegionSize = 0x1000;

		if (LockVirtualMemory(PID, &BaseAddress, &RegionSize, LOCK_VM_IN_WORKING_SET | LOCK_VM_IN_RAM))  //先注释掉,看看稳定性
		{
			LOG_DEBUG("R3(2):--->VirtualAddress:0x%llX RegionSize:0x%llX\n", VirtualAddress, RegionSize);

			if (GetPhysicalAddress(PID, (PVOID)VirtualAddress, PhysicalAddress))
			{
				LOG_DEBUG("ModifyCodeOnExecute--->目标地址VA:0x%llX 目标地址PA:0x%llX\r\n", VirtualAddress, PhysicalAddress);

				ULONG64 Cr3 = 0;
				if (GetCR3((HANDLE)PID, Cr3))
				{
					LOG_DEBUG("R3(1):--->VirtualAddress:%llX\n", VirtualAddress);

					//在这里调用内核给虚拟地址上一下锁
					//游戏关闭,反复启动,多次以后 挂钩3环，破坏了COW 导致的，必须锁起来

					bResult = m_Dbvm->WPMCloak(VirtualAddress, Buffer, Size, Cr3);

					return bResult;
				}
			}
		}
	}

	return false;
}

bool Hypervisor::UnModifyCodeOnExecute(HANDLE PID, uintptr_t VirtualAddress, size_t Size)
{
	bool bResult = false;

	if (PID && VirtualAddress)
	{
		ULONG64 PhysicalAddress = 0;

		if (GetPhysicalAddress(PID, (PVOID)VirtualAddress, PhysicalAddress))
		{
			LOG_DEBUG("UnModifyCodeOnExecute--->目标地址VA:0x%llX 目标地址PA:0x%llX\r\n", VirtualAddress, PhysicalAddress);

			ULONG64 Cr3 = 0;
			if (GetCR3((HANDLE)PID, Cr3))
			{
				bResult = m_Dbvm->RemoveCloak(VirtualAddress, Size, Cr3);

				return bResult;
			}
		}
	}

	return false;
}

bool Hypervisor::WPMHideWrapper(HANDLE PID, BOOL bRemoteAddressIsWinApi, void* pRemoteAddress, const void* pBuffer, size_t Size, std::vector<uint8_t>& OriginalBytes, std::function<void()> CallBackFunc) const
{
	ULONG64 Cr3 = 0;

	if (bRemoteAddressIsWinApi)
	{
		//触发写拷贝
		if (!TriggerCOW_ByProcessId(PID, (PVOID)pRemoteAddress, Size))
			return false;
	}

	if (GetCR3((HANDLE)PID, Cr3))
	{
		if (!m_Dbvm->WPMCloak((uintptr_t)pRemoteAddress, pBuffer, Size, Cr3))
			return false;

		CallBackFunc();

		if (!m_Dbvm->WPMCloak((uintptr_t)pRemoteAddress, OriginalBytes.data(), Size, Cr3))
			return false;
	}

	return true;
}

