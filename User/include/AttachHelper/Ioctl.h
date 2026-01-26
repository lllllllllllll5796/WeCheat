#pragma once

BOOL SendUserDataToDriver(DWORD dwIoControlCode,
	PVOID source,
	SIZE_T size,
	PVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned);

void Test();
void InitSymbols();
void InitDebug();