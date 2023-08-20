#include "helper.h"
#include <locale>
#include <codecvt>
#include <string>

namespace helper {
    std::string wideTostring(const std::wstring& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
        char* buf = new char[len];
        WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
        std::string r(buf);
        delete[] buf;
        return r;
    }

	std::string getCurrentDir() {
		TCHAR buffer[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");

		std::wstring wstr = std::wstring(buffer).substr(0, pos);
		return wideTostring(wstr);
	}
}