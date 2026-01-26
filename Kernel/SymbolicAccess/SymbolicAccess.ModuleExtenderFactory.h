#pragma once

#include "SymbolicAccess.ModuleExtender.h"
#include "SymbolicAccess.PdbGrabber.h"

namespace symbolic_access
{
	class ModuleExtenderFactory
	{
	public:
		ModuleExtenderFactory();
		ModuleExtenderFactory(const ModuleExtenderFactory&) = delete;
		ModuleExtenderFactory& operator=(const ModuleExtenderFactory&) = delete;
		ModuleExtenderFactory(ModuleExtenderFactory&&) = default;
		ModuleExtenderFactory& operator=(ModuleExtenderFactory&&) = default;

		eastl::optional<ModuleExtender> Create(eastl::wstring_view ModuleName);
	private:
		PdbGrabber m_PdbGrabber;
	};
}