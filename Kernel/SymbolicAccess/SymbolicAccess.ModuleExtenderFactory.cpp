#include "SymbolicAccess.pch.h"
#include "SymbolicAccess.ModuleExtenderFactory.h"
#include "SymbolicAccess.ProcessCreatorFactory.h"
#include "SymbolicAccess.ModuleFinder.h"
#include "SymbolicAccess.MsfReader.h"
#include "SymbolicAccess.SymbolExtractor.h"
#include "SymbolicAccess.StructExtractor.h"

namespace symbolic_access
{
	ModuleExtenderFactory::ModuleExtenderFactory() : m_PdbGrabber(ProcessCreatorFactory::Create())
	{
	}

	eastl::optional<ModuleExtender> ModuleExtenderFactory::Create(eastl::wstring_view ModuleName)
	{
		const auto moduleBaseAddress = GetModuleAddress(ModuleName);
		if (!moduleBaseAddress)
		{
			PrintDbg("Failed to get address of module %S\n", ModuleName.data());
			return {};
		}

		auto fileStream = m_PdbGrabber.GetPdbFileStream(moduleBaseAddress);
		if (!fileStream.has_value())
		{
			PrintDbg("Failed to get pdb file stream of module %S\n", ModuleName.data());
			return {};
		}

		MsfReader msfReader(std::move(*fileStream));
		if (!msfReader.Initialize())
		{
			PrintDbg("Failed to initialize msf reader for %S\n", ModuleName.data());
			return {};
		}

		SymbolsExtractor symbolsExtractor(msfReader);
		auto symbols = symbolsExtractor.Extract();
		if (symbols.empty())
		{
			PrintDbg("Failed to extract symbols for %S\n", ModuleName.data());
			return {};
		}

		return ModuleExtender{ moduleBaseAddress, StructExtractor(msfReader).Extract(), std::move(symbols) };
	}

}