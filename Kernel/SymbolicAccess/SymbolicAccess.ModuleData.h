#pragma once

namespace symbolic_access
{
	struct BitfieldData
	{
		uint8_t Position;
		uint8_t Length;
	};

	struct Member
	{
		eastl::string Name;
		size_t Offset;
		eastl::optional<BitfieldData> Bitfield;
	};

	using StructMembers = eastl::vector<Member>;
	using StructsMap = eastl::map<eastl::string, StructMembers, eastl::less<>>;
	using SymbolsMap = eastl::map<eastl::string, size_t, eastl::less<>>;
}