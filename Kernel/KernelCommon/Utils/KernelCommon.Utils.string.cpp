#include "KernelCommon.pch.h"
#include "KernelCommon.Utils.string.h"

namespace KernelCommon
{
	namespace Utils
	{
		BOOLEAN ValidateUnicodeString(PUNICODE_STRING usStr)
		{
			ULONG i = 0;
			__try
			{
				if (!ImpCall(MmIsAddressValid, usStr))
				{
					return FALSE;
				}

				if (usStr->Buffer == NULL || usStr->Length == 0)
				{
					return FALSE;
				}

				for (i = 0; i < usStr->Length; i++)
				{
					if (!ImpCall(MmIsAddressValid, (PUCHAR)usStr->Buffer + i))
					{
						return FALSE;
					}
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}
			return TRUE;
		}

		eastl::string UnicodeString2String(PUNICODE_STRING UnicodeString)
		{
			eastl::string FullPath;
			ANSI_STRING asName;
			char* buffer = nullptr;

			if (ValidateUnicodeString(UnicodeString))
			{
				auto ns = ImpCall(RtlUnicodeStringToAnsiString, &asName, UnicodeString, TRUE);
				if (NT_SUCCESS(ns))
				{
					buffer = (char*)malloc(asName.MaximumLength);
					if (buffer)
					{
						RtlZeroMemory(buffer, asName.MaximumLength);
						RtlCopyMemory(buffer, asName.Buffer, asName.Length);
						FullPath = buffer;
						free(buffer);
					}
					ImpCall(RtlFreeAnsiString, &asName);
				}
			}
			return FullPath;
		}

		eastl::string ConvertToString(const wchar_t* src, ULONG64 len)
		{
			if (len == -1) {
				len = ImpCall(wcslen, src);
			}
			eastl::string str;
			// dunno why easstl::DecodePart needs more space
			str.resize(len + 64);

			auto srcBegin = src;
			auto srcEnd = src + len;
			auto dstBegin = &str[0];
			auto dstEnd = dstBegin + len + 64;
			eastl::DecodePart(srcBegin, srcEnd, dstBegin, dstEnd);

			return std::move(str);
		}

		eastl::wstring ConvertToWString(const char* src, ULONG64 len)
		{
			if (len == -1) {
				len = strlen(src);
			}
			eastl::wstring wstr;
			wstr.resize(len);

			auto srcBegin = src;
			auto srcEnd = src + len;
			auto dstBegin = &wstr[0];
			auto dstEnd = dstBegin + len;
			eastl::DecodePart(srcBegin, srcEnd, dstBegin, dstEnd);

			return eastl::move(wstr);
		}

		eastl::wstring ConvertToWString(eastl::string const& str)
		{
			return eastl::move(ConvertToWString(str.c_str(), str.size()));
		}

		eastl::string ConvertToString(eastl::wstring const& str)
		{
			return std::move(ConvertToString(str.c_str(), str.size()));
		}

		eastl::wstring UnicodeString2WString(PUNICODE_STRING UnicodeString)
		{
			eastl::string string = UnicodeString2String(UnicodeString);
			return ConvertToWString(string);
		}

		BOOLEAN RtlStringContains(PSTRING Str, PSTRING SubStr, BOOLEAN CaseInsensitive)
		{
			if (Str == NULL || SubStr == NULL || Str->Length < SubStr->Length)
				return FALSE;

			CONST USHORT NumCharsDiff = (Str->Length - SubStr->Length);
			STRING Slice = *Str;
			Slice.Length = SubStr->Length;

			for (USHORT i = 0; i <= NumCharsDiff; ++i, ++Slice.Buffer, Slice.MaximumLength -= 1)
			{
				if (ImpCall(RtlEqualString, &Slice, SubStr, CaseInsensitive))
					return TRUE;
			}
			return FALSE;
		}

		BOOLEAN RtlUnicodeStringContains(PUNICODE_STRING Str, PUNICODE_STRING SubStr, BOOLEAN CaseInsensitive)
		{
			if (Str == NULL || SubStr == NULL || Str->Length < SubStr->Length)
				return FALSE;

			CONST USHORT NumCharsDiff = (Str->Length - SubStr->Length) / sizeof(WCHAR);
			UNICODE_STRING Slice = *Str;
			Slice.Length = SubStr->Length;

			for (USHORT i = 0; i <= NumCharsDiff; ++i, ++Slice.Buffer, Slice.MaximumLength -= sizeof(WCHAR))
			{
				if (ImpCall(RtlEqualUnicodeString, &Slice, SubStr, CaseInsensitive))
					return TRUE;
			}
			return FALSE;
		}

		//ÅÐ¶ÏUNICODE×Ö·û´®ÊÇ·ñÎª¿Õ
		BOOLEAN StrIsValid2(UNICODE_STRING filePath)
		{
			if (filePath.Length == 0)
				return FALSE;
			else
				return TRUE;
		}
	}
}
