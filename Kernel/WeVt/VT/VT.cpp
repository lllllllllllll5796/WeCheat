#include "WeVt.pch.h"

namespace VT
{
	BOOLEAN SuportVT()
	{
		LOG_DEBUG("[+] SuportVT 1\n");

		BOOLEAN  bSuportVT = VT_Util::HvmIsSuported();

		if (!bSuportVT)
		{
			LOG_DEBUG("[-] 此机器不支持VT虚拟化\n");
			return FALSE;
		}

		LOG_DEBUG("[+] SuportVT 3\n");

		return TRUE;
	}

	BOOLEAN StartVT(PCWSTR vmdiskPath, DWORD32 cpuid)  //这里的cpuid
	{
		BOOLEAN bRet = FALSE;
		LOG_DEBUG("[+] StartVT\n");

#if 0
		WCHAR FileName[512];
		RtlZeroMemory(FileName, sizeof(FileName));

		if (vmdiskPath == NULL)
		{
			wcscpy(FileName, L"\\??\\");
			wcscat(FileName, L"C:\\vmdisk.img");
		}
		else
		{
			wcscpy(FileName, vmdiskPath);
		}

		VT_VmxOffLoad::InitializeDBVM(FileName);
#else
	    //无模块启动
		VT_VmxOffLoad::InitializeDBVM_4NoModule(FALSE);
#endif

		//准备启动虚拟化
		if (VT_VmxOffLoad::InitializedVmm)
		{
			if (cpuid == 0xffffffff)
			{
				//所有CPU都需要加载虚拟化
				VT_Util::forEachCpu(VT_VmxOffLoad::VmxOffLoad_Dpc, NULL, NULL, NULL, VT_VmxOffLoad::VmxOffLoad_Override);
				VT_VmxOffLoad::CleanupDBVM();  //扫尾
			}
			else
			{
				VT_Util::forOneCpu((CCHAR)cpuid, VT_VmxOffLoad::VmxOffLoad_Dpc, NULL, NULL, NULL, VT_VmxOffLoad::VmxOffLoad_Override);
			}

			LOG_DEBUG("Returned from vmxoffload()\n");

			return TRUE;
		}

		return FALSE;
	}

	BOOLEAN EnableVT()
	{
		BOOLEAN bRet = FALSE;

		LOG_DEBUG("[+] EnableVT 1\n");

		if (Global::g_SuportVT)
		{
			LOG_DEBUG("[+] EnableVT 2\n");

			if (Global::g_HypervisorRunning == FALSE)
			{
				LOG_DEBUG("[+] EnableVT 3\n");
				//-------------------------
				if (VT::StartVT(NULL, 0xffffffff))
				{
					LOG_DEBUG("[+] 启动虚拟化成功\n");
					Global::g_HypervisorRunning = TRUE;
					bRet = TRUE;
				}
				else
				{
					LOG_DEBUG("[-] 启动虚拟化失败\n");
				}
			}
			else
			{
				LOG_DEBUG("[+] 虚拟化运行中\n");
				bRet = TRUE;
			}
		}
		else
		{
			//不支持VT
			LOG_DEBUG("[-] 不支持VT\r\n");
		}

 		return bRet;
	}

}
