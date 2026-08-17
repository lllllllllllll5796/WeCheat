#include "KernelCommon.pch.h"
#include "kernel_stl.h"

// crt
#include "../kcrt/kcrt.h"

_Use_decl_annotations_
DECLSPEC_NORETURN void
KernelStlRaiseException(ULONG bug_check_code)
{
	KdBreakPoint();
#pragma warning(push)
#pragma warning(disable : 28159)
	KeBugCheck(bug_check_code);
#pragma warning(pop)
}

DECLSPEC_NORETURN void __cdecl _invalid_parameter_noinfo_noreturn()
{
	KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
}

namespace std
{
	// VS2019��Ҫ��
	//https://bbs.kanxue.com/thread-263906.htm
	_CRTIMP2_PURE_IMPORT _Prhand _Raise_handler;

	DECLSPEC_NORETURN void __cdecl _Xbad_alloc()
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
		DECLSPEC_NORETURN void __cdecl _Xinvalid_argument(_In_z_ const char*)
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
		DECLSPEC_NORETURN void __cdecl _Xlength_error(_In_z_ const char*)
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
		DECLSPEC_NORETURN void __cdecl _Xout_of_range(_In_z_ const char*)
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
		DECLSPEC_NORETURN void __cdecl _Xoverflow_error(_In_z_ const char*)
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
		DECLSPEC_NORETURN void __cdecl _Xruntime_error(_In_z_ const char*)
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
		DECLSPEC_NORETURN void __cdecl _Xbad_function_call()
	{
		KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	_Use_decl_annotations_
	const char* __cdecl _Syserror_map(__in int)
	{
		return nullptr;
	}

} // namespace std

_Use_decl_annotations_
void* __cdecl
operator new[](size_t size)
{
	if (size == 0)
	{
		size = 1;
	}

	void* p = malloc(size);
	if (!p)
	{
		KernelStlRaiseException(MUST_SUCCEED_POOL_EMPTY);
	}
	return p;
}

_Use_decl_annotations_
void* __cdecl
operator new(size_t size)
{
	if (size == 0)
	{
		size = 1;
	}

	void* p = malloc(size);
	if (!p)
	{
		KernelStlRaiseException(MUST_SUCCEED_POOL_EMPTY);
	}
	return p;
}

_Use_decl_annotations_
void __cdecl
operator delete(void* p)
{
	if (p)
	{
		free(p);
	}
}

_Use_decl_annotations_
void __cdecl
operator delete(void* p, size_t size)
{
	UNREFERENCED_PARAMETER(size);
	if (p)
	{
		free(p);
	}
}

_Use_decl_annotations_
void __cdecl
operator delete[](_In_ void* p)
{
	if (p)
	{
		free(p);
	}
}

_Use_decl_annotations_
void __cdecl
operator delete[](_In_ void* p, size_t size)
{
	UNREFERENCED_PARAMETER(size);
	if (p)
	{
		free(p);
	}
}

// lua ��Ҫ��(��ʵ��eastl)
void* operator new[](size_t size, const char*, int, unsigned, const char*, int)
{
	return malloc(size);
}

void* operator new[](size_t size, size_t, size_t, const char*, int, unsigned, const char*, int)
{
	return malloc(size);
}