#include "WeVt.pch.h"

namespace VT_Test
{
//	using namespace VT_Cloak;
//	using namespace KernelCommon;
//
//	typedef int(*CLOAKTESTFUNCTION)(void);
//
//	extern "C" int cloakTestFunction(void);
//	extern "C" void* cloakTestFunctionEnd;
//	extern "C" void* cloakTestFunctionInstruction;
//
//	void dbvm_cloak_test()
//	{
//		int DBVMVersion = vmx_getversion();
//
//		if (DBVMVersion)
//		{
//			int r;
//			int i;
//			unsigned char* newmem = (unsigned char*)malloc(8192);
//			CLOAKTESTFUNCTION newtestfunction = (CLOAKTESTFUNCTION)&newmem[4094];
//			QWORD PA = ImpCall(MmGetPhysicalAddress,(void*)newtestfunction).QuadPart + 2;
//			LOG_DEBUG("newtestfunction Address %p (PA : %p)\n", newtestfunction, PA);
//			QWORD size = (QWORD)&cloakTestFunctionEnd - (QWORD)cloakTestFunction;
//			//LOG_DEBUG("Size=%d bytes\n", size);
//			//LOG_DEBUG("开始拷贝...\n");
//			RtlCopyMemory(newtestfunction, cloakTestFunction, size);
//			//LOG_DEBUG("拷贝完毕\n");
//
//			//LOG_DEBUG("调用原始cloakTestFunction\n");
//			r = cloakTestFunction();
//			//LOG_DEBUG("cloakTestFunction() [结果]:0x%X\n", r);  //66666666
//
//			//LOG_DEBUG("调用拷贝后函数newtestfunction\n");
//			r = newtestfunction();
//			//LOG_DEBUG("newtestfunction() [结果]:0x%X\n", r);
//			//-----------
//			//LOG_DEBUG("准备给newtestfunction打补丁\n");
//			unsigned char* funcindex = (unsigned char*)newtestfunction;
//			//LOG_DEBUG("原始内容:");
//
//			for (i = 1; i < 5; i++)
//			{
//				int index = static_cast<int>((QWORD)(&cloakTestFunctionInstruction) - ((QWORD)(cloakTestFunction)) + i);
//				//LOG_DEBUG("%02X ", funcindex[index]);
//				funcindex[index] = 0x88;
//			}
//			//LOG_DEBUG("\n");
//			//LOG_DEBUG("补丁已经应用了\n");
//			r = newtestfunction();
//			//LOG_DEBUG("补丁后 cloakTestFunction() [结果]:0x%X\n", r);  //结果是0x88888888
//
//			
//			//LOG_DEBUG("撤销补丁. 现在准备尝试隐身\n");
//			RtlCopyMemory(newtestfunction, cloakTestFunction, size);  //恢复原状
//			//int beforecloak = VT_Util::GenerateCRC((unsigned char*)newtestfunction, size);
//			//LOG_DEBUG("隐身之前的Crc是 0x%X\n", beforecloak);
//
// 			unsigned char* buf = (unsigned char*)newtestfunction;
//// 			LOG_DEBUG("原始newtestfunction内容:%p:", newtestfunction);
//// 			for (i = 0; i < size; i++)
//// 			{
//// 				DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "%02X ", buf[i]);
//// 			}
//// 			LOG_DEBUG("\n");
//			
//			//---------------准备隐身------------
//			//LOG_DEBUG("Activating cloak (mode 1)\n");
//
// 			r = vmx_Cloak_Activate(PA, 1);    //英特尔cpu上使PA不可读,而AMD上使其不可执行并且读的时候还是假数据
//
//			//LOG_DEBUG("Cloak activated\n");
//			//LOG_DEBUG("dbvm_cloak_activate(%p) returned %d\n", PA, r);  //returned 0
//
//			KernelIntrin__invlpg((void*)newtestfunction);
//
// 			buf = (unsigned char*)newtestfunction;
//
//// 			LOG_DEBUG("newtestfunction:%p:", newtestfunction);
//// 			for (i = 0; i < size; i++)
//// 				LOG_DEBUG("%02X ", buf[i]); //when testing, should be all 0xce's
//// 			
//// 			LOG_DEBUG("\n");
//
//			//int aftercloak = VT_Util::GenerateCRC((unsigned char*)newtestfunction, size);
//			//LOG_DEBUG("After cloak crc is 0x%X\n", aftercloak);
//
//			//LOG_DEBUG("Calling newtestfunction: (cloaked, unpatched)\n");
//			r = newtestfunction();
//			//LOG_DEBUG("After cloak but no patch ,result of cloakTestFunction() is 0x%X\n", r);  //is 1
//
//			//-----------------
//			unsigned char* executable = (unsigned char*)malloc(4096);
//			//LOG_DEBUG("Allocated memory for the executable copy at %p\n", executable);
//
//			//LOG_DEBUG("1: crc=%x\n", VT_Util::GenerateCRC((unsigned char*)newtestfunction, size));   //1: crc=
//
//			r = vmx_Cloak_ReadOriginal(PA, executable);   //读取原始内容
//			//LOG_DEBUG("2: crc=%x\n", VT_Util::GenerateCRC((unsigned char*)newtestfunction, size));   //2: crc=
//			//LOG_DEBUG("dbvm_cloak_readExecutable returned %d\n", r);    //returned 0
//			//此时executable里的内容和原始newtestfunction内容一致
//			
//			if (r == 0)
//			{
//				//int beforepatch = VT_Util::GenerateCRC((unsigned char*)newtestfunction, size);
//				//LOG_DEBUG("3: crc=%x\n", generateCRC((unsigned char*)newtestfunction, size)); //3: crc=
//
//				//LOG_DEBUG("Applying patch to executable memory\n");
//				
//				for (i = 1; i < 5; i++)
//					executable[0x13 + i] = 0x88;
//
//				//LOG_DEBUG("4: crc=%x\n", generateCRC((unsigned char*)newtestfunction, size));   //4: crc=
//
//				r = vmx_Cloak_WriteOriginal(PA, executable);
//				//LOG_DEBUG("dbvm_cloak_writeExectuable returned %d\n", r);   //returned 0
//				//LOG_DEBUG("5: crc=%x\n", generateCRC((unsigned char*)newtestfunction, size));  //5: crc=
//				
//				//int afterpatch = VT_Util::GenerateCRC((unsigned char*)newtestfunction, size);
//
//				//LOG_DEBUG("6: crc=%x\n", generateCRC((unsigned char*)newtestfunction, size));   //6: crc=
//
//				if (r == 0)
//				{
//					//LOG_DEBUG("Calling newtestfunction: (cloaked, patched)\n");
//					r = newtestfunction();
//					LOG_DEBUG("After cloak and patch ,result of newtestfunction() is 0x%X\n", r);  //is 0x88888888
//					//LOG_DEBUG("Cloak active and patched, result is 0x%X\n",r);  //is 0x88888888
//					//LOG_DEBUG("7: crc = 0x%X\n", generateCRC((unsigned char*)newtestfunction, size));  //7: crc =
//				}
//			}
//
//			r = vmx_Cloak_Deactivate(PA);
//			//LOG_DEBUG("8: crc=%x\n", generateCRC((unsigned char*)newtestfunction, size));  //8: crc=
//			//LOG_DEBUG("dbvm_cloak_deactivate(%p) returned %d\n", PA, r);    //returned 1
//			//LOG_DEBUG("\n");
//
//			r = newtestfunction();
//			LOG_DEBUG("last newtestfunction() is 0x%X\n", r);  //is 0x66666666
//
//			free(executable);
//			free(newmem);
//		}
//		else
//		LOG_DEBUG("Error, no DBVM loaded\n");
//	}
//
//	void watchTestFunction()
//	{
//		LOG_DEBUG("watchTestFunction\n");
//	}
//
//	void dbvm_watch_execute_test()
//	{
//		int DBVMVersion = vmx_getversion();
//
//		if (DBVMVersion)
//		{
//			int r;
//
//			QWORD PA;
//			void* results = malloc(8192);
//			RtlZeroMemory(results, 8192);
//
//			//EFER_MSR
//			UINT64 MsrValue = VT_Util::readMSR(0xc0000080);
//			KernelIntrin__writemsr(0xc0000080, MsrValue & ~(1 << 11));  //11; (* Execute/read code, accessed *)
//
//			PA = ImpCall(MmGetPhysicalAddress, (PVOID)watchTestFunction).QuadPart;
//
//			r = VT_Watch::vmx_Watch_PageExecutes(PA, 4, 0, 16);
//
//			free(results);
//		}
//	}

}