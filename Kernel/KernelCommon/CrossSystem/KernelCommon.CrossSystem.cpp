#include "KernelCommon.pch.h"
#include "KernelCommon.CrossSystem.h"

namespace KernelCommon
{
	SYSCALLNUMBERS g_SyscallNumbers;
	WIN32KSYSCALLNUMBERS g_Win32KSyscallNumbers;
	NTAPIOFFSET g_NtapiOffsets;

	#define GET_OFFSET_IN_STRUCT(S, F) \
	auto fnGetOffset = []() -> ULONG \
	{ \
		auto Version = GetSystemVersion(); \
		switch (Version) \
		{ \
		default: \
			return NULL; \
		case SystemVersion::Windows7: \
			return GET_OFFSET_64(wdk2::build_7600::S, F); \
		case SystemVersion::Windows7_SP1: \
			return GET_OFFSET_64(wdk2::build_7601::S, F); \
		case SystemVersion::Windows8: \
			return GET_OFFSET_64(wdk2::build_9200::S, F); \
		case SystemVersion::Windows8_1: \
			return GET_OFFSET_64(wdk2::build_9600::S, F); \
		case SystemVersion::Windows10_1507: \
			return GET_OFFSET_64(wdk2::build_10240::S, F); \
		case SystemVersion::Windows10_1511: \
			return GET_OFFSET_64(wdk2::build_10586::S, F); \
		case SystemVersion::Windows10_1607: \
			return GET_OFFSET_64(wdk2::build_14393::S, F); \
		case SystemVersion::Windows10_1703: \
			return GET_OFFSET_64(wdk2::build_15063::S, F); \
		case SystemVersion::Windows10_1709: \
			return GET_OFFSET_64(wdk2::build_16299::S, F); \
		case SystemVersion::Windows10_1803: \
			return GET_OFFSET_64(wdk2::build_17134::S, F); \
		case SystemVersion::Windows10_1809: \
			return GET_OFFSET_64(wdk2::build_17763::S, F); \
		case SystemVersion::Windows10_1903: \
			return GET_OFFSET_64(wdk2::build_18362::S, F); \
		case SystemVersion::Windows10_1909: \
			return GET_OFFSET_64(wdk2::build_18363::S, F); \
		case SystemVersion::Windows10_2004: \
			return GET_OFFSET_64(wdk2::build_19041::S, F); \
		case SystemVersion::Windows10_20H2: \
			return GET_OFFSET_64(wdk2::build_19042::S, F); \
		case SystemVersion::Windows10_21H1: \
			return GET_OFFSET_64(wdk2::build_19043::S, F); \
		case SystemVersion::Windows10_21H2: \
			return GET_OFFSET_64(wdk2::build_19044::S, F); \
		case SystemVersion::Windows10_22H2: \
			return GET_OFFSET_64(wdk2::build_19045::S, F); \
		case SystemVersion::Windows10_20H2_19569: \
			return GET_OFFSET_64(wdk2::build_19569::S, F); \
		case SystemVersion::Windows10_21H1_20150: \
			return GET_OFFSET_64(wdk2::build_20150::S, F); \
		case SystemVersion::Windows10_21H1_20180: \
			return GET_OFFSET_64(wdk2::build_20180::S, F); \
		case SystemVersion::Windows10_XXXX_21292: \
			return GET_OFFSET_64(wdk2::build_21292::S, F); \
		case SystemVersion::Windows11_21H2: \
			return GET_OFFSET_64(wdk2::build_22000::S, F); \
		case SystemVersion::Windows11_22H2: \
			return GET_OFFSET_64(wdk2::build_22621::S, F); \
		case SystemVersion::Windows11_23H2: \
			return GET_OFFSET_64(wdk2::build_22631::S, F); \
		} \
	};

	#define GET_FIELD_IN_STRUCT(T, S, X, F) \
	auto fnGetField = [X]() -> T \
	{ \
		switch (GetSystemVersion()) \
		{ \
		default: \
			return NULL; \
		case SystemVersion::Windows7: \
			return (T)((wdk2::build_7600::S)X)->F; \
		case SystemVersion::Windows7_SP1: \
			return (T)((wdk2::build_7601::S)X)->F; \
		case SystemVersion::Windows8: \
			return (T)((wdk2::build_9200::S)X)->F; \
		case SystemVersion::Windows8_1: \
			return (T)((wdk2::build_9600::S)X)->F; \
		case SystemVersion::Windows10_1507: \
			return (T)((wdk2::build_10240::S)X)->F; \
		case SystemVersion::Windows10_1511: \
			return (T)((wdk2::build_10586::S)X)->F; \
		case SystemVersion::Windows10_1607: \
			return (T)((wdk2::build_14393::S)X)->F; \
		case SystemVersion::Windows10_1703: \
			return (T)((wdk2::build_15063::S)X)->F; \
		case SystemVersion::Windows10_1709: \
			return (T)((wdk2::build_16299::S)X)->F; \
		case SystemVersion::Windows10_1803: \
			return (T)((wdk2::build_17134::S)X)->F; \
		case SystemVersion::Windows10_1809: \
			return (T)((wdk2::build_17763::S)X)->F; \
		case SystemVersion::Windows10_1903: \
			return (T)((wdk2::build_18362::S)X)->F; \
		case SystemVersion::Windows10_1909: \
			return (T)((wdk2::build_18363::S)X)->F; \
		case SystemVersion::Windows10_2004: \
			return (T)((wdk2::build_19041::S)X)->F; \
		case SystemVersion::Windows10_20H2: \
			return (T)((wdk2::build_19042::S)X)->F; \
		case SystemVersion::Windows10_21H1: \
			return (T)((wdk2::build_19043::S)X)->F; \
		case SystemVersion::Windows10_21H2: \
			return (T)((wdk2::build_19044::S)X)->F; \
		case SystemVersion::Windows10_22H2: \
			return (T)((wdk2::build_19045::S)X)->F; \
		case SystemVersion::Windows10_20H2_19569: \
			return (T)((wdk2::build_19569::S)X)->F; \
		case SystemVersion::Windows10_21H1_20150: \
			return (T)((wdk2::build_20150::S)X)->F; \
		case SystemVersion::Windows10_21H1_20180: \
			return (T)((wdk2::build_20180::S)X)->F; \
		case SystemVersion::Windows10_XXXX_21292: \
			return (T)((wdk2::build_21292::S)X)->F; \
		case SystemVersion::Windows11_21H2: \
			return (T)((wdk2::build_22000::S)X)->F; \
		case SystemVersion::Windows11_22H2: \
			return (T)((wdk2::build_22621::S)X)->F; \
		case SystemVersion::Windows11_23H2: \
			return (T)((wdk2::build_22631::S)X)->F; \
		} \
	};

	#define GET_POINTER_IN_STRUCT(S, X, F) \
	auto fnGetPointer = [X]() -> PVOID \
	{ \
		switch (GetSystemVersion()) \
		{ \
		default: \
			return NULL; \
		case SystemVersion::Windows7: \
			return (PVOID)(&(((wdk2::build_7600::S)X)->F)); \
		case SystemVersion::Windows7_SP1: \
			return (PVOID)(&(((wdk2::build_7601::S)X)->F)); \
		case SystemVersion::Windows8: \
			return (PVOID)(&(((wdk2::build_9200::S)X)->F)); \
		case SystemVersion::Windows8_1: \
			return (PVOID)(&(((wdk2::build_9600::S)X)->F)); \
		case SystemVersion::Windows10_1507: \
			return (PVOID)(&(((wdk2::build_10240::S)X)->F)); \
		case SystemVersion::Windows10_1511: \
			return (PVOID)(&(((wdk2::build_10586::S)X)->F)); \
		case SystemVersion::Windows10_1607: \
			return (PVOID)(&(((wdk2::build_14393::S)X)->F)); \
		case SystemVersion::Windows10_1703: \
			return (PVOID)(&(((wdk2::build_15063::S)X)->F)); \
		case SystemVersion::Windows10_1709: \
			return (PVOID)(&(((wdk2::build_16299::S)X)->F)); \
		case SystemVersion::Windows10_1803: \
			return (PVOID)(&(((wdk2::build_17134::S)X)->F)); \
		case SystemVersion::Windows10_1809: \
			return (PVOID)(&(((wdk2::build_17763::S)X)->F)); \
		case SystemVersion::Windows10_1903: \
			return (PVOID)(&(((wdk2::build_18362::S)X)->F)); \
		case SystemVersion::Windows10_1909: \
			return (PVOID)(&(((wdk2::build_18363::S)X)->F)); \
		case SystemVersion::Windows10_2004: \
			return (PVOID)(&(((wdk2::build_19041::S)X)->F)); \
		case SystemVersion::Windows10_20H2: \
			return (PVOID)(&(((wdk2::build_19042::S)X)->F)); \
		case SystemVersion::Windows10_21H1: \
			return (PVOID)(&(((wdk2::build_19043::S)X)->F)); \
		case SystemVersion::Windows10_21H2: \
			return (PVOID)(&(((wdk2::build_19044::S)X)->F)); \
		case SystemVersion::Windows10_22H2: \
			return (PVOID)(&(((wdk2::build_19045::S)X)->F)); \
		case SystemVersion::Windows10_20H2_19569: \
			return (PVOID)(&(((wdk2::build_19569::S)X)->F)); \
		case SystemVersion::Windows10_21H1_20150: \
			return (PVOID)(&(((wdk2::build_20150::S)X)->F)); \
		case SystemVersion::Windows10_21H1_20180: \
			return (PVOID)(&(((wdk2::build_20180::S)X)->F)); \
		case SystemVersion::Windows10_XXXX_21292: \
			return (PVOID)(&(((wdk2::build_21292::S)X)->F)); \
		case SystemVersion::Windows11_21H2: \
			return (PVOID)(&(((wdk2::build_22000::S)X)->F)); \
		case SystemVersion::Windows11_22H2: \
			return (PVOID)(&(((wdk2::build_22621::S)X)->F)); \
		case SystemVersion::Windows11_23H2: \
			return (PVOID)(&(((wdk2::build_22631::S)X)->F)); \
		} \
	};

	#define GET_FIELD_IN_STRUCT2(T, S, X, F, U) \
	auto fnGetField = [X]() -> T \
	{ \
		switch (GetSystemVersion()) \
		{ \
		default: \
			return NULL; \
		case SystemVersion::Windows7: \
			return (T)((wdk2::build_7600::S)X)->F.U; \
		case SystemVersion::Windows7_SP1: \
			return (T)((wdk2::build_7601::S)X)->F.U; \
		case SystemVersion::Windows8: \
			return (T)((wdk2::build_9200::S)X)->F.U; \
		case SystemVersion::Windows8_1: \
			return (T)((wdk2::build_9600::S)X)->F.U; \
		case SystemVersion::Windows10_1507: \
			return (T)((wdk2::build_10240::S)X)->F.U; \
		case SystemVersion::Windows10_1511: \
			return (T)((wdk2::build_10586::S)X)->F.U; \
		case SystemVersion::Windows10_1607: \
			return (T)((wdk2::build_14393::S)X)->F.U; \
		case SystemVersion::Windows10_1703: \
			return (T)((wdk2::build_15063::S)X)->F.U; \
		case SystemVersion::Windows10_1709: \
			return (T)((wdk2::build_16299::S)X)->F.U; \
		case SystemVersion::Windows10_1803: \
			return (T)((wdk2::build_17134::S)X)->F.U; \
		case SystemVersion::Windows10_1809: \
			return (T)((wdk2::build_17763::S)X)->F.U; \
		case SystemVersion::Windows10_1903: \
			return (T)((wdk2::build_18362::S)X)->F.U; \
		case SystemVersion::Windows10_1909: \
			return (T)((wdk2::build_18363::S)X)->F.U; \
		case SystemVersion::Windows10_2004: \
			return (T)((wdk2::build_19041::S)X)->F.U; \
		case SystemVersion::Windows10_20H2: \
			return (T)((wdk2::build_19042::S)X)->F.U; \
		case SystemVersion::Windows10_21H1: \
			return (T)((wdk2::build_19043::S)X)->F.U; \
		case SystemVersion::Windows10_21H2: \
			return (T)((wdk2::build_19044::S)X)->F.U; \
		case SystemVersion::Windows10_22H2: \
			return (T)((wdk2::build_19045::S)X)->F.U; \
		case SystemVersion::Windows10_20H2_19569: \
			return (T)((wdk2::build_19569::S)X)->F.U; \
		case SystemVersion::Windows10_21H1_20150: \
			return (T)((wdk2::build_20150::S)X)->F.U; \
		case SystemVersion::Windows10_21H1_20180: \
			return (T)((wdk2::build_20180::S)X)->F.U; \
		case SystemVersion::Windows10_XXXX_21292: \
			return (T)((wdk2::build_21292::S)X)->F.U; \
		case SystemVersion::Windows11_21H2: \
			return (T)((wdk2::build_22000::S)X)->F.U; \
		case SystemVersion::Windows11_22H2: \
			return (T)((wdk2::build_22621::S)X)->F.U; \
		case SystemVersion::Windows11_23H2: \
			return (T)((wdk2::build_22631::S)X)->F.U; \
		} \
	};

	#define GET_SDT_INDEX(S, F) \
	[]() -> SHORT \
	{ \
		auto Version = GetSystemVersion(); \
		switch (Version) \
		{ \
		default: \
			return NULL; \
		case SystemVersion::Windows7: \
			return wdk2::build_7600::S::F; \
		case SystemVersion::Windows7_SP1: \
			return wdk2::build_7601::S::F; \
		case SystemVersion::Windows8: \
			return wdk2::build_9200::S::F; \
		case SystemVersion::Windows8_1: \
			return wdk2::build_9600::S::F; \
		case SystemVersion::Windows10_1507: \
			return wdk2::build_10240::S::F; \
		case SystemVersion::Windows10_1511: \
			return wdk2::build_10586::S::F; \
		case SystemVersion::Windows10_1607: \
			return wdk2::build_14393::S::F; \
		case SystemVersion::Windows10_1703: \
			return wdk2::build_15063::S::F; \
		case SystemVersion::Windows10_1709: \
			return wdk2::build_16299::S::F; \
		case SystemVersion::Windows10_1803: \
			return wdk2::build_17134::S::F; \
		case SystemVersion::Windows10_1809: \
			return wdk2::build_17763::S::F; \
		case SystemVersion::Windows10_1903: \
			return wdk2::build_18362::S::F; \
		case SystemVersion::Windows10_1909: \
			return wdk2::build_18363::S::F; \
		case SystemVersion::Windows10_2004: \
			return wdk2::build_19041::S::F; \
		case SystemVersion::Windows10_20H2: \
			return wdk2::build_19042::S::F; \
		case SystemVersion::Windows10_21H1: \
			return wdk2::build_19043::S::F; \
		case SystemVersion::Windows10_21H2: \
			return wdk2::build_19044::S::F; \
		case SystemVersion::Windows10_22H2: \
			return wdk2::build_19045::S::F; \
		case SystemVersion::Windows10_20H2_19569: \
			return wdk2::build_19569::S::F; \
		case SystemVersion::Windows10_21H1_20150: \
			return wdk2::build_20150::S::F; \
		case SystemVersion::Windows10_21H1_20180: \
			return wdk2::build_20180::S::F; \
		case SystemVersion::Windows10_XXXX_21292: \
			return wdk2::build_21292::S::F; \
		case SystemVersion::Windows11_21H2: \
			return wdk2::build_22000::S::F; \
		case SystemVersion::Windows11_22H2: \
			return wdk2::build_22621::S::F; \
		case SystemVersion::Windows11_23H2: \
			return wdk2::build_22631::S::F; \
		} \
	};

	BOOL Is_Win7_7600()
	{
		return (NtSystemVersion == SystemVersion::Windows7);
	}

	BOOL Is_Win7_7601()
	{
		return (NtSystemVersion == SystemVersion::Windows7_SP1);
	}

	BOOL Is_Win8_9200()
	{
		return (NtSystemVersion == SystemVersion::Windows8);
	}

	BOOL Is_Win8_9600()
	{
		return (NtSystemVersion == SystemVersion::Windows8_1);
	}

	BOOL Is_Win7()
	{
		if (Is_Win7_7600())
		{
			return TRUE;
		}

		if (Is_Win7_7601())
		{
			return TRUE;
		}

		return FALSE;
	}

	BOOL Is_Win8()
	{
		if (Is_Win8_9200())
		{
			return TRUE;
		}

		if (Is_Win8_9600())
		{
			return TRUE;
		}

		return FALSE;
	}

	BOOL Is_Win10()
	{
		if (NtSystemVersion >= SystemVersion::Windows10 &&
			NtSystemVersion < SystemVersion::Windows11
			)
		{
			return TRUE;
		}

		return FALSE;
	}

	BOOL Is_Win11()
	{
		if (NtSystemVersion >= SystemVersion::Windows11)
		{
			return TRUE;
		}

		return FALSE;
	}

	//PTEB
	PVOID NtCurrentTeb(VOID)
	{
		PVOID Teb = nullptr;
		const ULONG_PTR ulTeb = (ULONG_PTR)__readgsdword(0x30);
		Teb = (wdk::PTEB64)(ulTeb);
		return Teb;
	}

	BOOL GetSuppressDebugMsg(PVOID pTeb64)
	{
		BOOL bRet = FALSE;
		bRet = static_cast<wdk::PTEB64>(pTeb64)->SuppressDebugMsg;
		return bRet;
	}


	/************************************************************************/
	/*                        GET_OFFSET_IN_STRUCT                          */
	/************************************************************************/
	::PPEB GetProcessPeb(PEPROCESS Process)
	{
		GET_OFFSET_IN_STRUCT(EPROCESS, Peb);
		return *reinterpret_cast<::PPEB*>((char*)Process + fnGetOffset());
	}

	PWOW64_PROCESS GetProcessWow64Process(PEPROCESS Process)
	{
		GET_OFFSET_IN_STRUCT(EPROCESS, WoW64Process);
		return *reinterpret_cast<PWOW64_PROCESS*>((char*)Process + fnGetOffset());
	}

	PDEBUG_OBJECT GetProcessDebugPort(PEPROCESS Process) 
	{
		GET_OFFSET_IN_STRUCT(EPROCESS, DebugPort);

		return *reinterpret_cast<PDEBUG_OBJECT*>((char*)Process + fnGetOffset());
	}


	ULONG GetObjectTable_Offset()
	{
		ULONG xresult = 0;
		GET_OFFSET_IN_STRUCT(EPROCESS, ObjectTable);
		xresult = fnGetOffset();
		return xresult;
	}

	ULONG GetSectionBaseAddress_Offset()
	{
		ULONG xresult = 0;
		GET_OFFSET_IN_STRUCT(EPROCESS, SectionBaseAddress);
		xresult = fnGetOffset();
		return xresult;
	}

	ULONG GetActiveProcessLinks_Offset()
	{
		ULONG xresult = 0;
		GET_OFFSET_IN_STRUCT(EPROCESS, ActiveProcessLinks);
		xresult = fnGetOffset();
		return xresult;
	}

	ULONG GetWin32StartAddress_Offset()
	{
		ULONG xresult = 0;
		GET_OFFSET_IN_STRUCT(ETHREAD, Win32StartAddress);
		xresult = fnGetOffset();
		return xresult;
	}

	ULONG GetSectionObject_Offset()
	{
		ULONG xresult = 0;
		GET_OFFSET_IN_STRUCT(EPROCESS, SectionObject);
		xresult = fnGetOffset();
		return xresult;
	}

	ULONG GetPreviousMode_Offset()
	{
		ULONG xresult = 0;
		GET_OFFSET_IN_STRUCT(KTHREAD, PreviousMode);
		xresult = fnGetOffset();
		return xresult;
	}


	//---------------------------------------------
	ULONG32 PspGetThreadGrantAccess(PETHREAD Thread)
	{
		return 1;
	}

	BOOL IS_SYSTEM_THREAD(PETHREAD Thread)
	{
		return ((GetCrossThreadFlags(Thread) & PS_CROSS_THREAD_FLAGS_SYSTEM) != 0);
	}

	/************************************************************************/
	/*                        GET_FIELD_IN_STRUCT                           */
	/************************************************************************/

	PVOID PspGetProcessSectionObject(PEPROCESS Process)
	{
		PVOID pValue = nullptr;
		GET_FIELD_IN_STRUCT(PVOID, PEPROCESS, Process, SectionObject);
		pValue = fnGetField();
		return pValue;
	}

	PVOID PspGetProcessSectionBaseAddress(PEPROCESS Process)
	{
		PVOID pValue = nullptr;
		GET_FIELD_IN_STRUCT(PVOID, PEPROCESS, Process, SectionBaseAddress);
		pValue = fnGetField();
		return pValue;
	}

	PVOID PspGetThreadStartAddress(PETHREAD Thread)
	{
		PVOID pValue = nullptr;
		GET_FIELD_IN_STRUCT(PVOID, PETHREAD, Thread, StartAddress);
		pValue = fnGetField();
		return pValue;
	}

	PVOID PspGetThreadWin32StartAddress(PETHREAD Thread)
	{
		PVOID pValue = nullptr;
		GET_FIELD_IN_STRUCT(PVOID, PETHREAD, Thread, Win32StartAddress);
		pValue = fnGetField();
		return pValue;
	}

	PVOID PsGetProcessDebugPortPointer(PEPROCESS Process)
	{
		GET_POINTER_IN_STRUCT(PEPROCESS, Process, DebugPort);
		return static_cast<PULONG_PTR>(fnGetPointer());
	}

	PVOID PsGetProcessFlagsPointer(PEPROCESS Process)
	{
		GET_POINTER_IN_STRUCT(PEPROCESS, Process, Flags);
		return static_cast<PULONG_PTR>(fnGetPointer());
	}

	ULONG32 PsGetProcessFlags(PEPROCESS Process)
	{
		ULONG32 Value = 0;
		GET_FIELD_IN_STRUCT(ULONG32, PEPROCESS, Process, Flags);
		Value = fnGetField();
		return Value;
	}

	UINT32 GetThreadClonedThread(PETHREAD Thread)
	{
		UINT32 ClonedThread = 0;
		GET_FIELD_IN_STRUCT(UINT32, PETHREAD, Thread, ClonedThread);
		ClonedThread = fnGetField();
		return ClonedThread;
	}

	UINT32 GetThreadInserted(PETHREAD Thread)
	{
		UINT32 ThreadInserted = 0;
		GET_FIELD_IN_STRUCT(UINT32, PETHREAD, Thread, ThreadInserted);
		ThreadInserted = fnGetField();
		return ThreadInserted;
	}

	ULONG32 GetActiveThreads(PEPROCESS Process)
	{
		GET_FIELD_IN_STRUCT(ULONG32, PEPROCESS, Process, ActiveThreads);
		return fnGetField();
	}

	INT32 GetProcessExitStatus(PEPROCESS Process)
	{
		GET_FIELD_IN_STRUCT(INT32, PEPROCESS, Process, ExitStatus);
		return fnGetField();
	}

	NTSTATUS GetThreadExitStatus(PETHREAD Thread)
	{
		GET_FIELD_IN_STRUCT(NTSTATUS, PETHREAD, Thread, ExitStatus);
		return fnGetField();
	}

	INT32 GetLastThreadExitStatus(PEPROCESS Process)
	{
		GET_FIELD_IN_STRUCT(INT32, PEPROCESS, Process, LastThreadExitStatus);
		return fnGetField();
	}

	/************************************************************************/
	/*                        GET_POINTER_IN_STRUCT                         */
	/************************************************************************/

	CLIENT_ID GetThreadCid(PETHREAD Thread) 
	{
		CLIENT_ID cid{};
		cid.UniqueProcess = PsGetThreadProcessId(Thread);
		cid.UniqueThread = PsGetThreadId(Thread);
		return cid;
	}

	PCLIENT_ID PspGetThreadCIDPointer(PETHREAD Thread)
	{
		PCLIENT_ID Pointer = nullptr;
		GET_POINTER_IN_STRUCT(PETHREAD, Thread, Cid);
		Pointer = static_cast<PCLIENT_ID>(fnGetPointer());
		return Pointer;
	}

	PEX_RUNDOWN_REF GetExAcquireRundownProtection(PETHREAD Thread)
	{
		PEX_RUNDOWN_REF Pointer = nullptr;
		GET_POINTER_IN_STRUCT(PETHREAD, Thread, RundownProtect);
		Pointer = static_cast<PEX_RUNDOWN_REF>(fnGetPointer());
		return Pointer;
	}

	PULONG_PTR GetCrossThreadFlagsPointer(PETHREAD Thread)
	{
		GET_POINTER_IN_STRUCT(PETHREAD, Thread, CrossThreadFlags);
		return static_cast<PULONG_PTR>(fnGetPointer());
	}

	PEX_RUNDOWN_REF GetThreadRundownProtectPointer(PETHREAD Thread)
	{
		GET_POINTER_IN_STRUCT(PETHREAD, Thread, RundownProtect);
		return static_cast<PEX_RUNDOWN_REF>(fnGetPointer());
	}

	PEX_RUNDOWN_REF GetProcessRundownProtectPointer(PEPROCESS Process)
	{
		GET_POINTER_IN_STRUCT(PEPROCESS, Process, RundownProtect);
		return static_cast<PEX_RUNDOWN_REF>(fnGetPointer());
	}

	ULONG32 GetCrossThreadFlags(PETHREAD Thread)
	{
		auto vFlags = ULONG32();
		GET_POINTER_IN_STRUCT(PETHREAD, Thread, CrossThreadFlags);

		vFlags = *((ULONG32*)fnGetPointer());

		if (!IsWindows8OrGreater())
		{
			vFlags &= 0xF;

			if (IsWindows7SP1OrGreater())
			{
				if (reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->HardErrorsAreDisabled)
				{
					vFlags |= PsCrossThreadFlagsHardErrorsAreDisabled;
				}
				if (reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->BreakOnTermination)
				{
					vFlags |= PsCrossThreadFlagsBreakOnTermination;
				}
				if (reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->SkipCreationMsg)
				{
					vFlags |= PsCrossThreadFlagsSkipCreationMsg;
				}
				if (reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->SkipTerminationMsg)
				{
					vFlags |= PsCrossThreadFlagsSkipTerminationMsg;
				}
			}
			else
			{
				if (reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->HardErrorsAreDisabled)
				{
					vFlags |= PsCrossThreadFlagsHardErrorsAreDisabled;
				}
				if (reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->BreakOnTermination)
				{
					vFlags |= PsCrossThreadFlagsBreakOnTermination;
				}
				if (reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->SkipCreationMsg)
				{
					vFlags |= PsCrossThreadFlagsSkipCreationMsg;
				}
				if (reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->SkipTerminationMsg)
				{
					vFlags |= PsCrossThreadFlagsSkipTerminationMsg;
				}
			}
		}

		return vFlags;
	}


	VOID SetCrossThreadFlags(PETHREAD Thread, UINT32 aFlags)
	{
		auto vFlags = GetCrossThreadFlagsPointer(Thread);

		if (vFlags)
		{
			if (IsWindows8OrGreater())
			{
				RtlInterlockedSetBitsDiscardReturn(vFlags, aFlags);
			}
			else
			{
				if (IsWindows7SP1OrGreater())
				{
					if (aFlags & PsCrossThreadFlagsHardErrorsAreDisabled)
					{
						aFlags &= ~PsCrossThreadFlagsHardErrorsAreDisabled;
						reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->HardErrorsAreDisabled = true;
					}
					if (aFlags & PsCrossThreadFlagsBreakOnTermination)
					{
						aFlags &= ~PsCrossThreadFlagsBreakOnTermination;
						reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->BreakOnTermination = true;
					}
					if (aFlags & PsCrossThreadFlagsSkipCreationMsg)
					{
						aFlags &= ~PsCrossThreadFlagsSkipCreationMsg;
						reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->SkipCreationMsg = true;
					}
					if (aFlags & PsCrossThreadFlagsSkipTerminationMsg)
					{
						aFlags &= ~PsCrossThreadFlagsSkipTerminationMsg;
						reinterpret_cast<wdk2::build_7601::PETHREAD>(Thread)->SkipTerminationMsg = true;
					}
				}
				else
				{
					if (aFlags & PsCrossThreadFlagsHardErrorsAreDisabled)
					{
						aFlags &= ~PsCrossThreadFlagsHardErrorsAreDisabled;
						reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->HardErrorsAreDisabled = true;
					}
					if (aFlags & PsCrossThreadFlagsBreakOnTermination)
					{
						aFlags &= ~PsCrossThreadFlagsBreakOnTermination;
						reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->BreakOnTermination = true;
					}
					if (aFlags & PsCrossThreadFlagsSkipCreationMsg)
					{
						aFlags &= ~PsCrossThreadFlagsSkipCreationMsg;
						reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->SkipCreationMsg = true;
					}
					if (aFlags & PsCrossThreadFlagsSkipTerminationMsg)
					{
						aFlags &= ~PsCrossThreadFlagsSkipTerminationMsg;
						reinterpret_cast<wdk2::build_7600::PETHREAD>(Thread)->SkipTerminationMsg = true;
					}
				}

				RtlInterlockedSetBitsDiscardReturn(vFlags, aFlags);
			}
		}
	}

	VOID SetProcessDebugPort(PEPROCESS Process, PDEBUG_OBJECT Port)
	{
		GET_OFFSET_IN_STRUCT(EPROCESS, DebugPort);

		*reinterpret_cast<PDEBUG_OBJECT*>((char*)Process + fnGetOffset()) = Port;
	}

	PLARGE_INTEGER GetProcessExitTimePointer(PEPROCESS Process)
	{
		GET_POINTER_IN_STRUCT(PEPROCESS, Process, ExitTime);
		return static_cast<PLARGE_INTEGER>(fnGetPointer());
	}

	/************************************************************************/
	/*                        GET_FIELD_IN_STRUCT2                          */
	/************************************************************************/
	PVOID PspGetThreadTebPointer(PETHREAD Thread)
	{
		PVOID Teb = nullptr;
		GET_FIELD_IN_STRUCT2(PVOID, PETHREAD, Thread, Tcb, Teb);
		Teb = fnGetField();
		return Teb;
	}

	UINT8 GetThreadApcStateIndex(PETHREAD Thread)
	{
		UINT8 ApcStateIndex = 0;
		GET_FIELD_IN_STRUCT2(UINT8, PETHREAD, Thread, Tcb, ApcStateIndex);
		ApcStateIndex = fnGetField();
		return ApcStateIndex;
	}

	PVOID GetThreadInitialStack(PETHREAD Thread)
	{
		PVOID InitialStack = nullptr;
		GET_FIELD_IN_STRUCT2(PVOID, PETHREAD, Thread, Tcb, InitialStack);
		InitialStack = fnGetField();
		return InitialStack;
	}

	PVOID GetThreadProcess(PETHREAD Thread)
	{
		PVOID Process = nullptr;
		GET_FIELD_IN_STRUCT2(PVOID, PETHREAD, Thread, Tcb, Process);
		Process = fnGetField();
		return Process;
	}

	/************************************************************************/
	/*                         单独更新                                     */
	/************************************************************************/
	PEPROCESS PsGetProcessByThread(PETHREAD Thread)
	{
		PEPROCESS Process = nullptr;
		wdk::KAPC_STATE kApcState = {nullptr};
		ULONG xresult = 0;
		switch (GetSystemVersion())
		{
		default:
			LOG_DEBUG("unkown windows version!\r\n");
			return nullptr;
		case SystemVersion::Windows7:
			xresult = GET_OFFSET_64(wdk2::build_7600::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread) + xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_7600::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows7_SP1:
			xresult = GET_OFFSET_64(wdk2::build_7601::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread) + xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_7601::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows8:
			xresult = GET_OFFSET_64(wdk2::build_9200::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_9200::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows8_1:
			xresult = GET_OFFSET_64(wdk2::build_9600::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_9600::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1507:
			xresult = GET_OFFSET_64(wdk2::build_10240::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_10240::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1511:
			xresult = GET_OFFSET_64(wdk2::build_10586::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_10586::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1607:
			xresult = GET_OFFSET_64(wdk2::build_14393::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_14393::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1703:
			xresult = GET_OFFSET_64(wdk2::build_15063::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_15063::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1709:
			xresult = GET_OFFSET_64(wdk2::build_16299::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_16299::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1803:
			xresult = GET_OFFSET_64(wdk2::build_17134::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_17134::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1809:
			xresult = GET_OFFSET_64(wdk2::build_17763::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_17763::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1903:
			xresult = GET_OFFSET_64(wdk2::build_18362::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_18362::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_1909:
			xresult = GET_OFFSET_64(wdk2::build_18363::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE *)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_18363::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_2004:
			xresult = GET_OFFSET_64(wdk2::build_19041::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_19041::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_20H2:
			xresult = GET_OFFSET_64(wdk2::build_19042::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_19042::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_21H1:             // 10.0.19043
			xresult = GET_OFFSET_64(wdk2::build_19043::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_19043::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_21H2:             // 10.0.19044
			xresult = GET_OFFSET_64(wdk2::build_19044::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_19044::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_22H2:             // 10.0.19045
			xresult = GET_OFFSET_64(wdk2::build_19045::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_19045::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_20H2_19569: 
			xresult = GET_OFFSET_64(wdk2::build_19569::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_19569::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_21H1_20150: 
			xresult = GET_OFFSET_64(wdk2::build_20150::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_20150::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_21H1_20180: 
			xresult = GET_OFFSET_64(wdk2::build_20180::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_20180::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows10_XXXX_21292: 
			xresult = GET_OFFSET_64(wdk2::build_21292::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_21292::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows11_21H2:             // 11.0.22000
			xresult = GET_OFFSET_64(wdk2::build_22000::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_22000::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows11_22H2:             // 11.0.22621
			xresult = GET_OFFSET_64(wdk2::build_22621::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_22621::EPROCESS, Pcb));
			break;
		case SystemVersion::Windows11_23H2:             // 11.0.22631
			xresult = GET_OFFSET_64(wdk2::build_22631::KTHREAD, ApcState);
			kApcState = *(wdk::KAPC_STATE*)((PCHAR)(Thread)+xresult);
			Process = (PEPROCESS)(CONTAINING_RECORD(kApcState.Process, wdk2::build_22631::EPROCESS, Pcb));
			break;
		}
		return Process;
	}

	VOID SetProcessDebugActive(PEPROCESS Process, BOOL DebugActive)
	{
		ULONG xresult = 0;

		switch (GetSystemVersion())
		{
		default:
			LOG_DEBUG("unkown windows version!\r\n");
			return;
		case SystemVersion::Windows7:
			xresult = GET_OFFSET_64(wdk2::build_7600::KPROCESS, Header);
			break;
		case SystemVersion::Windows7_SP1:
			xresult = GET_OFFSET_64(wdk2::build_7601::KPROCESS, Header);
			break;
		case SystemVersion::Windows8:
			xresult = GET_OFFSET_64(wdk2::build_9200::KPROCESS, Header);
			break;
		case SystemVersion::Windows8_1:
			xresult = GET_OFFSET_64(wdk2::build_9600::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1507:
			xresult = GET_OFFSET_64(wdk2::build_10240::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1511:
			xresult = GET_OFFSET_64(wdk2::build_10586::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1607:
			xresult = GET_OFFSET_64(wdk2::build_14393::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1703:
			xresult = GET_OFFSET_64(wdk2::build_15063::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1709:
			xresult = GET_OFFSET_64(wdk2::build_16299::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1803:
			xresult = GET_OFFSET_64(wdk2::build_17134::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1809:
			xresult = GET_OFFSET_64(wdk2::build_17763::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1903:
			xresult = GET_OFFSET_64(wdk2::build_18362::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1909:
			xresult = GET_OFFSET_64(wdk2::build_18363::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_2004:
			xresult = GET_OFFSET_64(wdk2::build_19041::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_20H2:
			xresult = GET_OFFSET_64(wdk2::build_19042::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H1:             // 10.0.19043
			xresult = GET_OFFSET_64(wdk2::build_19043::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H2:             // 10.0.19044
			xresult = GET_OFFSET_64(wdk2::build_19044::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_22H2:             // 10.0.19045
			xresult = GET_OFFSET_64(wdk2::build_19045::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_20H2_19569:
			xresult = GET_OFFSET_64(wdk2::build_19569::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H1_20150:
			xresult = GET_OFFSET_64(wdk2::build_20150::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H1_20180:
			xresult = GET_OFFSET_64(wdk2::build_20180::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_XXXX_21292:
			xresult = GET_OFFSET_64(wdk2::build_21292::KPROCESS, Header);
			break;
		case SystemVersion::Windows11_21H2:             // 11.0.22000
			xresult = GET_OFFSET_64(wdk2::build_22000::KPROCESS, Header);
			break;
		case SystemVersion::Windows11_22H2:             // 11.0.22621
			xresult = GET_OFFSET_64(wdk2::build_22621::KPROCESS, Header);
			break;
		case SystemVersion::Windows11_23H2:             // 11.0.22631
			xresult = GET_OFFSET_64(wdk2::build_22631::KPROCESS, Header);
			break;
		}

		if (Process && xresult)
		{
			*(UCHAR*)((PCHAR)(Process)+xresult) = DebugActive;
		}
	}

	BOOLEAN GetProcessDebugActive(PEPROCESS Process)
	{
		ULONG xresult = 0;

		switch (GetSystemVersion())
		{
		default:
			LOG_DEBUG("unkown windows version!\r\n");
			return FALSE;
		case SystemVersion::Windows7:
			xresult = GET_OFFSET_64(wdk2::build_7600::KPROCESS, Header);
			break;
		case SystemVersion::Windows7_SP1:
			xresult = GET_OFFSET_64(wdk2::build_7601::KPROCESS, Header);
			break;
		case SystemVersion::Windows8:
			xresult = GET_OFFSET_64(wdk2::build_9200::KPROCESS, Header);
			break;
		case SystemVersion::Windows8_1:
			xresult = GET_OFFSET_64(wdk2::build_9600::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1507:
			xresult = GET_OFFSET_64(wdk2::build_10240::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1511:
			xresult = GET_OFFSET_64(wdk2::build_10586::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1607:
			xresult = GET_OFFSET_64(wdk2::build_14393::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1703:
			xresult = GET_OFFSET_64(wdk2::build_15063::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1709:
			xresult = GET_OFFSET_64(wdk2::build_16299::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1803:
			xresult = GET_OFFSET_64(wdk2::build_17134::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1809:
			xresult = GET_OFFSET_64(wdk2::build_17763::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1903:
			xresult = GET_OFFSET_64(wdk2::build_18362::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_1909:
			xresult = GET_OFFSET_64(wdk2::build_18363::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_2004:
			xresult = GET_OFFSET_64(wdk2::build_19041::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_20H2:
			xresult = GET_OFFSET_64(wdk2::build_19042::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H1:             // 10.0.19043
			xresult = GET_OFFSET_64(wdk2::build_19043::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H2:             // 10.0.19044
			xresult = GET_OFFSET_64(wdk2::build_19044::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_22H2:             // 10.0.19045
			xresult = GET_OFFSET_64(wdk2::build_19045::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_20H2_19569:
			xresult = GET_OFFSET_64(wdk2::build_19569::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H1_20150:
			xresult = GET_OFFSET_64(wdk2::build_20150::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_21H1_20180:
			xresult = GET_OFFSET_64(wdk2::build_20180::KPROCESS, Header);
			break;
		case SystemVersion::Windows10_XXXX_21292:
			xresult = GET_OFFSET_64(wdk2::build_21292::KPROCESS, Header);
			break;
		case SystemVersion::Windows11_21H2:             // 11.0.22000
			xresult = GET_OFFSET_64(wdk2::build_22000::KPROCESS, Header);
			break;
		case SystemVersion::Windows11_22H2:             // 11.0.22621
			xresult = GET_OFFSET_64(wdk2::build_22621::KPROCESS, Header);
			break;
		case SystemVersion::Windows11_23H2:             // 11.0.22631
			xresult = GET_OFFSET_64(wdk2::build_22631::KPROCESS, Header);
			break;
		}

		if (Process && xresult)
		{
			return (BOOLEAN)(*(UCHAR*)((PCHAR)(Process)+xresult));
		}

		return FALSE;
	}

	BOOL IsSystemThread(PETHREAD Thread)
	{
		BOOL SystemThread = 0;

		switch (GetSystemVersion())
		{
		default:
			LOG_DEBUG("unkown windows version!\r\n");
			return 0;
		case SystemVersion::Windows7:
			SystemThread = ((wdk2::build_7600::PETHREAD)Thread)->SystemThread;
			break;
		case SystemVersion::Windows7_SP1:
			SystemThread = ((wdk2::build_7601::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows8:
			SystemThread = ((wdk2::build_9200::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows8_1:
			SystemThread = ((wdk2::build_9600::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1507:
			SystemThread = ((wdk2::build_10240::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1511:
			SystemThread = ((wdk2::build_10586::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1607:
			SystemThread = ((wdk2::build_14393::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1703:
			SystemThread = ((wdk2::build_15063::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1709:
			SystemThread = ((wdk2::build_16299::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1803:
			SystemThread = ((wdk2::build_17134::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1809:
			SystemThread = ((wdk2::build_17763::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1903:
			SystemThread = ((wdk2::build_18362::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_1909:
			SystemThread = ((wdk2::build_18363::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_2004:
			SystemThread = ((wdk2::build_19041::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_20H2:
			SystemThread = ((wdk2::build_19042::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_21H1:             // 10.0.19043
			SystemThread = ((wdk2::build_19043::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_21H2:             // 10.0.19044
			SystemThread = ((wdk2::build_19044::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_22H2:             // 10.0.19045
			SystemThread = ((wdk2::build_19045::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_20H2_19569: 
			SystemThread = ((wdk2::build_19569::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_21H1_20150: 
			SystemThread = ((wdk2::build_20150::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_21H1_20180: 
			SystemThread = ((wdk2::build_20180::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows10_XXXX_21292: 
			SystemThread = ((wdk2::build_21292::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows11_21H2:             // 11.0.22000
			SystemThread = ((wdk2::build_22000::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows11_22H2:             // 11.0.22621
			SystemThread = ((wdk2::build_22621::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		case SystemVersion::Windows11_23H2:             // 11.0.22631
			SystemThread = ((wdk2::build_22631::PETHREAD)Thread)->Tcb.SystemThread;
			break;
		}
		return SystemThread;
	}

	//SSDT相关
	BOOL Get_SSDT_Numbers()
	{
	#define INIT_SSDT_NUMBER(F) \
			{ \
				auto fnGetIndex = GET_SDT_INDEX(SSDT_INDEX, F); \
				g_SyscallNumbers.F = fnGetIndex(); \
				if(g_SyscallNumbers.F == NULL) \
				{                              \
					return FALSE;              \
				}                              \
			}

		INIT_SSDT_NUMBER(NtSetInformationThread);
		INIT_SSDT_NUMBER(NtQueryInformationProcess);
		INIT_SSDT_NUMBER(NtSetContextThread);
		INIT_SSDT_NUMBER(NtQuerySystemInformation);
		INIT_SSDT_NUMBER(NtGetContextThread);
		INIT_SSDT_NUMBER(NtClose);
		INIT_SSDT_NUMBER(NtQueryInformationThread);
		INIT_SSDT_NUMBER(NtCreateThreadEx);
		INIT_SSDT_NUMBER(NtCreateFile);
		INIT_SSDT_NUMBER(NtCreateProcessEx);
		INIT_SSDT_NUMBER(NtYieldExecution);
		INIT_SSDT_NUMBER(NtQuerySystemTime);
		INIT_SSDT_NUMBER(NtQueryPerformanceCounter);
		INIT_SSDT_NUMBER(NtContinue);
		INIT_SSDT_NUMBER(NtQueryInformationJobObject);
		INIT_SSDT_NUMBER(NtCreateUserProcess);
		INIT_SSDT_NUMBER(NtGetNextProcess);
		INIT_SSDT_NUMBER(NtOpenProcess);
		INIT_SSDT_NUMBER(NtOpenThread);
		INIT_SSDT_NUMBER(NtSetInformationProcess);

		return TRUE;
	}

	BOOL Get_ShadowSSDT_Numbers()
	{
	#define INIT_SSSDT_NUMBER(F) \
			{ \
				auto fnGetIndex = GET_SDT_INDEX(SHADOW_SSDT_INDEX, F); \
				g_Win32KSyscallNumbers.F = fnGetIndex() - 4096; \
				if(g_Win32KSyscallNumbers.F == NULL) \
				{                              \
					return FALSE;              \
				}                              \
			}

		INIT_SSSDT_NUMBER(NtUserGetThreadState);
		INIT_SSSDT_NUMBER(NtUserQueryWindow);
		INIT_SSSDT_NUMBER(NtUserWindowFromPoint);
		INIT_SSSDT_NUMBER(NtUserBuildHwndList);
		INIT_SSSDT_NUMBER(NtUserGetForegroundWindow);
		INIT_SSSDT_NUMBER(NtUserInternalGetWindowText);
		INIT_SSSDT_NUMBER(NtUserFindWindowEx);
		INIT_SSSDT_NUMBER(NtUserGetClassName);
		INIT_SSSDT_NUMBER(NtUserChildWindowFromPointEx);
		INIT_SSSDT_NUMBER(NtUserRealChildWindowFromPoint);

		return TRUE;
	}

	BOOL Get_Offsets_InStruct()
	{
		BOOL bRet = FALSE;
		//自己按特定操作系统修改
		if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_XXXX_21292 || 
			g_CurrentWindowsBuildNumber == WINDOWS_11_VERSION_21H2 || g_CurrentWindowsBuildNumber == WINDOWS_11_VERSION_22H2 || g_CurrentWindowsBuildNumber == WINDOWS_11_VERSION_23H2)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0x74;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x560;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x560;
			g_NtapiOffsets.Offset_PicoContext =  0x630;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x460;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_21H1 || g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_21H2 || 
			g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_22H1 || g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_20H2_19569 ||
			g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_21H1_20150 || g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_21H1_20180 ||

			g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_20H2 || g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_20H1)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0x74;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x510;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x510;
			g_NtapiOffsets.Offset_PicoContext = 0x5e0;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x460;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_19H2 || g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_19H1)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0x74;     //从这个版本出现了  https://www.freebuf.com/articles/system/260373.html
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6e0;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6e0;
			g_NtapiOffsets.Offset_PicoContext = 0x7a8;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x308;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_REDSTONE5)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6d0;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6d0;
			g_NtapiOffsets.Offset_PicoContext = 0x798;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x300;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_REDSTONE4)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6d0;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6d0;
			g_NtapiOffsets.Offset_PicoContext = 0x7a0;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x300;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_REDSTONE3)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6d0;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6d0;
			g_NtapiOffsets.Offset_PicoContext = 0x7a0;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x300;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_REDSTONE2)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6c8;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6c8;
			g_NtapiOffsets.Offset_PicoContext = 0x798;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0x810;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_REDSTONE1)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6c0;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6c0;
			g_NtapiOffsets.Offset_PicoContext = 0x790;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_THRESHOLD2)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6bc;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6bc;
			g_NtapiOffsets.Offset_PicoContext = 0x788;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_10_VERSION_THRESHOLD1)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6bc;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6bc;
			g_NtapiOffsets.Offset_PicoContext = 0x788;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_8_1)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x6b4;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x6b4;
			g_NtapiOffsets.Offset_PicoContext = 0x770;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0;
			bRet = TRUE;
		}
		else if (g_CurrentWindowsBuildNumber == WINDOWS_8)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x42c;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x42c;
			g_NtapiOffsets.Offset_PicoContext = 0x770;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0;
			bRet = TRUE;
		}

		else if (g_CurrentWindowsBuildNumber == WINDOWS_7_SP1 || g_CurrentWindowsBuildNumber == WINDOWS_7)
		{
			g_NtapiOffsets.Offset_BypassProcessFreezeFlag = 0;
			g_NtapiOffsets.Offset_ThreadHideFromDebuggerFlag = 0x448;
			g_NtapiOffsets.Offset_ThreadBreakOnTerminationFlag = 0x448;
			g_NtapiOffsets.Offset_PicoContext = 0;
			g_NtapiOffsets.Offset_RestrictSetThreadContext = 0;
			bRet = TRUE;
		}

		if (bRet)
		{
	#define INIT_OFFSET_IN_STRUCT(S, F) \
			{ \
				GET_OFFSET_IN_STRUCT(S, F); \
				g_NtapiOffsets.Offset_##F = fnGetOffset(); \
			}

			INIT_OFFSET_IN_STRUCT(EPROCESS, ObjectTable);         //14393(EPROCESS + 0x418)
			INIT_OFFSET_IN_STRUCT(EPROCESS, SectionBaseAddress);  //14393(EPROCESS + 0x3C0)
			INIT_OFFSET_IN_STRUCT(EPROCESS, SectionObject);       //14393(EPROCESS + 0x3B8)
			INIT_OFFSET_IN_STRUCT(EPROCESS, SeAuditProcessCreationInfo);

			INIT_OFFSET_IN_STRUCT(ETHREAD, Win32StartAddress);   //14393(ETHREAD  + 0x688)
			INIT_OFFSET_IN_STRUCT(KTHREAD, PreviousMode);        //14393(KTHREAD  + 0x232)
			INIT_OFFSET_IN_STRUCT(KTHREAD, SuspendCount);        //14393(KTHREAD  + 0x284)

			if (g_NtapiOffsets.Offset_ObjectTable)
			{
				LOG_DEBUG("[+] EPROCESS_Offset--->ObjectTable:0x%X\r\n", g_NtapiOffsets.Offset_ObjectTable);
			}
			else
			{
				LOG_DEBUG("[-] EPROCESS_Offset--->ObjectTable\r\n");
			}
		
			if (g_NtapiOffsets.Offset_SectionBaseAddress)
			{
				LOG_DEBUG("[+] EPROCESS_Offset--->SectionBaseAddress:0x%X\r\n", g_NtapiOffsets.Offset_SectionBaseAddress);
			}
			else
			{
				LOG_DEBUG("[-] EPROCESS_Offset--->SectionBaseAddress\r\n");
			}
		
			if (g_NtapiOffsets.Offset_SectionObject)
			{
				LOG_DEBUG("[+] EPROCESS_Offset--->SectionObject:0x%X\r\n", g_NtapiOffsets.Offset_SectionObject);
			}
			else
			{
				LOG_DEBUG("[-] EPROCESS_Offset--->SectionObject\r\n");
			}

			if (g_NtapiOffsets.Offset_SeAuditProcessCreationInfo)
			{
				LOG_DEBUG("[+] EPROCESS_Offset--->SeAuditProcessCreationInfo:0x%X\r\n", g_NtapiOffsets.Offset_SeAuditProcessCreationInfo);
			}
			else
			{
				LOG_DEBUG("[-] EPROCESS_Offset--->SeAuditProcessCreationInfo\r\n");
			}

			if (g_NtapiOffsets.Offset_Win32StartAddress)
			{
				LOG_DEBUG("[+] ETHREAD_Offset--->Win32StartAddress:0x%X\r\n", g_NtapiOffsets.Offset_Win32StartAddress);
			}
			else
			{
				LOG_DEBUG("[-] ETHREAD_Offset--->Win32StartAddress\r\n");
			}
		
			if (g_NtapiOffsets.Offset_PreviousMode)
			{
				LOG_DEBUG("[+] KTHREAD_Offset--->PreviousMode:0x%X\r\n", g_NtapiOffsets.Offset_PreviousMode);
			}
			else
			{
				LOG_DEBUG("[-] KTHREAD_Offset--->PreviousMode\r\n");
			}

			if (g_NtapiOffsets.Offset_SuspendCount)
			{
				LOG_DEBUG("[+] KTHREAD_Offset--->SuspendCount:0x%X\r\n", g_NtapiOffsets.Offset_SuspendCount);
			}
			else
			{
				LOG_DEBUG("[-] KTHREAD_Offset--->SuspendCount\r\n");
			}
		}

		return bRet;
	}
}