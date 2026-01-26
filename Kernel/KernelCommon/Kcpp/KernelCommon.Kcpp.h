#pragma once

#include <fltKernel.h>
#include <ntimage.h>
#include <ntstrsafe.h>
#include <cassert>

// =============== EASTL ===============
#include <EASTL/algorithm.h>
#include <EASTL/functional.h>
#include <EASTL/random.h>
#include <EASTL/set.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/string.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/vector.h>
#include <EASTL/optional.h>
#include <EASTL/any.h>
#include <EASTL/variant.h>
// =====================================

//#define LOG_DEBUG(format, ...) 

extern "C" void* FuckStack_Stub();

//FuckStack Return Address
__declspec(selectany) PVOID ReturnAddressStubEnc = 0;

template<typename Ret = void,
	typename First = void*,
	typename Second = void*,
	typename Third = void*,
	typename Fourth = void*,
	typename Five = void*,
	typename Six = void*,
	typename Seven = void*,
	typename Eight = void*,
	typename Nine = void*,
	typename Ten = void*,
	typename... Stack>
	__forceinline Ret FuckStackCall(void* Func,
		First a1 = First{},
		Second a2 = Second{},
		Third a3 = Third{},
		Fourth a4 = Fourth{},
		Five a5 = Five{},
		Six a6 = Six{},
		Seven a7 = Seven{},
		Eight a8 = Eight{},
		Nine a9 = Nine{},
		Ten a10 = Ten{},
		Stack&&... args)
{
	//DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, __PRETTY_FUNCTION__);
	struct shell_params { const void* a1; void* a2; void* a3; };
	shell_params CallCtx = { ReturnAddressStubEnc, Func, nullptr };

	typedef Ret(*ShellFn)(First, Second, Third, Fourth, PVOID, PVOID, Five, Six, Seven, Eight, Nine, Ten, Stack...);
	return ((ShellFn)&FuckStack_Stub)(a1, a2, a3, a4, &CallCtx, nullptr, a5, a6, a7, a8, a9, a10, eastl::forward<std::remove_reference_t<Stack>>(args)...);
}

__forceinline void SetFuckStackStub(PVOID R15_Stub) { ReturnAddressStubEnc = R15_Stub; }

#define ApiSpoofCall(a, ...) \
		FuckStackCall<decltype(a(__VA_ARGS__))>(a##Fn, __VA_ARGS__)

#ifdef __cplusplus
#include "noimp/Internals.h"
#include "noimp/FIntrin.h"
#include "crt/crt.h"
#include "noimp/HideImport.h"
#include "noimp/Helpers.h"
#include "noimp/MapLoad.h"

#include "kcrt/kcrt.h"
#include "kstl/kstl.h"

#else
#    error 必须用CPP来引用我
#endif