#include "WeVt.pch.h"

volatile VT_Apic::PAPIC DBVM_APIC_BASE;

namespace VT_Apic
{
	using namespace VT_Util;
	BOOL x2APICMode = FALSE;

	void apic_clearPerfmon()
	{
		if (x2APICMode)
		{
			KernelIntrin__writemsr(MSR_IA32_X2APIC_LVT_PMI, (KernelIntrin__readmsr(MSR_IA32_X2APIC_LVT_PMI) & (UINT64)0xff));
			KernelIntrin__writemsr(MSR_IA32_X2APIC_EOI, 0);
		}
		else
		{
			LOG_DEBUG("Clear perfmon (APIC_BASE at %llx)\n", DBVM_APIC_BASE);
			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.a at %p\n", &DBVM_APIC_BASE->LVT_Performance_Monitor.a);
			LOG_DEBUG("APIC_BASE->EOI.a at %p\n", &DBVM_APIC_BASE->EOI.a);

			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.a had value %x\n", DBVM_APIC_BASE->LVT_Performance_Monitor.a);
			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.b had value %x\n", DBVM_APIC_BASE->LVT_Performance_Monitor.b);
			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.c had value %x\n", DBVM_APIC_BASE->LVT_Performance_Monitor.c);
			LOG_DEBUG("APIC_BASE->LVT_Performance_Monitor.d had value %x\n", DBVM_APIC_BASE->LVT_Performance_Monitor.d);

			DBVM_APIC_BASE->LVT_Performance_Monitor.a = DBVM_APIC_BASE->LVT_Performance_Monitor.a & 0xff;
			DBVM_APIC_BASE->EOI.a = 0;
		}
	}

	void setup_APIC_BASE(void)
	{
		PHYSICAL_ADDRESS Physical_APIC_BASE;
		UINT64 APIC_BASE_VALUE = readMSR(MSR_IA32_APICBASE);
		//LOG_DEBUG("Fetching the APIC base\n");

		Physical_APIC_BASE.QuadPart = APIC_BASE_VALUE & 0xFFFFFFFFFFFFF000ULL;

		//LOG_DEBUG("Physical_APIC_BASE=%p\n", Physical_APIC_BASE.QuadPart);

		DBVM_APIC_BASE = (PAPIC)ImpCall(MmMapIoSpace, Physical_APIC_BASE, sizeof(APIC), MmNonCached);

		//LOG_DEBUG("APIC_BASE at %p\n", APIC_BASE);

		x2APICMode = (APIC_BASE_VALUE & (1 << 10)) != 0;
		//LOG_DEBUG("x2APICMode=%d\n", x2APICMode);
	}

	void clean_APIC_BASE(void)
	{
		if (DBVM_APIC_BASE)
			ImpCall(MmUnmapIoSpace, (PVOID)DBVM_APIC_BASE, sizeof(APIC));
	}

}