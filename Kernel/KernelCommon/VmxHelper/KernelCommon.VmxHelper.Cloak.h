#pragma once

namespace VmxHelper
{
	int vmx_Cloak_Activate(QWORD physicalAddress, int mode = 1);

	int vmx_Cloak_ReadOriginal(QWORD PhysicalAddress, void* destination);

	int vmx_Cloak_WriteOriginal(QWORD PhysicalAddress, void* source);

	int vmx_Cloak_Deactivate(QWORD PhysicalAddress);

	void AddRecoveryMap(PEPROCESS Process, QWORD VirtualAddress, QWORD PhysicalAddress);
	void DelRecoveryMap(PEPROCESS Process, QWORD VirtualAddress, QWORD PhysicalAddress);

	void RecoveryCloaks(PEPROCESS Process);

	//void CloakReset();
}