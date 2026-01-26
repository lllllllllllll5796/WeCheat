#pragma once

//´òÓ¡µ÷ÊÔÊä³ö
template<typename ...Args>
void OutputDebugStringEx(std::string&& format, Args&&... args)
{
	char* text = new char[8192];

	sprintf(text, format.data(), std::forward<std::remove_reference_t<Args>>(args)...);
	OutputDebugStringA(text);

	delete[] text;
	text = nullptr;
}

template<typename ...Args>
void OutputDebugStringEx(std::wstring&& format, Args&&... args)
{
	wchar_t* text = new wchar_t[8192];

	swprintf(text, format.data(), std::forward<std::remove_reference_t<Args>>(args)...);
	OutputDebugStringW(text);

	delete[] text;
	text = nullptr;
}

#if CONFIG_DEBUG_PRINT_ON
#define LOG_DEBUG(format, ...) \
	OutputDebugStringEx(format, __VA_ARGS__);
#else
#define LOG_DEBUG(format, ...)
#endif

int GetNumberOfProcessors();

void ReportSeriousError(LPCSTR lpText);
