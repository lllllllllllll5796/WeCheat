#pragma once

#define naked 

#define MAX_PATH 260

#define NOINLINE __declspec(noinline)

#define INLINE inline

#define GET_OFFSET_64(structure, member) ((int64_t)&((structure*)0)->member) // 64
#define GET_OFFSET_32(structure, member) ((int32_t)&((structure*)0)->member) // 32

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))
#define PTR_SUB_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) - (ULONG_PTR)(Offset)))
#define GETWORD(X)    (*(USHORT*)(X))
#define GETDWORD(X)   (*(ULONG*)(X))
#define GETQWORD(X)   (*(ULONG64*)(X))

#define _max(a, b)  (((a) > (b)) ? (a) : (b))
#define _min(a, b)  (((a) < (b)) ? (a) : (b))

#define NANOSECONDS(nanos)   \
	 (((signed __int64)(nanos)) / 100L)

#define MICROSECONDS(micros) \
	 (((signed __int64)(micros)) * NANOSECONDS(1000L))

#define MILLISECONDS(milli)  \
	 (((signed __int64)(milli)) * MICROSECONDS(1000L))

#define SECONDS(seconds)	 \
	 (((signed __int64)(seconds)) * MILLISECONDS(1000L))

#define HTONS(n) (((((unsigned short)(n) & 0xFFu  )) << 8) | \
                   (((unsigned short)(n) & 0xFF00u) >> 8))

#define MAKE_PAGE_ALIGN(Va) ((PVOID)((QWORD)(Va) & ~(PAGE_SIZE - 1)))
