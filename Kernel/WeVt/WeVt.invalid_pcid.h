#pragma once

struct __invpcid_descriptor
{
	unsigned __int64 pcid : 12;
	unsigned __int64 reserved : 52;
	unsigned __int64 linear_address;
};
