#include "WeVt.pch.h"
#include "WeVt.Hypervisor.h"

namespace Hypervisor
{
	BOOL SuportVT()
	{
		return VT::SuportVT();
	}

	BOOL EnableVT()
	{
		return VT::EnableVT();
	}

	void DisableVT()
	{
		//DBVM≤ª÷ß≥÷–∂‘ÿ
	}
}