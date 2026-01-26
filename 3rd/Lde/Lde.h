#pragma once

typedef int(*LDE_DISASM)(void* p, int dw);

BOOLEAN WINAPI LDE_INIT(_In_ LDE_DISASM* LDE);
VOID WINAPI LDE_UNINIT(LDE_DISASM LDE);
size_t WINAPI instruction_len(void* virtual_address, size_t need_length);