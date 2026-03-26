#include "WeVt.pch.h"
#include "WeVt.Symbolic.h"
#include "SymbolicAccess.ModuleExtenderFactory.h"
#include "WeVt.Trace.h"
#include "WeVt.Symbolic.tmh"

//SRV* C:\Symbols* https://msdl.microsoft.com/download/symbols

//直接用迅雷下载
//19042
//http://msdl.microsoft.com/download/symbols/ntkrnlmp.pdb/769C521E4833ECF72E21F02BF33691A51/ntkrnlmp.pdb


BOOLEAN InitNtoskrnlSymbolsTable()
{
	symbolic_access::ModuleExtenderFactory extenderFactory{};
	const auto& moduleExtender = extenderFactory.Create(L"ntoskrnl.exe");
	if (!moduleExtender.has_value())
	{
#if ENABLE_TRACE
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "[-] ntoskrnl.exe Not found");
#endif
		return FALSE;
	}

	Global::PsGetNextProcess = (fnPsGetNextProcess)moduleExtender->GetPointer<fnPsGetNextProcess>("PsGetNextProcess");

#if ENABLE_TRACE
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[+] Global::PsGetNextProcess:0x%p", Global::PsGetNextProcess);
#endif

	return TRUE;
}