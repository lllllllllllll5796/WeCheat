#include "WeVt.pch.h"

namespace VT_Watch
{
	using namespace VmxHelper;
	unsigned int vmx_Watch_PageWrites(UINT64 PhysicalAddress, int Size, int Options, int MaxEntryCount)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command; //VMCALL_FINDWHATWRITESPAGE
			UINT64 PhysicalAddress;
			int Size;
			int Options; //binary.  
						 //  Bit 0: 0=Log RIP once. 1=Log RIP multiple times (when different registers)
						 //  Bit 1: 0=Only log given Physical Address. 1=Log everything in the page(s) that is/are affected
						 //  Bit 2: 0=Do not save FPU/XMM data, 1=Also save FPU/XMM data
						 //  Bit 3: 0=Do not save a stack snapshot, 1=Save stack snapshot
						 //  Bit 4: 0=No PMI when full, 1=PMI when full
			int MaxEntryCount; //how much memory should DBVM allocate for the buffer 		
			int UsePMI; //trigger a PMI interrupt when full (so you don't lose info)
			int ID; //ID describing this watcher for this CPU (keep track of this on a per cpu basis if you do more than 1)
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_WATCH_WRITES;
		vmcallinfo.PhysicalAddress = PhysicalAddress;

		if (((PhysicalAddress + Size) & 0xfffffffffffff000ULL) > PhysicalAddress) //passes a pageboundary, strip of the excess
			Size = 0x1000 - (PhysicalAddress & 0xfff);

		vmcallinfo.Size = Size;
		vmcallinfo.Options = Options;
		vmcallinfo.MaxEntryCount = MaxEntryCount;
		vmcallinfo.ID = 0xffffffff;

		DoVmcall(&vmcallinfo);
		return vmcallinfo.ID;
	}

	unsigned int vmx_Watch_PageAccess(UINT64 PhysicalAddress, int Size, int Options, int MaxEntryCount)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command; //VMCALL_FINDWHATWRITESPAGE
			UINT64 PhysicalAddress;
			int Size;
			int Options; //binary.  
			//  Bit 0: 0=Log RIP once. 1=Log RIP multiple times (when different registers)
			//  Bit 1: 0=Only log given Physical Address. 1=Log everything in the page(s) that is/are affected
			//  Bit 2: 0=Do not save FPU/XMM data, 1=Also save FPU/XMM data
			//  Bit 3: 0=Do not save a stack snapshot, 1=Save stack snapshot
			//  Bit 4: 0=No PMI when full, 1=PMI when full
			int MaxEntryCount; //how much memory should DBVM allocate for the buffer 		

			int ID; //ID describing this watcher for this CPU (keep track of this on a per cpu basis if you do more than 1)
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_WATCH_READS;
		vmcallinfo.PhysicalAddress = PhysicalAddress;

		if (((PhysicalAddress + Size) & 0xfffffffffffff000ULL) > PhysicalAddress) //passes a pageboundary, strip of the excess
			Size = 0x1000 - (PhysicalAddress & 0xfff);

		vmcallinfo.Size = Size;
		vmcallinfo.Options = Options;
		vmcallinfo.MaxEntryCount = MaxEntryCount;
		vmcallinfo.ID = 0xffffffff;

		DoVmcall(&vmcallinfo);
		return vmcallinfo.ID;
	}

	//自己添加的
	unsigned int vmx_Watch_PageExecutes(UINT64 PhysicalAddress, int Size, int Options, int MaxEntryCount)
	{
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command;
			UINT64 PhysicalAddress;
			int Size;
			int Options; //binary.  
			//  Bit 0: 0=Log RIP once. 1=Log RIP multiple times (when different registers)
			//  Bit 1: 0=Only log given Physical Address. 1=Log everything in the page(s) that is/are affected
			//  Bit 2: 0=Do not save FPU/XMM data, 1=Also save FPU/XMM data
			//  Bit 3: 0=Do not save a stack snapshot, 1=Save stack snapshot
			//  Bit 4: 0=No PMI when full, 1=PMI when full
			int MaxEntryCount; //how much memory should DBVM allocate for the buffer 		

			int ID; //ID describing this watcher for this CPU (keep track of this on a per cpu basis if you do more than 1)
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_WATCH_EXECUTES;
		vmcallinfo.PhysicalAddress = PhysicalAddress;

		if (((PhysicalAddress + Size) & 0xfffffffffffff000ULL) > PhysicalAddress) //passes a pageboundary, strip of the excess
			Size = 0x1000 - (PhysicalAddress & 0xfff);

		vmcallinfo.Size = Size;
		vmcallinfo.Options = Options;
		vmcallinfo.MaxEntryCount = MaxEntryCount;
		vmcallinfo.ID = 0xffffffff;

		DoVmcall(&vmcallinfo);

		return vmcallinfo.ID;
	}

	unsigned int vmx_Watch_Retreivelog(int ID, PPageEventListDescriptor result, int* resultsize)
		/*
		Used to retrieve both read and write watches
		*/
	{
		unsigned int r;
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command; //VMCALL_FINDWHATWRITESPAGE
			DWORD ID;
			UINT64 results;
			int resultsize;
			int copied; //the number of bytes copied so far (This is a repeating instruction)
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_WATCH_RETRIEVELOG;

		vmcallinfo.ID = ID;
		vmcallinfo.results = (UINT64)result;
		vmcallinfo.resultsize = *resultsize;
		r = (unsigned int)DoVmcall(&vmcallinfo);
		*resultsize = vmcallinfo.resultsize;
		return r; //returns 0 on success, 1 on too small buffer.  buffersize contains the size in both cases
	}

	unsigned int vmx_Watch_Delete(int ID)
	{
		//disables the watch operation
#pragma pack(1)
		struct
		{
			unsigned int structsize;
			unsigned int level2pass;
			unsigned int command; //VMCALL_FINDWHATWRITESPAGE
			DWORD ID;
		} vmcallinfo;
#pragma pack()

		vmcallinfo.structsize = sizeof(vmcallinfo);
		vmcallinfo.level2pass = vmx_password2;
		vmcallinfo.command = VMCALL_WATCH_DELETE;

		vmcallinfo.ID = ID;

		return (unsigned int)DoVmcall(&vmcallinfo); //0 on success, anything else fail
	}
}
