#pragma once

	namespace ProcessList
	{
		VOID CreateProcessNotifyRoutine(IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create);
		VOID CreateProcessNotifyRoutineEx(IN HANDLE  ParentId, IN HANDLE  ProcessId, __in_opt PPS_CREATE_NOTIFY_INFO CreateInfo);

		struct ThreadData
		{
			HANDLE ThreadID;
			PETHREAD PEThread;
			KAPC SuspendApc;
			KSEMAPHORE SuspendSemaphore; //why not mutex?
			int suspendcount;
			struct ThreadData* previous;
			struct ThreadData* next;
		};

		typedef struct
		{
			HANDLE ProcessID;
			PEPROCESS PEProcess;
			HANDLE ProcessHandle;
			BOOLEAN Deleted;
		} ProcessListData, * PProcessListData;


		struct ProcessData
		{
			HANDLE ProcessID;
			PEPROCESS PEProcess;
			struct ThreadData* Threads;
			struct ProcessData* previous;
			struct ProcessData* next;
		};

		typedef struct tagProcessEventData
		{
			UINT64 Created;
			UINT64 ProcessID;
			UINT64 PEProcess;
		} ProcessEventdta;

		VOID CreateThreadNotifyRoutine(IN HANDLE ProcessId, IN HANDLE ThreadId, IN BOOLEAN Create);

		typedef struct tagThreadEventData
		{
			BOOLEAN Created;
			UINT64 ProcessID;
			UINT64 ThreadID;
		} ThreadEventDta;

		extern BOOLEAN CreateProcessNotifyRoutineEnabled;
		extern ERESOURCE ProcesslistR;
		extern ProcessData* processlist;
		extern UCHAR ProcessEventCount;
		extern UCHAR ThreadEventCount;
		//extern HANDLE WatcherHandle;
		extern PEPROCESS WatcherProcess;
		extern BOOLEAN ProcessWatcherOpensHandles;
		extern BOOLEAN CreateThreadNotifyRoutineEnabled;

		extern PKEVENT ProcessEvent;
		extern PKEVENT ThreadEvent;
		extern ProcessEventdta ProcessEventdata[50];
		extern ThreadEventDta  ThreadEventData[50];

		VOID CleanProcessList();
		HANDLE GetHandleForProcessID(IN HANDLE ProcessID);
	}

