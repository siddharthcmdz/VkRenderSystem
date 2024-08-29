#pragma once

#include <Windows.h>
#include <string>
#include <RSdataTypes.h>

struct GlobalContext {
	uint32_t width, height;
	RScontextID ctxID;
	RSviewID viewID;
	std::string appName;
};

struct AppInfo {
	std::string name;
};

class App
{
private:
	std::string _name;

public:
	void init(const AppInfo& appInfo, const GlobalContext& gctx);
	std::string getExampleName() const;

	void render(const GlobalContext& ctx) const;

	void dispose(const GlobalContext& ctx);
};