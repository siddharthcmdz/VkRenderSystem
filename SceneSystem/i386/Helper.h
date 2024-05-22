#pragma once
#include <rsids.h>
#include <Windows.h>
#include <string>
#include <rsenums.h>

namespace helper {

	std::string getCurrentDir();

	std::wstring ConvertUtf8ToWide(const std::string& str);
	std::string ConvertWideToUtf8(const std::wstring& wstr);
}

