#pragma once

#pragma pack(push,1)

typedef struct _EptHookItem
{
	void* target_address;
	void* target_page_address;
	int target_address_offset;
	unsigned char* backup_page_context;
	int hookcode_length;
} EptHookItem;

#pragma pack(pop)

// typedef enum {
//   cc_type,
//   ud_type,
//   jmp_type,
// } HookCodeType;

namespace VmxHelper
{
	//导出这个给Detours用
	BOOL EptHook(const eastl::string& function_name, void* target_address, void* fake_function, void* trampoline_address, void* hookcode, size_t hookcode_length);
	VOID EptUnHook(const eastl::string& function_name);


	BOOL HookFunction(const eastl::string& function_name, void* target_address, void* fake_function, PVOID* trampoline_address, HookCodeType hookcode_type);
	void UnHookFunction(const eastl::string& function_name);

	extern eastl::unordered_map<eastl::string, eastl::shared_ptr<EptHookItem>> g_EptHooks;
}
