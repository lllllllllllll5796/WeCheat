#pragma once

BOOLEAN InitNtoskrnlSymbolsTable();

BOOLEAN InitWin32kBaseSymbolsTable();

BOOLEAN InitWin32kFullSymbolsTable();

void CheckFunctionPointers();

bool DispatchOffsetToHost();