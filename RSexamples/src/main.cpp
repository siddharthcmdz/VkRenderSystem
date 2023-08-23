
#include <conio.h>

#include "HelloVulkanExample.h"
#include "PrimitiveExample.h"
#include <Windows.h>
#include <iostream>

enum RSexampleName {
	enHelloVulkan,
	enPrimitiveType,
	enMax
};

const std::string RSexampleNameStr[] = {
	"HelloVulkan",
	"PrimitiveType",
	"None"
};

struct RSexampleOptions {
	RSexampleName example;
};

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

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {

	switch (umsg) {
	case WM_DESTROY:
		std::cout << "Destroying window" << std::endl;
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;


	case WM_CREATE:
		std::cout << "Creating window" << std::endl;
		break;

	case WM_PAINT:
		ValidateRect(hwnd, nullptr);
		std::cout << "Painting window" << std::endl;
		break;
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
	const RSexampleOptions& exopts = processArgs(argc, argv);
	RSexample* example = nullptr;
	switch (exopts.example) {
	case RSexampleName::enHelloVulkan:
		example = new HelloVulkanExample();
		break;

	case RSexampleName::enPrimitiveType:
		example = new PrimitiveExample();
		break;

	case RSexampleName::enMax:
	default:
		example = new HelloVulkanExample();
	}
	
	//HWND window = createWindow("RSexamples", "main", 800, 600);
	//renderloop();

	example->init();
	example->render();
	example->dispose();


	getchar();
	return 0;
}