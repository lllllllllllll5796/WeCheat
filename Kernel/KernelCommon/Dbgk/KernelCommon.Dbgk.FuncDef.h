#pragma once

using fnDbgkpWakeTarget = VOID(NTAPI*)(IN PDEBUG_EVENT DebugEvent);

using fnDbgkpSuppressDbgMsg = BOOLEAN(NTAPI*)(PVOID teb);

#if WIN7
using fnDbgkpSendApiMessage = NTSTATUS(NTAPI*)(BOOLEAN SuspendProcess, PDBGKM_APIMSG ApiMsg);
#else
using fnDbgkpSendApiMessage = NTSTATUS(NTAPI*)(PEPROCESS Process, BOOLEAN SuspendProcess, PDBGKM_APIMSG ApiMsg);
#endif

using fnDbgkpMarkProcessPeb = VOID(NTAPI*)(PEPROCESS Process);

using fnDbgkpSendErrorMessage = NTSTATUS(NTAPI*)(PEXCEPTION_RECORD ExceptionRecord, ULONG Falge, PDBGKM_APIMSG ApiMsg);

using fnPsGetNextProcessThread = PETHREAD(NTAPI*)(PEPROCESS  Process, PETHREAD Thread);

using fnDbgkpSendApiMessageLpc = NTSTATUS(NTAPI*)(PDBGKM_APIMSG ApiMsg, PVOID Port, BOOLEAN SuspendProcess);

using fnPsCaptureExceptionPort = PVOID(NTAPI*)(PEPROCESS Process);

using fnDbgkpSetProcessDebugObject = NTSTATUS(NTAPI*)(PEPROCESS DebuggedProcess, PDEBUG_OBJECT DebugObject, NTSTATUS MsgStatus, PETHREAD LastThread);

using fnDbgkpSectionToFileHandle = HANDLE(NTAPI*)(PVOID SectionObject);

using fnDbgkSendSystemDllMessages = VOID(NTAPI*)(PETHREAD Thread, PDEBUG_OBJECT	DebugObject, PDBGKM_APIMSG ApiMsg);

using fnDbgkpPostFakeThreadMessages = NTSTATUS(NTAPI*)(PEPROCESS Process, PDEBUG_OBJECT DebugObject, PETHREAD StartThread, PETHREAD* pFirstThread, PETHREAD* pLastThread);

using fnDbgkpPostFakeProcessCreateMessages = NTSTATUS(NTAPI*)(PEPROCESS Process, PDEBUG_OBJECT DebugObject, PETHREAD* pLastThread);

using fnDbgkpQueueMessage = NTSTATUS(NTAPI*)(PEPROCESS Process, PETHREAD Thread, PDBGKM_APIMSG ApiMsg, ULONG Flags, PDEBUG_OBJECT DebugObject);

using fnKiDispatchException = VOID(NTAPI*)(PEXCEPTION_RECORD ExceptionRecord, void* ExceptionFrame, void* TrapFrame, KPROCESSOR_MODE PreviousMode, BOOLEAN FirstChance);

using fnRtlInitializeExtendedContext = NTSTATUS(__fastcall*)(
	_In_    PVOID     pContext,
	_In_    ULONG     ContextFlags,
	_Out_   PCONTEXT_EX* ContextEx);

using fnRtlGetExtendedContextLength = NTSTATUS(__fastcall*)(
	_In_    ULONG ContextFlags,
	_Out_   PULONG pContextLength);

using fnRtlpCopyExtendedContext = NTSTATUS(__fastcall* )(_In_ BOOLEAN isCopy,
	_Inout_   PCONTEXT_EX DestContextEx,
	_In_      PCONTEXT_EX target_offset,
	_In_      ULONG ContextFlags,
	_In_      PCONTEXT_EX SrcContextEx,
	_In_      PCONTEXT_EX src_offset);

using fnNtShutdownSystem = NTSTATUS(__fastcall*)(SHUTDOWN_ACTION);

using fnObCreateObjectType = NTSTATUS(__fastcall*)(
	_In_ PUNICODE_STRING TypeName,
	_In_ POBJECT_TYPE_INITIALIZER ObjectTypeInitializer,
	_In_opt_ ::PSECURITY_DESCRIPTOR SecurityDescriptor,
	_Out_ POBJECT_TYPE* ObjectType);

using fnSeAuditingWithTokenForSubcategory = BOOLEAN(__fastcall* )(_In_ ULONG AuditCategory,
	_In_opt_ PACCESS_TOKEN Token);

using fnSeAuditProcessCreation = VOID(__fastcall*)(_In_ PEPROCESS Process, _In_ __int64 a2);

using fnPspGetJobFromSet = NTSTATUS(__fastcall*)(_In_ PEJOB ParentJob, _In_ ULONG JobMemberLevel, _Out_ PEJOB* pJob);

using fnPspBindProcessSessionToJob = NTSTATUS(__fastcall*)(_In_ PEJOB Job, _In_ PEPROCESS Process);

using fnPspAddProcessToJob = NTSTATUS(__fastcall* )(_In_ PEJOB Job, _In_ PEPROCESS Process);

using fnKiCheckForKernelApcDelivery = VOID(__fastcall*)();

using fnDbgkCopyProcessDebugPort = NTSTATUS(__fastcall*)(_In_ PEPROCESS TargetProcess,
	_In_ PEPROCESS SourceProcess,
	_Inout_ PDEBUG_OBJECT DebugObject,
	_Out_ PBOOLEAN boCopyDebugPort);

using fnSeCreateAccessStateEx = NTSTATUS(__fastcall*)(_In_opt_ PETHREAD Thread,
	_In_ PEPROCESS Process,
	_Out_ PACCESS_STATE AccessState,
	_Out_ PVOID AuxData,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ PGENERIC_MAPPING GenericMapping);

using fnObInsertObjectEx = NTSTATUS(__fastcall*)(_In_ PVOID Object,
	_In_opt_ PACCESS_STATE PassedAccessState,
	_In_opt_ ACCESS_MASK DesiredAccess,
	_In_ ULONG ObjectPointerBias,
	_In_ ULONG a5,
	_Out_opt_ PVOID* NewObject,
	_Out_opt_ PHANDLE Handle);

using fnPspValidateJobAffinityState = NTSTATUS(__fastcall*)(_In_ PEJOB Job,
	_In_ PEPROCESS Process);

using fnObCheckRefTraceProcess = NTSTATUS(__fastcall* )(_In_ PEPROCESS Process);

using fnSeDeleteAccessState = VOID(__fastcall*)(_In_ PACCESS_STATE AccessState);

using fnPspInsertProcess = NTSTATUS(__fastcall*)(_In_ PEPROCESS CurrentProcess,
	_In_ PEPROCESS ParentProcess,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ ULONG Flags,
	_In_ ULONG JobMemberLevel,
	_In_ HANDLE DebugObjectHandle,
	_In_ ULONG CrossThreadFlags,
	_In_ PVOID ObjectCreationState);

using fnPspInsertThread = NTSTATUS(__fastcall*)(_In_ PETHREAD Thread,
	_In_ PEPROCESS CurrentProcess,
	_In_ PVOID InitialTeb,
	_In_ ULONG ProcessFlags,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ PUCHAR Flag,
	_In_ PVOID CreateProcessContext,
	_In_ PVOID AccessState,
	_In_ PGROUP_AFFINITY GroupAffinity,
	_Out_ HANDLE* ProcessHandle,
	_In_ PCLIENT_ID ClientID);

using fnNtDebugContinue = NTSTATUS(__fastcall*)(
	IN HANDLE DebugObjectHandle,
	IN PCLIENT_ID ClientId,
	IN NTSTATUS ContinueStatus);

using fnDbgkMapViewOfSection = VOID(__fastcall*)(IN PEPROCESS Process, IN PVOID SectionObject, IN PVOID BaseAddress, IN ULONG SectionOffset, IN ULONG_PTR ViewSize);

using fnDbgkUnMapViewOfSection = VOID(__fastcall*)(IN PEPROCESS Process, IN PVOID BaseAddress);

using fnDbgkCreateThread = VOID(__fastcall*)(IN PETHREAD Thread);

using fnDbgkExitThread = VOID(__fastcall*)(IN NTSTATUS ExitStatus);

using fnDbgkExitProcess = VOID(__fastcall*)(IN NTSTATUS ExitStatus);

using fnPsSynchronizeWithThreadInsertion = VOID(__fastcall*)(IN PETHREAD Thread, IN PETHREAD CurrentThread);

using fnEtwTraceThread = VOID(__fastcall* )(_In_ PETHREAD Thread, _In_ PVOID InitialTeb, _In_ BOOLEAN bFlag);

using fnExReferenceCallBackBlock = PEX_CALLBACK_ROUTINE_BLOCK(__fastcall*)(_Inout_ PEX_CALLBACK CallBack);

using fnExGetCallBackBlockRoutine = PEX_CALLBACK_FUNCTION(__fastcall*)(_In_ PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock);

using fnExDereferenceCallBackBlock = VOID(__fastcall*)(_Inout_ PEX_CALLBACK CallBack, _In_ PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock);

using fnObReferenceObjectSafe = BOOLEAN(__fastcall*)(_In_ PVOID Object);

using fnPspCatchCriticalBreak = VOID(__fastcall*)(_In_ PCHAR Msg, _In_ PVOID Object, _In_ PUCHAR ImageFileName);

using fnLpcRequestPort = NTSTATUS(__fastcall*)(_In_ PVOID PortAddress, _In_ PPORT_MESSAGE RequestMessage);

using fnAlpcpSendMessage = NTSTATUS(__fastcall*)(_Inout_ PVOID a1, _In_ PLPC_CLIENT_DIED_MSG a2, _In_ __int64 a3, _In_ BOOLEAN a4);

using fnPspW32ThreadCallout = NTSTATUS(__fastcall*)(_In_ PETHREAD Thread, _In_ PSW32THREADCALLOUTTYPE CalloutType);

using fnPspW32ProcessCallout = NTSTATUS(__fastcall*)(_In_ PEPROCESS Process, _In_ BOOLEAN Initialize);

using fnIoCancelThreadIo = VOID(__fastcall* )(_In_ PETHREAD Thread);

using fnExTimerRundown = VOID(__fastcall*)();

using fnCmNotifyRunDown = VOID(__fastcall*)(_In_ PETHREAD Thread);

using fnKeRundownThread = VOID(__fastcall*)();

using fnPspUmsUnInitThread = NTSTATUS(__fastcall*)(_In_ PETHREAD Thread);

using fnKeRundownQueueEx = PLIST_ENTRY(__fastcall*)(_Inout_ PKQUEUE Queue, _In_ BOOLEAN a2);

using fnRtlFreeUserStack = NTSTATUS(__fastcall*)(_In_ PVOID DeallocationStack);

using fnMmDeleteTeb = VOID(__fastcall*)(_In_ PEPROCESS TargetProcess, _In_ PVOID TebBase);

using fnPspExitProcess = VOID(__fastcall* )(_In_ BOOLEAN LastThreadExit, _In_ PEPROCESS Process);

using fnSeAuditProcessExit = VOID(__fastcall*)(_In_ PEPROCESS Process, _In_ NTSTATUS ExitStatus);

using fnObKillProcess = VOID(__fastcall* )(_In_ PEPROCESS Process);

using fnPspExitProcessFromJob = VOID(__fastcall*)(_In_ PEJOB Job, _In_ PEPROCESS Process);

using fnKeFlushQueueApc = PLIST_ENTRY(__fastcall*)(_Inout_ PKTHREAD Thread, _In_ KPROCESSOR_MODE ProcessorMode);

using fnMmCleanProcessAddressSpace = VOID(__fastcall*)(_In_ PEPROCESS Process);

using fnLpcExitProcess = VOID(__fastcall*)(_In_ PEPROCESS Process);

using fnPspLegoNotifyRoutine = VOID(__fastcall*)(_In_ PKTHREAD Thread);

using fnKeSetProcess = LONG(__fastcall*)(_In_ PKPROCESS Process);

using fnKeTerminateThread = VOID(__fastcall*)(_In_ PETHREAD Thread);

using fnPspExitThread = VOID(__fastcall*)(_In_ NTSTATUS ExitStatus);

using fnPsGetNextProcess = PEPROCESS(__fastcall*)(_In_ PEPROCESS Process);

using fnPsTerminateProcess = NTSTATUS(__fastcall*)(_In_ PEPROCESS Process, _In_ NTSTATUS Status);

using fnDbgkpCloseObject = VOID(__fastcall*)(IN PEPROCESS Process, IN PVOID Object, IN ACCESS_MASK GrantedAccess, IN ULONG_PTR SystemHandleCount);

using fnKeStartThread = VOID(__fastcall*)(_In_ PETHREAD Thread, _In_opt_ PGROUP_AFFINITY GroupAffinity, _In_opt_ PULONG IdealProcessor);

using fnObReferenceObjectEx = LONG_PTR(__fastcall*)(_In_ PVOID Object, _In_ ULONG Count);

using fnPspInitializeCpuQuotaApc = VOID(__fastcall*)(_Out_ PVOID Apc, _In_ PKTHREAD Thread);

using fnPspAdjustThreadSecurityDescriptor = NTSTATUS(__fastcall*)(_In_ PETHREAD Thread);

using fnPspCreateThread = NTSTATUS(__fastcall*)(HANDLE* ThreadHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	HANDLE ProcessHandle,
	PEPROCESS arg_Process,
	__int64 CreateProcessContext,
	__int64 a7,
	__int64 a8,
	__int64 a9,
	unsigned int a10,
	PKSTART_ROUTINE StartRoutine,
	__int64 a12,
	__int64 a13);

using fnPsThawProcess = void(__fastcall* )(PEPROCESS a1, __int64 a2);

using fnEtwTraceProcess = VOID(__fastcall*)(_In_ PEPROCESS Process, _In_ ULONG a2);

using fnPspCreateObjectHandle = NTSTATUS(__fastcall*)(_In_ PVOID Object, _Inout_ PVOID AccessState, _In_ POBJECT_TYPE ObjectType);

using fnKeReadyThread = VOID(__fastcall*)(_Inout_ PKTHREAD Thread);

using fnMmDeleteKernelStack = VOID(__fastcall*)(_In_ PVOID PointerKernelStack, _In_ BOOLEAN LargeStack);

using fnPsSetProcessFaultInformation = void(__fastcall*)(PEPROCESS Process, int* a2);

using fnNtRemoveProcessDebug = NTSTATUS(__fastcall*)(
	IN HANDLE ProcessHandle,
	IN HANDLE DebugObjectHandle);


using fnKiStackAttachProcess = VOID(__fastcall*)(_KPROCESS* Process, BOOLEAN a2, ::PKAPC_STATE ApcState);

using fnKiUnstackDetachProcess = VOID(__fastcall*)(::PKAPC_STATE a1, int a2);

using fnDbgkpConvertKernelToUserStateChange = VOID(__fastcall*)(OUT PDBGUI_WAIT_STATE_CHANGE WaitStateChange, IN PDEBUG_EVENT DebugEvent);

using fnDbgkpOpenHandles = VOID(__fastcall*)(IN PDBGUI_WAIT_STATE_CHANGE WaitStateChange, IN PEPROCESS Process, IN PETHREAD Thread);

using fnKeCopyExceptionRecord = VOID(__fastcall*)(PEXCEPTION_RECORD dest, PEXCEPTION_RECORD source);

using fnObReferenceObjectByHandleWithTag = NTSTATUS(__fastcall*)(HANDLE Handle,
	int a2,
	struct _OBJECT_TYPE* a3,
	char a4,
	int a5,
	PVOID* Object,
	__int64 a7);

using fnObReferenceObjectByHandle = NTSTATUS(__fastcall*)(HANDLE Handle,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode,
	PVOID* Object,
	POBJECT_HANDLE_INFORMATION HandleInformation);

using fnObfDereferenceObjectWithTag = LONG_PTR(__fastcall*)(
	_In_ PVOID Object,
	_In_ ULONG Tag
	);

using fnObfDereferenceObject = LONG_PTR(__fastcall*)(
	_In_ PVOID Object
	);

using fnKeEnterCriticalRegionThread = VOID(__fastcall*)(_KTHREAD* Thread);

using fnKeLeaveCriticalRegionThread = VOID(__fastcall*)(_KTHREAD* Thread);

using fnMmCopyVirtualMemory = NTSTATUS(__fastcall*)(PEPROCESS FromProcess,
	PVOID FromAddress,
	PEPROCESS ToProcess,
	PVOID ToAddress,
	size_t BufferSize,
	KPROCESSOR_MODE PreviousMode,
	size_t* NumberOfBytesCopied);

using fnPspCreateUserContext = VOID(__fastcall*)(PCONTEXT context, char a2, PVOID ApcFunc, PVOID pUserfunc, __int64 a5);

using fnPspCallThreadNotifyRoutines = VOID(__fastcall*)(PETHREAD Thread, BOOLEAN Create, BOOLEAN a3);

using fnPspAllocateThread = NTSTATUS(__fastcall*)(PEPROCESS Process,
	POBJECT_ATTRIBUTES ObjectAttributes,
	unsigned __int8 a3,
	__int64 a4,
	__int64 a5,
	__int64 a6,
	PKSTART_ROUTINE StartRoutine,
	__int64 a8,
	__int64 a9,
	__int64 a10,
	__int64 a11,
	__int64 a12);

using fnObpReferenceObjectByHandleWithTag = NTSTATUS(__fastcall*)(HANDLE Handle,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode,
	ULONG Tag,
	PVOID* Object,
	POBJECT_HANDLE_INFORMATION HandleInformation,
	__int64 a8);

using fnMiObtainReferencedVadEx = PMMVAD(__fastcall*)(PVOID StartingAddress, char a2, PNTSTATUS status);

using fnMmProtectVirtualMemory = NTSTATUS(__fastcall*)(PEPROCESS sourceProcess,
	PEPROCESS TargetProcess,
	PVOID* BaseAddress,
	PSIZE_T RegionSize,
	WIN32_PROTECTION_MASK NewProtectWin32,
	PWIN32_PROTECTION_MASK LastProtect);

using fnZwGetContextThread = NTSTATUS(__fastcall*)(_In_ HANDLE hThread, _Inout_ PCONTEXT ThreadContext);

using fnPspGetContextThreadInternal = NTSTATUS(__fastcall*)(_In_ PETHREAD Thread,
	_Inout_ PCONTEXT ThreadContext,
	_In_ KPROCESSOR_MODE PreviousMode,
	_In_ KPROCESSOR_MODE PreviousMode2,
	_In_ char a5);

using fnKeStackAttachProcess = VOID(__fastcall*)(_Inout_ PRKPROCESS PROCESS, _Out_::PRKAPC_STATE ApcState);

using fnNtSetInformationDebugObject = NTSTATUS(__fastcall*)(
	IN HANDLE DebugObjectHandle,
	IN DEBUGOBJECTINFOCLASS DebugObjectInformationClass,
	IN PVOID DebugInformation,
	IN ULONG DebugInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

typedef PWND(__fastcall* fnValidateHwnd)(_In_ HWND hwnd);

typedef HWND(__fastcall* fnNtUserWindowFromPoint)(POINT Point);

typedef PEPROCESS(__fastcall* fnPsGetCurrentProcessByThread)(IN PETHREAD Thread);

using fnNtCreateUserProcess = NTSTATUS(NTAPI*)(PHANDLE ProcessHandle,
	PHANDLE ThreadHandle,
	ACCESS_MASK ProcessDesiredAccess,
	ACCESS_MASK ThreadDesiredAccess,
	PVOID ProcessObjectAttributes,
	PVOID ThreadObjectAttributes,
	ULONG ProcessFlags,
	ULONG ThreadFlags, PVOID ProcessParameters,
	void* CreateInfo, void* AttributeList);

using fnNtCreateDebugObject = NTSTATUS(NTAPI*)(
	_Out_ PHANDLE DebugObjectHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ ULONG Flags
	);

using fnObFastReferenceObject = PVOID(NTAPI*) (
	_In_ PEX_FAST_REF FastRef
	);

using fnObFastReferenceObjectLocked = PVOID(NTAPI*)(
	_In_ PEX_FAST_REF FastRef
	);

using fnMiSectionControlArea = PVOID(NTAPI*)(PVOID Section);

using fnMiReferenceControlAreaFile = PFILE_OBJECT(NTAPI*)(PVOID SectionControlArea);

using fnObFastDereferenceObject = VOID(NTAPI*)(_In_ PEX_FAST_REF FastRef, _In_ PVOID Object);

using fnMmGetFileNameForSection = NTSTATUS(NTAPI*)(
	IN  PSECTION SectionObject,
	OUT POBJECT_NAME_INFORMATION* FileNameInfo);

using fnDbgkClearProcessDebugObject = NTSTATUS(NTAPI*)(
	IN PEPROCESS Process,
	IN PDEBUG_OBJECT SourceDebugObject);

using fnRtlCaptureImageExceptionValues = VOID(NTAPI*)(IN PVOID ImageBase, OUT PIMAGE_RUNTIME_FUNCTION_ENTRY* RuntimeFunctionTable, OUT PULONG SizeOfTable);

using fnRtlpxLookupFunctionTable = PIMAGE_RUNTIME_FUNCTION_ENTRY(NTAPI*)(
	PVOID ControlPc,
	PINVERTED_FUNCTION_TABLE_ENTRY TableEntry);

using fnKeResumeThread = ULONG(__fastcall*)(_Inout_ PKTHREAD Thread);

using fnKeSuspendThread = ULONG(__fastcall*)(_Inout_ PKTHREAD Thread);

using fnKeForceResumeThread = ULONG(__fastcall*)(_Inout_ PKTHREAD Thread);

using fnKeFreezeAllThreads = VOID(NTAPI*)(VOID);

using fnKeThawAllThreads = VOID(NTAPI*)(VOID);

using fnPsQuitNextProcessThread = VOID(NTAPI*)(IN PETHREAD Thread);

using fnLpcRequestWaitReplyPortEx = NTSTATUS(NTAPI*)(PVOID PortAddress, PPORT_MESSAGE RequestMessage, PPORT_MESSAGE ReplyMessage);

using fnKeContextFromKframes = VOID(NTAPI*)(IN PKTRAP_FRAME TrapFrame, IN KEXCEPTION_FRAME* ExceptionFrame, OUT PCONTEXT ContextFrame);

using fnKeContextToKframes = VOID(NTAPI*)(IN PKTRAP_FRAME TrapFrame, IN KEXCEPTION_FRAME* ExceptionFrame, IN PCONTEXT ContextFrame, IN ULONG ContextFlags, IN KPROCESSOR_MODE PreviousMode);

using fnKiCheckForAtlThunk = BOOLEAN(NTAPI*)(IN PEXCEPTION_RECORD ExceptionRecord, IN PCONTEXT Context);

using fnKiSegSsToTrapFrame = VOID(NTAPI*)(IN PKTRAP_FRAME TrapFrame, IN ULONG SegSs);

using fnKiEspToTrapFrame = VOID(NTAPI*)(IN PKTRAP_FRAME TrapFrame, IN ULONG Esp);

using fnKiDebugRoutine = BOOLEAN(NTAPI*)(IN PKTRAP_FRAME TrapFrame, IN KEXCEPTION_FRAME* ExceptionFrame,
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PCONTEXT ContextRecord,
	IN KPROCESSOR_MODE PreviousMode,
	IN BOOLEAN SecondChance);

using fnObGetProcessHandleCount = ULONG(NTAPI*)(IN PEPROCESS Process);

using fnPsGetProcessSessionId = ULONG(NTAPI*)(IN PEPROCESS Process);

using fnObIsLUIDDeviceMapsEnabled = ULONG(NTAPI*)(VOID);

using fnObQueryDeviceMapInformation = NTSTATUS(NTAPI*)(IN PEPROCESS Process,
	IN PPROCESS_DEVICEMAP_INFORMATION DeviceMapInfo,
	IN ULONG Flags);

using fnSeLocateProcessImageName = NTSTATUS(__fastcall* )(IN PEPROCESS Process, OUT PUNICODE_STRING* pImageFileName);

using fnMmGetImageInformation = VOID(NTAPI*)(OUT PSECTION_IMAGE_INFORMATION ImageInformation);

using fnMmGetExecuteOptions =  NTSTATUS(NTAPI*)(IN PULONG ExecuteOptions);

using fnExMapHandleToPointer = PHANDLE_TABLE_ENTRY(NTAPI*)(IN PVOID HandleTable, IN HANDLE Handle);

using fnExUnlockHandleTableEntry = VOID(NTAPI*)(IN PVOID HandleTable, IN PHANDLE_TABLE_ENTRY HandleTableEntry);

using fnZwAlpcSendWaitReceivePort = NTSTATUS(__fastcall*)(HANDLE PortHandle,
	ULONG Flags,
	PPORT_MESSAGE SendMessage,
	PALPC_MESSAGE_ATTRIBUTES SendMessageAttributes,
	PPORT_MESSAGE ReceiveMessage,
	PSIZE_T BufferLength,
	PALPC_MESSAGE_ATTRIBUTES ReceiveMessageAttributes,
	PLARGE_INTEGER Timeout);

using fnMmGetSectionInformation = NTSTATUS(__fastcall*)(PSECTION SectionObject,
	ULONG Flags,
	PSECTION_IMAGE_INFORMATION ImageInfo);

using fnKiPreprocessFault = BOOLEAN(__fastcall*)(IN OUT PEXCEPTION_RECORD ExceptionRecord,
	IN PKTRAP_FRAME TrapFrame,
	IN OUT PCONTEXT ContextRecord,
	IN KPROCESSOR_MODE PreviousMode);

using fnDbgkpStartSystemErrorHandler = NTSTATUS(__fastcall*)();

using fnAlpcInitializeMessageAttribute = NTSTATUS(__fastcall*)(ULONG AttributeFlags,
	PALPC_MESSAGE_ATTRIBUTES Buffer,
	ULONG BufferSize,
	PULONG RequiredBufferSize);

using fnAlpcGetMessageAttribute = PVOID(__fastcall*)(_In_ PALPC_MESSAGE_ATTRIBUTES Buffer, _In_ ULONG AttributeFlag);

using fnDbgkpRemoveErrorPort = VOID(__fastcall*)(_In_ PETHREAD Thread, _In_ PDBGKP_ERROR_PORT DbgErrorPort);

using fnDbgkOpenProcessDebugPort = NTSTATUS(__fastcall*)(IN PEPROCESS Process, IN KPROCESSOR_MODE PreviousMode, OUT HANDLE* pHandle);

using fnPsFreezeProcess = BOOLEAN(__fastcall*)(IN PEPROCESS Process, BOOLEAN a2);

using fnNtWaitForDebugEvent = NTSTATUS(__fastcall*)(IN HANDLE DebugHandle,
	IN BOOLEAN Alertable,
	IN PLARGE_INTEGER Timeout OPTIONAL,
	OUT PDBGUI_WAIT_STATE_CHANGE StateChange);