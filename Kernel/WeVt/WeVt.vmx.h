#pragma once

union __vmx_ept_vpid_cap
{
	unsigned __int64 all;
	struct
	{
		unsigned __int64 supports_execute_only : 1;
		unsigned __int64 reserved_0 : 5;
		unsigned __int64 support_page_walk_length_4 : 1;
		unsigned __int64 reserved_1 : 1;
		unsigned __int64 ept_support_uc : 1;
		unsigned __int64 reserved_2 : 5;
		unsigned __int64 ept_support_wb : 1;
		unsigned __int64 reserved_3 : 1;
		unsigned __int64 ept_support_2mb_page : 1;
		unsigned __int64 ept_support_1gb_page : 1;
		unsigned __int64 reserved_4 : 2;
		unsigned __int64 support_invept_instruction : 1;
		unsigned __int64 support_access_dirty_flags : 1;
		unsigned __int64 reports_advanced_vm_exit_information : 1;
		unsigned __int64 supervisor_shadow_stack_control : 1;
		unsigned __int64 reserved_5 : 1;
		unsigned __int64 support_single_context_invept : 1;
		unsigned __int64 support_all_context_invept : 1;
		unsigned __int64 reserved_6 : 5;
		unsigned __int64 support_invvpid_instruction : 1;
		unsigned __int64 reserved_7 : 7;
		unsigned __int64 support_individual_address_invvpid : 1;
		unsigned __int64 support_single_context_invvpid : 1;
		unsigned __int64 support_all_context_invvpid : 1;
		unsigned __int64 single_context_retaining_globals_invvpid : 1;
		unsigned __int64 reserved_8 : 4;
		unsigned __int64 enumerate_max_hlat_prefix_size : 6;
	};
};
