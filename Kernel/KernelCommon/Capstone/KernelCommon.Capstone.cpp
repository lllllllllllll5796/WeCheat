#include "kernelCommon.pch.h"
#include "Capstone/KernelCommon.Capstone.h"

// A pool tag for memory allocation
//static const ULONG CS_DRIVER_POOL_TAG = 'rdsC';

// A structure to implement realloc()
typedef struct _CS_DRIVER_MEMBLOCK
{
  size_t size;   // A number of bytes allocated
  char data[1];  // An address returned to a caller
} CS_DRIVER_MEMBLOCK;
C_ASSERT(sizeof(CS_DRIVER_MEMBLOCK) == sizeof(void *) * 2);


// free()
static void CAPSTONE_API csdrv_free(void *ptr)
{
  if (ptr)
  {
    ImpCall(ExFreePool, CONTAINING_RECORD(ptr, CS_DRIVER_MEMBLOCK, data)/*,CS_DRIVER_POOL_TAG*/);
  }
}

// malloc()
static void * CAPSTONE_API csdrv_malloc(size_t size)
{
  // Disallow zero length allocation because they waste pool header space and,
  // in many cases, indicate a potential validation issue in the calling code.
  NT_ASSERT(size);

  CS_DRIVER_MEMBLOCK *block = (CS_DRIVER_MEMBLOCK *)ImpCall(ExAllocatePool,
    NonPagedPoolNx, size + sizeof(CS_DRIVER_MEMBLOCK)/*,CS_DRIVER_POOL_TAG*/);
  if (!block)
  {
    return NULL;
  }
  block->size = size;
  return block->data;
}

// calloc()
static void * CAPSTONE_API csdrv_calloc(size_t n, size_t size)
{
  size_t total = n * size;

  void *new_ptr = csdrv_malloc(total);
  if (!new_ptr)
  {
    return NULL;
  }

  return RtlFillMemory(new_ptr, total, 0);
}

// realloc()
static void * CAPSTONE_API csdrv_realloc(void *ptr, size_t size)
{
  void *new_ptr = NULL;
  size_t current_size = 0;
  size_t smaller_size = 0;

  if (!ptr)
  {
    return csdrv_malloc(size);
  }

  new_ptr = csdrv_malloc(size);
  if (!new_ptr)
  {
    return NULL;
  }

  current_size = CONTAINING_RECORD(ptr, CS_DRIVER_MEMBLOCK, data)->size;
  smaller_size = (current_size < size) ? current_size : size;
  RtlCopyMemory(new_ptr, ptr, smaller_size);
  csdrv_free(ptr);
  return new_ptr;
}

// vsnprintf(). _vsnprintf() is avaialable for drivers, but it differs from
// vsnprintf() in a return value and when a null-terminater is set.
// csdrv_vsnprintf() takes care of those differences.
#pragma warning(push)
#pragma warning(disable : 28719)  // Banned API Usage : _vsnprintf is a Banned
                                  // API as listed in dontuse.h for security
                                  // purposes.
static int CAPSTONE_API csdrv_vsnprintf(char *buffer, size_t count,
  const char *format, va_list argptr)
{
  int result = _vsnprintf(buffer, count, format, argptr);

  // _vsnprintf() returns -1 when a string is truncated, and returns "count"
  // when an entire string is stored but without '\0' at the end of "buffer".
  // In both cases, null-terminater needs to be added manually.
  if (result == -1 || (size_t)result == count)
  {
    buffer[count - 1] = '\0';
  }
  if (result == -1)
  {
    // In case when -1 is returned, the function has to get and return a number
    // of characters that would have been written. This attempts so by re-tring
    // the same conversion with temp buffer that is most likely big enough to
    // complete formatting and get a number of characters that would have been
    // written.
    char tmp[1024];
    result = _vsnprintf(tmp, RTL_NUMBER_OF(tmp), format, argptr);
    NT_ASSERT(result != -1);
  }

  return result;
}
#pragma warning(pop)

/*
Initializes Capstone dynamic memory management for Windows drivers

@return: CS_ERR_OK on success, or other value on failure.
Refer to cs_err enum for detailed error.

NOTE: cs_driver_init() can be called at IRQL <= DISPATCH_LEVEL.
*/
cs_err CAPSTONE_API Capstone_Init()
{
  KFLOATING_SAVE float_save;
  NTSTATUS status;
  cs_opt_mem setup;
  cs_err err;

  //NT_ASSERT(ImpCall(KeGetCurrentIrql) <= DISPATCH_LEVEL);

  // Capstone API may use floating point.
  status = KeSaveFloatingPointState(&float_save);
  if (!NT_SUCCESS(status))
  {
    return CS_ERR_MEM;
  }

  setup.malloc = csdrv_malloc;
  setup.calloc = csdrv_calloc;
  setup.realloc = csdrv_realloc;
  setup.free = csdrv_free;
  setup.vsnprintf = csdrv_vsnprintf;
  err = cs_option(0, CS_OPT_MEM, (size_t)&setup);

  KeRestoreFloatingPointState(&float_save);
  return err;
}

ULONG_PTR Capstone_Follow_Jump(void* address)
{
	//LOG_DEBUG("地址:%p\r\n", address);
	//__debugbreak();
	ULONG_PTR JmpAddress = (ULONG_PTR)address;
	// Save floating point state
	KFLOATING_SAVE float_save;

	auto status = KeSaveFloatingPointState(&float_save); //可能
	if (!NT_SUCCESS(status)) {
		return 0;
	}

	// Disassemble at most 15 bytes to get an instruction size
	csh handle;
	const auto mode = CS_MODE_64;
	auto res = cs_open(CS_ARCH_X86, mode, &handle);
	if (res != CS_ERR_OK) {//类似打开一个库 CS_ARCH_X86 包含了x64架构
		KeRestoreFloatingPointState(&float_save);
		return 0;
	}

	//--------------分析参数需要加入--------------
	cs_option(handle, CS_OPT_SKIPDATA, CS_OPT_OFF);
	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
	//--------------------------------------------

	static const auto kLongestInstSize = 15;
	cs_insn* instructions = nullptr;
	//利用那个反汇编库 把指定数组地址的二进制操作码(kLongestInstSize数组大小)反汇编为指令 
	//保存到instructions 第三个参数表示数组的第一个要被反汇编的指令的起始地址  
	//第四个参数表示从起始地址开始要反汇编几个指令
	const auto count =
		cs_disasm(handle, reinterpret_cast<uint8_t*>(address), kLongestInstSize,
			reinterpret_cast<uint64_t>(address), 1, &instructions);

	if (count == 0)
	{
		cs_close(&handle);
		KeRestoreFloatingPointState(&float_save);
		return 0;
	}


	if (instructions && instructions->id == X86_INS_JMP && instructions->detail->x86.op_count == 1)
	{
		//LOG_DEBUG("是Jmp跳转\r\n");
		if (instructions->detail->x86.operands[0].type == X86_OP_IMM)
		{
			JmpAddress = instructions->detail->x86.operands[0].imm;
		}
		else if (instructions->detail->x86.operands[0].type == X86_OP_MEM)
		{
			uintptr_t absolute = (uintptr_t)address + instructions->detail->x86.operands[0].mem.disp + instructions->size;
			JmpAddress = *(uintptr_t*)absolute;
		}
	}

	// Get a size of the first instruction
	// const auto size = instructions[0].size;  //得到反汇编结果的 第一个指令的长度
	// LOG_DEBUG("指令长度:%d\r\n", size);

	cs_free(instructions, count);//记住要释放instructions
	cs_close(&handle);//关闭反汇编库句柄

	// Restore floating point state
	KeRestoreFloatingPointState(&float_save);
	return JmpAddress;
}

// Returns a size of an instruction at the address
SIZE_T Capstone_GetInstructionSize(void* address)
{
	// Save floating point state
	KFLOATING_SAVE float_save;
	auto status = KeSaveFloatingPointState(&float_save); //可能
	if (!NT_SUCCESS(status)) {
		return 0;
	}

	// Disassemble at most 15 bytes to get an instruction size
	csh handle;
	const auto mode = CS_MODE_64;
	if (cs_open(CS_ARCH_X86, mode, &handle) != CS_ERR_OK) {//类似打开一个库 CS_ARCH_X86 包含了x64架构
		KeRestoreFloatingPointState(&float_save);
		return 0;
	}

	static const auto kLongestInstSize = 15;
	cs_insn* instructions = nullptr;
	//利用那个反汇编库 把指定数组地址的二进制操作码(kLongestInstSize数组大小)反汇编为指令 
	//保存到instructions 第三个参数表示数组的第一个要被反汇编的指令的起始地址  
	//第四个参数表示从起始地址开始要反汇编几个指令
	const auto count =
		cs_disasm(handle, reinterpret_cast<uint8_t*>(address), kLongestInstSize,
			reinterpret_cast<uint64_t>(address), 1, &instructions);
	if (count == 0) {
		cs_close(&handle);
		KeRestoreFloatingPointState(&float_save);
		return 0;
	}

	// Get a size of the first instruction
	const auto size = instructions[0].size;  //得到反汇编结果的 第一个指令的长度
	cs_free(instructions, count);//记住要释放instructions
	cs_close(&handle);//关闭反汇编库句柄

	// Restore floating point state
	KeRestoreFloatingPointState(&float_save);
	return size;
}
