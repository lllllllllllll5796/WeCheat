#include "WeDebug.pch.h"
#include "WeDebug.Process.h"

EXTERN_C
NTSTATUS __fastcall Fake_PspInsertProcess(
	PEPROCESS TargetProcess,
	PEPROCESS ParentProcess,
	ACCESS_MASK DesiredAccess,
	ULONG Flags,
	ULONG JobMemberLevel,
	HANDLE DebugObjectHandle,
	ULONG CrossThreadFlags,
	PVOID ObjectCreationStateRaw)
{
	PPSP_OBJECT_CREATION_STATE ObjectCreationState = (PPSP_OBJECT_CREATION_STATE)ObjectCreationStateRaw;
	PEJOB Job;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS Parent;
	PVOID DebugPortObject = NULL;
	BOOLEAN boCopyDebugPort = FALSE;
	PETHREAD CurrentThread;
	PEPROCESS Process = NULL;
	int v18 = 0;

	PAGED_CODE();
	UNREFERENCED_PARAMETER(JobMemberLevel);

	CurrentThread = PsGetCurrentThread();
	Parent = PsGetCurrentProcess();

	size_t ptr_ObjectTable = (size_t)TargetProcess + Offset::EProcess::ObjectTable;
	size_t ptr_handle_table_UniqueProcessId = *(size_t*)ptr_ObjectTable + Offset::HandleTable::UniqueProcessId;
	size_t ptr_UniqueProcessId = (size_t)TargetProcess + Offset::EProcess::UniqueProcessId;
	*(size_t*)ptr_handle_table_UniqueProcessId = *(size_t*)ptr_UniqueProcessId;

	if (Global::SeAuditingWithTokenForSubcategory && Global::SeAuditingWithTokenForSubcategory(0x85, NULL))
	{
		if (Global::SeAuditProcessCreation)
		{
			Global::SeAuditProcessCreation(TargetProcess, 0);
		}
	}

	size_t ptr_ParentJob = ParentProcess ? ((size_t)ParentProcess + Offset::EProcess::Job) : 0;

	BOOLEAN jobOk = TRUE;
	if (ParentProcess && ptr_ParentJob && *(PEJOB*)ptr_ParentJob)
	{
		Status = Global::PspImplicitAssignProcessToJob(*(PEJOB*)ptr_ParentJob, TargetProcess, Flags);
		jobOk = (Status >= 0);
	}

	BOOLEAN inheritOk = TRUE;
	if (jobOk && Global::PspInheritSyscallProvider)
	{
		Status = Global::PspInheritSyscallProvider(TargetProcess, ParentProcess);
		inheritOk = (Status >= 0);
	}

	if (!ParentProcess || (jobOk && inheritOk))
	{
		PspLockProcessListExclusive(CurrentThread);
		size_t ptr_ActiveProcessLinks = (size_t)TargetProcess + Offset::EProcess::ActiveProcessLinks;
		InsertTailList(Global::PsActiveProcessHead, (PLIST_ENTRY)ptr_ActiveProcessLinks);

		size_t ptr_SequenceNumber = (size_t)TargetProcess + Offset::EProcess::SequenceNumber;
		*(size_t*)ptr_SequenceNumber = ++(*Global::PspProcessSequenceNumber);

		if (Global::PspUnlockProcessListExclusive)
		{
			Global::PspUnlockProcessListExclusive((PKTHREAD)CurrentThread);
		}
		else
		{
			ExReleasePushLockExclusive(reinterpret_cast<PULONG_PTR>(Global::PspActiveProcessLock));
			KeLeaveGuardedRegion();
		}

		size_t ptr_ParentFlags = (size_t)Parent + Offset::EProcess::Flags;
		PROCESSFLAGS ParentFlags = *(PROCESSFLAGS*)ptr_ParentFlags;
		if (ParentFlags.ProcessDelete)
		{
			Status = STATUS_PROCESS_IS_TERMINATING;
		}
		else if (!DebugObjectHandle || (Status = ObReferenceObjectByHandle(
			DebugObjectHandle,
			DEBUG_PROCESS_ASSIGN,
			Global::WeDebug_DbgkDebugObjectType,
			KernelMode,
			&DebugPortObject,
			NULL),
			Status >= 0))
		{
			Status = Global::DbgkCopyProcessDebugPort(TargetProcess, Parent, (PDEBUG_OBJECT)DebugPortObject, &boCopyDebugPort);
			if (DebugPortObject)
			{
				ObDereferenceObject(DebugPortObject);
			}

			if (NT_SUCCESS(Status))
			{
				if (boCopyDebugPort && (Flags & 2))
				{
					size_t ptr_TargetFlags = (size_t)TargetProcess + Offset::EProcess::Flags;
					InterlockedOr((volatile LONG*)ptr_TargetFlags, 2u);
				}

				v18 = 0;
				if (!ParentProcess || (Process = PsInitialSystemProcess, ParentProcess != PsInitialSystemProcess))
				{
					if (CrossThreadFlags & 2)
					{
						Process = TargetProcess;
						v18 = 1;
					}
					else
					{
						Process = Parent;
					}
				}

				size_t ptr_TypeInfo = (size_t)(*PsProcessType) + Offset::ObjectType::TypeInfo;
				size_t ptr_GenericMapping = ptr_TypeInfo + Offset::ObjectTypeInitializer::GenericMapping;
				Status = Global::SeCreateAccessStateEx(
					NULL,
					Process,
					&ObjectCreationState->AccessState,
					&ObjectCreationState->AuxData,
					DesiredAccess,
					(PGENERIC_MAPPING)ptr_GenericMapping);

				if (NT_SUCCESS(Status))
				{
					ObReferenceObjectWithTag(TargetProcess, 'rCsP');
					Status = Global::ObInsertObjectEx(
						TargetProcess,
						&ObjectCreationState->AccessState,
						DesiredAccess,
						0,
						v18,
						NULL,
						NULL);

					if (NT_SUCCESS(Status))
					{
						ObDereferenceObjectWithTag(TargetProcess, 'rCsP');

						size_t ptr_TargetJob = (size_t)TargetProcess + Offset::EProcess::Job;
						Job = *(PEJOB*)ptr_TargetJob;
						if (!Job)
						{
							size_t ptr_TargetFlags = (size_t)TargetProcess + Offset::EProcess::Flags;
							InterlockedOr((volatile LONG*)ptr_TargetFlags, 0x4000000u);
						LABEL_21:
							Global::ObCheckRefTraceProcess(TargetProcess);
							if (CrossThreadFlags & 1)
							{
								PspUnlockProcessExclusive(TargetProcess, CurrentThread);
							}
							return STATUS_SUCCESS;
						}
						Status = Global::PspValidateJobAffinityState(Job, TargetProcess);
						if (NT_SUCCESS(Status))
						{
							goto LABEL_21;
						}
					}
					Global::SepDeleteAccessState(&ObjectCreationState->AccessState);
					SeReleaseSubjectContext(&ObjectCreationState->AccessState.SubjectSecurityContext);
				}
			}
		}
	}

	if (CrossThreadFlags & 1)
	{
		PspUnlockProcessExclusive(TargetProcess, CurrentThread);
	}
	return Status;
}
