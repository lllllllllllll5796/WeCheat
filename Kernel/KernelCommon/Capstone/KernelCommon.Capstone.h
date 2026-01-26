#ifndef CAPSTONE_CS_DRIVER_H_
#define CAPSTONE_CS_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <capstone/Include/capstone.h>

/*
 Initializes Capstone dynamic memory management for Windows drivers

 @return: CS_ERR_OK on success, or other value on failure.
 Refer to cs_err enum for detailed error.

 NOTE: cs_driver_init() can be called at IRQL <= DISPATCH_LEVEL.
*/
cs_err CAPSTONE_API Capstone_Init();

ULONG_PTR Capstone_Follow_Jump(void* address);

SIZE_T Capstone_GetInstructionSize(void* address);

#ifdef __cplusplus
}
#endif

#endif  // CAPSTONE_CS_DRIVER_H_
