#pragma once

#include "SymbolicAccess.ModuleData.h"

namespace symbolic_access
{
	class ModuleExtender
	{
	public:
		ModuleExtender(size_t ModuleBaseAddress, StructsMap&& Structs, SymbolsMap&& Symbols) :
			m_ModuleBaseAddress(ModuleBaseAddress), m_Structs(eastl::move(Structs)), m_Symbols(eastl::move(Symbols))
		{
		}

		ModuleExtender(ModuleExtender const&) = delete;
		ModuleExtender& operator=(ModuleExtender const&) = delete;
		ModuleExtender(ModuleExtender&&) = default;
		ModuleExtender& operator=(ModuleExtender&&) = default;

		// Return pointer of T of symbol in module
		template<typename T>
		T* GetPointer(eastl::string_view SymbolName) const
		{
			const auto& iter = m_Symbols.find(eastl::string{ SymbolName });
			if (iter == m_Symbols.end())
				return nullptr;

			return reinterpret_cast<T*>(m_ModuleBaseAddress + iter->second);
		}

		// Return pointer of T of structure member at given object address
		template<typename T>
		T* GetPointer(eastl::string_view StructName, eastl::string_view MemberName, void* ObjectAddress) const
		{
			return GetExtendedPointer<T>(StructName, MemberName, ObjectAddress).GetRaw();
		}

		// Return copy of value of T of symbol in module
		template<typename T>
		eastl::optional<T> Get(eastl::string_view SymbolName) const
		{
			const auto pointer = GetPointer<T>(SymbolName);
			if (!pointer)
				return {};

			return *pointer;
		}

		// Return copy of value of T of structure member at given object address
		template<typename T>
		eastl::optional<T> Get(eastl::string_view StructName, eastl::string_view MemberName, void* ObjectAddress) const
		{
			const auto pointer = GetExtendedPointer<T>(StructName, MemberName, ObjectAddress);
			if (!pointer)
				return {};

			return *pointer;
		}

		// Return offset of member inside structure
		eastl::optional<size_t> GetOffset(eastl::string_view StructName, eastl::string_view MemberName) const
		{
			const auto& structsIter = m_Structs.find(eastl::string{ StructName });
			if (structsIter == m_Structs.end())
				return {};

			const auto& membersIter = eastl::find_if(structsIter->second.begin(), structsIter->second.end(),
				[&](const auto& MemberNameAndOffset) { return MemberNameAndOffset.Name == MemberName; });

			if (membersIter == structsIter->second.end())
				return {};

			return membersIter->Offset;
		}

		// Set value of symbol in module
		// Returns false if symbol wasn't found
		template<typename T>
		bool Set(eastl::string_view SymbolName, T Value) const
		{
			const auto pointer = GetPointer<T>(SymbolName);
			if (!pointer)
				return false;

			*pointer = Value;
			return true;
		}

		// Set value of structure member at given object address
		// Returns false if structure or member wasn't found
		template<typename T>
		bool Set(eastl::string_view StructName, eastl::string_view MemberName, void* ObjectAddress, T Value) const
		{
			const auto pointer = GetExtendedPointer<T>(StructName, MemberName, ObjectAddress);
			if (!pointer)
				return false;

			pointer = Value;
			return true;
		}

		// Call a function with custom number of arguments with non void return type
		template <typename Result, typename... Args, eastl::enable_if_t<!eastl::is_void_v<Result>, int> = 0>
		eastl::optional<Result> Call(eastl::string_view FunctionName, Args&& ... args) const
		{
			const auto pointer = GetPointer<void>(FunctionName);
			if (!pointer)
				return {};

			return reinterpret_cast<Result(*)(Args...)>(pointer)(eastl::forward<Args>(args)...);
		}

 		// Call a function with custom number of arguments with void return type
 		template <typename Result = void, typename... Args, eastl::enable_if_t<eastl::is_void_v<Result>, int> = 0>
 		void Call(eastl::string_view FunctionName, Args&& ... args) const
 		{
 			const auto pointer = GetPointer<void>(FunctionName);
 			if (!pointer)
 				return;
 
 			return reinterpret_cast<Result(*)(Args...)>(pointer)(eastl::forward<Args>(args)...);
 		}
	private:
		template <typename T>
		class ExtendedPointer
		{
		public:
			ExtendedPointer(void* Pointer, eastl::optional<BitfieldData> BitfieldData) :
				m_Pointer(reinterpret_cast<T*>(Pointer)), m_BitfieldData(BitfieldData)
			{
			}

			T* GetRaw() const
			{
				return m_Pointer;
			}

			T operator*() const
			{
				if constexpr (eastl::is_arithmetic_v<T>)
				{
					if (m_BitfieldData.has_value())
					{
						const auto mask = (1 << m_BitfieldData->Length) - 1;
						return (*m_Pointer >> m_BitfieldData->Position) & mask;
					}
				}

				return *m_Pointer;
			}

			void operator =(T Value) const
			{
				if constexpr (eastl::is_arithmetic_v<T>)
				{
					if (m_BitfieldData.has_value())
					{
						const auto mask = (1 << m_BitfieldData->Length) - 1;
						Value &= mask;
						Value <<= m_BitfieldData->Position;
						*m_Pointer |= Value;

						return;
					}
				}

				*m_Pointer = Value;
			}

			bool operator!() const
			{
				return !m_Pointer;
			}

		private:
			T* m_Pointer;
			eastl::optional<BitfieldData> m_BitfieldData;
		};

		template<typename T>
		ExtendedPointer<T> GetExtendedPointer(eastl::string_view StructName, eastl::string_view MemberName, void* ObjectAddress) const
		{
			const auto& structsIter = m_Structs.find(StructName);
			if (structsIter == m_Structs.end())
				return { nullptr, {} };

			const auto& membersIter = eastl::find_if(structsIter->second.begin(), structsIter->second.end(),
				[&](const auto& MemberNameAndOffset) { return MemberNameAndOffset.Name == MemberName; });

			if (membersIter == structsIter->second.end())
				return { nullptr, {} };

			return ExtendedPointer<T>(reinterpret_cast<uint8_t*>(ObjectAddress) + membersIter->Offset, membersIter->Bitfield);
		}

		size_t m_ModuleBaseAddress;
		StructsMap m_Structs;
		SymbolsMap m_Symbols;
	};
}