#pragma once
#include <fltKernel.h>

#ifdef _HAS_EXCEPTIONS
#    undef _HAS_EXCEPTIONS
#endif

#define _HAS_EXCEPTIONS 0

DECLSPEC_NORETURN void
KernelStlRaiseException(_In_ ULONG bug_check_code);

DECLSPEC_NORETURN void __cdecl _invalid_parameter_noinfo_noreturn();

namespace std
{
	DECLSPEC_NORETURN void __cdecl _Xbad_alloc();
	DECLSPEC_NORETURN void __cdecl _Xinvalid_argument(_In_z_ const char*);
	DECLSPEC_NORETURN void __cdecl _Xlength_error(_In_z_ const char*);
	DECLSPEC_NORETURN void __cdecl _Xout_of_range(_In_z_ const char*);
	DECLSPEC_NORETURN void __cdecl _Xoverflow_error(_In_z_ const char*);
	DECLSPEC_NORETURN void __cdecl _Xruntime_error(_In_z_ const char*);
	DECLSPEC_NORETURN void __cdecl _Xbad_function_call();
}

void* __cdecl
operator new(_In_ size_t size);
void* __cdecl
operator new[](_In_ size_t size);

void*
operator new[](size_t size, const char*, int, unsigned, const char*, int);
void*
operator new[](size_t size, size_t, size_t, const char*, int, unsigned, const char*, int);

void __cdecl
operator delete(_In_ void* p);
void __cdecl
operator delete[](_In_ void* p);
void __cdecl
operator delete[](_In_ void* p, size_t size);

void __cdecl
operator delete(_In_ void* p, _In_ size_t size);

_Success_(return >= 0) EXTERN_C inline int __cdecl __stdio_common_vsprintf_s(
	_In_ unsigned __int64 _Options, _Out_writes_z_(_BufferCount) char* _Buffer,
	_In_ size_t _BufferCount,
	_In_z_ _Printf_format_string_params_(2) char const* _Format,
	_In_opt_ _locale_t _Locale, va_list _ArgList);

_Success_(return >= 0) _Check_return_opt_ EXTERN_C
inline int __cdecl __stdio_common_vswprintf_s(
	_In_ unsigned __int64 _Options,
	_Out_writes_z_(_BufferCount) wchar_t* _Buffer, _In_ size_t _BufferCount,
	_In_z_ _Printf_format_string_params_(2) wchar_t const* _Format,
	_In_opt_ _locale_t _Locale, va_list _ArgList);


template <typename T>
inline T allocate_pool(unsigned __int64 size)
{
	return (T)malloc(size);
}

template <typename T>
inline T* allocate_pool()
{
	return (T*)malloc(sizeof(T));
}

template <typename T>
inline T* allocate_contignous_memory()
{
	PHYSICAL_ADDRESS a;
	a.QuadPart = 0ULL - 1;
	return (T*)MmAllocateContiguousMemory(sizeof(T), a);
}

template <typename T>
inline T allocate_contignous_memory(unsigned __int64 size)
{
	PHYSICAL_ADDRESS a;
	a.QuadPart = 0ULL - 1;
	return (T)MmAllocateContiguousMemory(size, a);
}

inline void free_pool(void* pool_address)
{
	free(pool_address);
}

//释放连续内存
inline void free_contignous_memory(void* memory_address)
{
	MmFreeContiguousMemory(memory_address);
}