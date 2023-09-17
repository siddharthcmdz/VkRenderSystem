#pragma once
#include <rsids.h>
#include <Windows.h>
#include <string>
#include <rsenums.h>

namespace helper {


	std::string getCurrentDir();

	std::string toString(std::wstring wstr);
}
