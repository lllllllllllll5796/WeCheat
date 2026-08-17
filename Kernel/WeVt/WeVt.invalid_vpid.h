#pragma once

struct __invvpid_descriptor
{
	union
	{
		unsigned __int64 vpid : 16;
		unsigned __int64 reserved : 48;
	};

	unsigned __int64 linear_address;
};

void invvpid_invidual_address_func(unsigned __int64 linear_address, unsigned __int8 vpid);
void invvpid_single_context_func(unsigned __int8 vpid);
void invvpid_all_contexts();
void invvpid_single_context_except_global_translations(unsigned __int8 vpid);