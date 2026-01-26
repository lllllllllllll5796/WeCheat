#include "SymbolicAccess.pch.h"
#include "SymbolicAccess.Log.h"

namespace symbolic_access
{
	void PrintToDebugger(eastl::string_view Format, ...)
	{
		char messageBuffer[1024]{};

		va_list args{};
		va_start(args, Format);
		RtlStringCchVPrintfA(messageBuffer, sizeof(messageBuffer), Format.data(), args);
		va_end(args);

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s", messageBuffer);
	}
}