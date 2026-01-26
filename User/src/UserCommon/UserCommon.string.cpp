#include "UserCommon.pch.h"
#include "UserCommon.string.h"

std::string ReplaceString(std::string str, const std::string& to_find, const std::string& to_replace)
{
	if (to_find.empty())
		return str;

	for (size_t position = str.find(to_find); position != std::string::npos; position = str.find(to_find, position))
		str.replace(position, to_find.length(), to_replace);
	return str;
}

bool ContainsString(const std::string& str, const std::string& strToFind)
{
	return str.find(strToFind) != std::string::npos;
}
 
bool EndsWith(const std::string& value, const std::string& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
 
bool IsNumber(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
 		s.end(), [](const char c) { return !std::isdigit(c); }) == s.end();
}
 
bool IsHexNumber(const std::string& s)
{
	return std::all_of(s.begin(), s.end(), [](const unsigned char c) { return std::isxdigit(c); });
}
 
std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter)
{
	std::vector<std::string> strings;

	std::string::size_type pos;
	std::string::size_type prev = 0;
	while ((pos = str.find(delimiter, prev)) != std::string::npos)
	{
 		strings.push_back(str.substr(prev, pos - prev));
 		prev = pos + 1;
	}
 
	// To get the last substring (or only, if delimiter is not found)
	strings.emplace_back(str.substr(prev));

	return strings;
}

std::wstring RandomWstring(std::size_t length)
{
	srand(time(NULL));

	auto randchar = []() -> wchar_t
		{
			const wchar_t charset[] =
				L"0123456789"
				L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				L"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[rand() % max_index];
		};

	std::wstring str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

std::string RandomString(std::size_t length)
{
	const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string res(length, 0);

	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<> distribution(0, (int)chars.size() - 1);

	for (std::size_t i = 0; i < length; ++i) {
		res[i] += chars[(int)distribution(generator)];
	}

	return res;
}

std::wstring trim(const std::wstring& s)
{
	auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return std::isspace(c); });
	auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return std::isspace(c); }).base();
	return (wsback <= wsfront ? std::wstring() : std::wstring(wsfront, wsback));
}

std::wstring to_hex_string(uintptr_t i)
{
	std::wstringstream s;
	s << (const wchar_t*)L"0x" << std::hex << i;
	return s.str();
}

const std::string ws2s(const std::wstring& src)
{
#if 1
	const std::locale sys_locale("");

	const wchar_t* data_from = src.c_str();
	const wchar_t* data_from_end = src.c_str() + src.size();
	const wchar_t* data_from_next = 0;

	int wchar_size = 4;
	char* data_to = new char[(src.size() + 1) * wchar_size];
	char* data_to_end = data_to + (src.size() + 1) * wchar_size;
	char* data_to_next = 0;

	memset(data_to, 0, (src.size() + 1) * wchar_size);

	typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
	mbstate_t out_state = { 0 };
	const auto result = std::use_facet<convert_facet>(sys_locale).out(
		out_state, data_from, data_from_end, data_from_next,
		data_to, data_to_end, data_to_next);
	if (result == convert_facet::ok)
	{
		std::string dst = data_to;
		delete[] data_to;
		return dst;
	}
	else
	{
		//printf("convert error!\n");
		delete[] data_to;
		return std::string("");
	}
#else

#endif

	return "";
}

const std::wstring s2ws(const std::string& src)
{
#if 1
	std::locale sys_locale("");

	const char* data_from = src.c_str();
	const char* data_from_end = src.c_str() + src.size();
	const char* data_from_next = 0;

	wchar_t* data_to = new wchar_t[src.size() + 1];
	wchar_t* data_to_end = data_to + src.size() + 1;
	wchar_t* data_to_next = 0;

	wmemset(data_to, 0, src.size() + 1);

	typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
	mbstate_t in_state = { 0 };
	const auto result = std::use_facet<convert_facet>(sys_locale).in(
		in_state, data_from, data_from_end, data_from_next,
		data_to, data_to_end, data_to_next);
	if (result == convert_facet::ok)
	{
		std::wstring dst = data_to;
		delete[] data_to;
		return dst;
	}
	else
	{
		//printf("convert error!\n");
		delete[] data_to;
		return std::wstring(L"");
	}
#endif
	return L"";
}

std::string uctail(unsigned int cnt, unsigned int uc)
{
	char c;

	std::string str("");

	for (int m = cnt - 1; m >= 0; m--)
	{
		c = static_cast<char>((((uc >> (m * 6)) & 0x3F) | 0x80));

		str.append(1, c);
	}

	return str;
}

std::string uc2utf8(unsigned int uc)
{
    std::string str("");

    char c;

    if (uc < 128)
    {
        c = (char)uc;
        str.append(1, c);
        return str;
    }

    if (uc < (0x0800))
    {
        c = (((uc >> 6) & 0x1F) | 0xc0);

        str.append(1, c);

        str += uctail(1, uc);

        return str;
    }

    if (uc < (0x010000))
    {
        c = (((uc >> 12) & 0x0F) | 0xe0);

        str.append(1, c);
        str += uctail(2, uc);

        return str;
    }

    if (uc < (0x200000))
    {
        c = (((uc >> 18) & 0x07) | 0xf0);

        str.append(1, c);
        str += uctail(3, uc);

        return str;
    }

    if (uc < (0x04000000))
    {
        c = (((uc >> 24) & 0x03) | 0xf8);

        str.append(1, c);
        str += uctail(4, uc);

        return str;
    }

    if (uc < (0x80000000))
    {
        c = (((uc >> 30) & 0x01) | 0xfc);

        str.append(1, c);
        str += uctail(5, uc);

        return str;
    }

    return str;
}

std::string ws2utf8(const std::wstring& wstr)
{
    std::string str("");

    for (unsigned int i = 0; i < wstr.length(); i++)
    {
        str += uc2utf8(wstr[i]);
    }

    return str;
}

unsigned int nbts(unsigned char s)
{
    int m = 0;

    for (int m = 0; m < 8; m++)
    {
        if (((s << m) & 0x80) == 0)
        {
            return m == 0 ? 1 : m;
        }
    }

    return m;
}

inline unsigned int utf82uc(const std::string& str, unsigned int stridx, unsigned int& charcnt)
{
    unsigned char c = (unsigned char)str[stridx];

    charcnt = nbts(c);

    if (charcnt == 1)
    {
        return (unsigned int)c;
    }

    unsigned int uc = (((1 << (7 - charcnt)) - 1) & str[stridx]) << (charcnt - 1) * 6;

    stridx++;

    for (unsigned int i = 1; i < charcnt; stridx++, i++)
    {
        c = (unsigned char)str[stridx];

        uc += ((c & 0x3F) << (charcnt - 1 - i) * 6);
    }

    return uc;
}

std::wstring utf82ws(const std::string& str)
{
    wchar_t wcharbuf[128];

    unsigned int strlength = (unsigned int)str.length();
    unsigned int stridx = 0;
    unsigned int charcnt;

    unsigned int wcharcnt = 0;

    while (strlength > 0)
    {
        wcharbuf[wcharcnt] = utf82uc(str, stridx, charcnt);

        stridx += charcnt;
        strlength -= charcnt;
        wcharcnt++;
    }

    return std::wstring(wcharbuf, wcharcnt);
}

/**
 *
 * UTF8 Conversion Utitilities
 *
 * When conversion fails, an empty string is returned.
 *
 */

std::string wtoutf8(const std::wstring& wstr)
{
// #if defined(_WIN32) && _MSC_VER >= 1600
//     std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
// 
//     return conv.to_bytes(wstr);
// #else
    return ws2utf8(wstr);
//#endif
}

std::wstring utf8tow(const std::string& str)
{
// #if defined(_WIN32) && _MSC_VER >= 1600
// 
//     std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
// 
//     return conv.from_bytes(str);
// #else
    return utf82ws(str);
//#endif
}