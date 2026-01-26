#pragma once

namespace KernelCommon
{
	typedef struct _THREAD_DATA {
		// 在这里定义你需要的线程参数
		// ...
	} THREAD_DATA, * PTHREAD_DATA;

	extern bool boLogInit;

	NTSTATUS CreateLogsDirectory(const wchar_t* path);

	NTSTATUS WriteLogToXmlFileW(const wchar_t* logMessage);

	NTSTATUS WriteLogToXmlFileA(const char* logMessage);

	NTSTATUS LogDriverMessageW(const wchar_t* message);

	NTSTATUS LogDriverMessageA(const char* message);

	NTSTATUS SetLogFilePath(const wchar_t* path);

	NTSTATUS ReadIniValue(_In_ const wchar_t* filePath, _In_ const wchar_t* sectionName, _In_ const wchar_t* keyName, _Out_ wchar_t* value, _In_ ULONG valueSize);

	PWSTR ReadIni(_In_ const wchar_t* filePath, _In_ const wchar_t* sectionName, _In_ const wchar_t* keyName);

	NTSTATUS InitDriverLog(const wchar_t* LogPath);

	NTSTATUS CreateInternalThread(PKSTART_ROUTINE StartRoutine, PVOID StartContext, PETHREAD* Thread);

	NTSTATUS CreateKernelThread(PKSTART_ROUTINE StartRoutine, PTHREAD_DATA threadData, PETHREAD* Thread);

	VOID KernelSleep(UINT32 milliseconds);

	VOID RemovePath(const wchar_t* fullPath);
}