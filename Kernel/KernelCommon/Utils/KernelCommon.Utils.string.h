#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		BOOLEAN ValidateUnicodeString(PUNICODE_STRING usStr);

		eastl::string UnicodeString2String(PUNICODE_STRING UnicodeString);

		eastl::string ConvertToString(const wchar_t* src, ULONG64 len);

		eastl::wstring ConvertToWString(const char* src, ULONG64 len);

		eastl::wstring ConvertToWString(eastl::string const& str);

		eastl::string ConvertToString(eastl::wstring const& str);

		eastl::wstring UnicodeString2WString(PUNICODE_STRING UnicodeString);

		BOOLEAN RtlStringContains(PSTRING Str, PSTRING SubStr, BOOLEAN CaseInsensitive);

		BOOLEAN RtlUnicodeStringContains(PUNICODE_STRING Str, PUNICODE_STRING SubStr, BOOLEAN CaseInsensitive);

		BOOLEAN StrIsValid2(UNICODE_STRING filePath);
	}
}
