#pragma once

namespace VT_Watch
{
	unsigned int vmx_Watch_PageWrites(UINT64 PhysicalAddress, int Size, int Options, int MaxEntryCount);
	unsigned int vmx_Watch_PageAccess(UINT64 PhysicalAddress, int Size, int Options, int MaxEntryCount);
	unsigned int vmx_Watch_PageExecutes(UINT64 PhysicalAddress, int Size, int Options, int MaxEntryCount);
	unsigned int vmx_Watch_Retreivelog(int ID, VmxHelper::PPageEventListDescriptor result, int* resultsize);
	unsigned int vmx_Watch_Delete(int ID);
}