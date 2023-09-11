#include "helper.h"
#include <locale>
#include <codecvt>
#include <string>

namespace helper {
	std::string ConvertWideToUtf8(const std::wstring& wstr)	{
		int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), NULL, 0, NULL, NULL);
		std::string str(count, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
		
		return str;
	}

	std::wstring ConvertUtf8ToWide(const std::string& str) {
		int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], count);
		
		return wstr;
	}

	std::string getCurrentDir() {
		TCHAR buffer[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");

		const std::wstring& wstr = std::wstring(buffer).substr(0, pos);
		std::string str = ConvertWideToUtf8(wstr);
		return str;
	}
}