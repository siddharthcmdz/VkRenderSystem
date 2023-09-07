
#include <conio.h>

#include "HelloVulkanExample.h"
#include "PrimitiveExample.h"
#include <VkRenderSystem.h>
#include <Windows.h>
#include <iostream>

RSexample* g_example = nullptr;
RSexampleOptions g_exopts;
RSexampleGlobal g_globals;


const RSexampleOptions processArgs(int argc, char** argv) {
	RSexampleOptions exopts;
	for (int i = 1; i < argc; i++) {
		std::string mode = argv[i];
		if (mode == "-example") {
			std::string exname = argv[(i + 1)];
			for (int j = 0; j < RSexampleName::enMax; j++) {
				if (exname == RSexampleNameStr[j]) {
					exopts.example = static_cast<RSexampleName>(j);
					break;
				}
			}
		}
	}
	return exopts;
}

void getShaderPath(RSinitInfo& initInfo) {
	std::string currDir = helper::getCurrentDir();
	std::cout << "current dir: " << currDir << std::endl;
	currDir += "\\shaders";
	strcpy_s(initInfo.shaderPath, currDir.c_str());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {

	auto& vkrs = VkRenderSystem::getInstance();
	switch (umsg) {

	case WM_CREATE:
	{
		RSinitInfo info;
		info.parentHwnd = hwnd;
		info.parentHinst = GetModuleHandle(nullptr);
		sprintf_s(info.appName, "RSexamples");
		info.enableValidation = true;
		info.onScreenCanvas = true;
		getShaderPath(info);
		vkrs.renderSystemInit(info);

		RScontextInfo ctxInfo;
		RECT dim;
		GetWindowRect(hwnd, &dim);
		ctxInfo.initWidth = dim.right - dim.left;
		ctxInfo.initHeight = dim.bottom - dim.top;
		ctxInfo.hwnd = hwnd;
		ctxInfo.hinst = GetModuleHandle(nullptr);
		sprintf_s(ctxInfo.title, g_example->getExampleName().c_str());
		g_globals.width = ctxInfo.initWidth;
		g_globals.height = ctxInfo.initHeight;
		vkrs.contextCreate(g_globals.ctxID, ctxInfo);

		RSview view;
		view.cameraType = CameraType::ORBITAL;
		view.clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		view.dirty = true;
		vkrs.viewCreate(g_globals.viewID, view, g_globals.ctxID);

		if (g_example) {
			g_example->init(g_exopts, g_globals);
		}

		std::cout << "Window created" << std::endl;
		break;
	}

	case WM_MOUSEMOVE:
	{
		InvalidateRect(hwnd, nullptr, FALSE);
		std::cout << "Moused moved" << std::endl;
		break;
	}

	case WM_SIZE:
	{
		g_globals.width = LOWORD(lparam);
		g_globals.height = HIWORD(lparam);

		if (g_example) {
			vkrs.contextResized(g_globals.ctxID, g_globals.viewID, g_globals.width, g_globals.height);
		}
		std::cout << "Window resized - width: "<<g_globals.width<<" , height: "<<g_globals.height << std::endl;
		break;
	}

	case WM_PAINT:
	{
		ValidateRect(hwnd, nullptr);
		if (g_example) {
			g_example->render(g_globals);
		}
		std::cout << "Window painting " << std::endl;
		break;
	}

	case WM_DESTROY:
	{
		if (g_example != nullptr) {
			g_example->dispose(g_globals);
		}
		vkrs.viewDispose(g_globals.viewID);
		vkrs.contextDispose(g_globals.ctxID);
		vkrs.renderSystemDispose();

		DestroyWindow(hwnd);
		PostQuitMessage(0);

		std::cout << "Window destroying" << std::endl;
		break;
	}
	}

	return (DefWindowProc(hwnd, umsg, wparam, lparam));
}

HWND createWindow(std::string appname, std::string title, int width, int height) {
	
	WNDCLASSEX wndclass{};
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = GetModuleHandle(nullptr);
	wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = appname.c_str();
	wndclass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	
	ATOM res = RegisterClassEx(&wndclass);
	if (!res) {
		std::cout << "Could not register window class!" << std::endl;
		fflush(stdout);
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = width;
	windowRect.bottom = height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	HWND window = CreateWindowEx(0,
		appname.c_str(),
		title.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr);

	if (!window) {
		std::cout << "Could not create window!" << std::endl;
		fflush(stdout);
		return nullptr;
	}

	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

	return window;
}

void renderloop() {
	MSG msg;
	bool quitMessageRecevied = false;
	while (!quitMessageRecevied) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				quitMessageRecevied = true;
				break;
			}
		}
	}
}


int main(int argc, char** argv) {
	g_exopts = processArgs(argc, argv);
	switch (g_exopts.example) {
	case RSexampleName::enHelloVulkan:
		g_example = new HelloVulkanExample();
		break;

	case RSexampleName::enPrimitiveType:
		g_example = new PrimitiveExample();
		break;

	case RSexampleName::enMax:
	default:
		g_example = new HelloVulkanExample();
	}
	
	HWND window = createWindow("RSexamples", "main", 800, 600);
	renderloop();

	getchar();
	return 0;
}