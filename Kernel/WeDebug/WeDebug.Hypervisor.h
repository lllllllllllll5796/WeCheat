#pragma once

namespace hvgt
{
	void vmoff(unsigned __int32 processor_count);
	void invept(bool invept_all);
	void hypervisor_visible(bool value);
	bool ept_unhook();
	bool ept_unhook(void* function_address);
	bool test_vmcall();
	bool hook_function(void* target_address, void* proxy_function, void** origin_function);
	bool vmcall(void* vmcallinfo);
	bool get_hide_software_breakpoint(void* target_address, void* buffer, unsigned __int64 buffer_size);
	bool set_hide_software_breakpoint(void* vmcallinfo);
}
