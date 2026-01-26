#pragma once

template <typename T>
struct _UNICODE_STRING_T
{
	WORD Length;
	WORD MaximumLength;
	T Buffer;
};

typedef struct _PROCESS_BASIC_INFORMATION64 {
	NTSTATUS ExitStatus;
	UINT32 Reserved0;
	UINT64 PebBaseAddress;
	UINT64 AffinityMask;
	UINT32 BasePriority;
	UINT32 Reserved1;
	UINT64 UniqueProcessId;
	UINT64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64;

typedef struct _PROCESS_BASIC_INFORMATION32 {
	NTSTATUS ExitStatus;
	UINT32 PebBaseAddress;
	UINT32 AffinityMask;
	UINT32 BasePriority;
	UINT32 UniqueProcessId;
	UINT32 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION32;