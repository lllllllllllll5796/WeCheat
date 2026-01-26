#pragma once

//°²×°hook
void HookOn(_In_ PVOID* pfun, _In_ PVOID proxy_fun, _In_ HANDLE hThread);

//Ð¶ÔØhook
void HookOff(_In_ PVOID* pfun, _In_ PVOID proxy_fun, _In_ HANDLE hThread);

#define GET_TYPE(thisFunc, func) static_cast<decltype(&thisFunc)>(func)