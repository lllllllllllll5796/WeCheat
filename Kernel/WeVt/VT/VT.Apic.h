#pragma once

namespace VT_Apic
{
#define MSR_IA32_APICBASE               0x0000001b
#define MSR_IA32_X2APIC_EOI				0x80b
#define MSR_IA32_X2APIC_LVT_PMI			0x834

	typedef struct _UINT128
	{
		DWORD a;
		DWORD b;
		DWORD c;
		DWORD d;
	} UINT128;

	typedef volatile struct _APIC
	{
		UINT128 Reserved1;
		UINT128 Reserved2;
		UINT128 LocalAPIC_ID;
		UINT128 LocalAPIC_Version;
		UINT128 Reserved3;
		UINT128 Reserved4;
		UINT128 Reserved5;
		UINT128 Reserved6;
		UINT128 Task_Priority;
		UINT128 Arbritation_Priority;
		UINT128 Processor_Priority;
		UINT128 EOI;
		UINT128 Reserved7;
		UINT128 Logical_Destination;
		UINT128 Destination_Format;
		UINT128 Spurious_Interrupt_Vector;
		UINT128 In_Service[8];
		UINT128 Trigger_Mode[8];
		UINT128 Interrupt_Request[8];
		UINT128 Error_Status;
		UINT128 Reserved8[7];
		UINT128 Interrupt_Command_Low32Bit;
		UINT128 Interrupt_Command_High32Bit;
		UINT128 LVT_Timer;
		UINT128 LVT_Thermal_Sensor;
		UINT128 LVT_Performance_Monitor;
		UINT128 LVT_LINT0;
		UINT128 LVT_LINT1;
		UINT128 LVT_Error;
		UINT128 Initial_Count;
		UINT128 Current_Count;
		UINT128 Reserved9[4];
		UINT128 Divide_Configuration;
		UINT128 Reserved10;
	} APIC, * PAPIC;

	void apic_clearPerfmon();
	void setup_APIC_BASE(void);
	void clean_APIC_BASE(void);
}

