#pragma once

namespace DeepKernel
{
	VOID LoadImageNotifyRoutine(IN PUNICODE_STRING  FullImageName, IN HANDLE  ProcessId, IN PIMAGE_INFO  ImageInfo);
	BOOLEAN MakeWritable(PVOID StartAddress, UINT_PTR size, BOOLEAN usecopyonwrite);

	extern BOOLEAN		ImageNotifyRoutineLoaded;
	extern char*       ModuleList;
	extern int			ModuleListSize;
	extern ULONG		Size;

	extern UINT_PTR	ActiveLinkOffset;
	extern UINT_PTR	ProcessNameOffset;
	extern UINT_PTR	DebugportOffset;
	extern UINT_PTR	PIDOffset;

}