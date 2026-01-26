#pragma once

bool ContainsString(const std::string& str, const std::string& strToFind);

std::string ReplaceString(std::string str, const std::string& to_find, const std::string& to_replace);

bool EndsWith(const std::string& value, const std::string& ending);

bool IsNumber(const std::string& s);

bool IsHexNumber(const std::string& s);

std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter);

std::wstring RandomWstring(std::size_t length);

std::string RandomString(std::size_t length);

std::wstring trim(const std::wstring& s);

std::wstring to_hex_string(uintptr_t i);

const std::string ws2s(const std::wstring& src);

const std::wstring s2ws(const std::string& src);

static std::string ws2utf8(const std::wstring& wstr);

static std::wstring utf82ws(const std::string& str);