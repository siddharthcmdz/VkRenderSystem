
#include <conio.h>

#include "HelloVulkanExample.h"
#include "PrimitiveExample.h"
#include "ModelLoadExample.h"
#include "QuadricExample.h"
#include "VolumeSliceExample.h"
#include "RenderableUtils.h"
#include <VkRenderSystem.h>
#include <Windows.h>
#include <iostream>
#include <glm/glm.hpp>
#include "Camera.h"


struct GridData
{
	RScollectionID collectionID;
	ss::GridID gridID;
	ss::GridInfo gridInfo;
};

struct TriadData
{
	RScollectionID collectionID;
	ss::TriadID triadID;
	ss::TriadInfo triadInfo;
};

RSexample* g_example = nullptr;
RSexampleOptions g_exopts;
RSexampleGlobal g_globals;
Camera g_camera;
glm::vec2 g_mousePos;
GridData _gridData;
TriadData _triadData;

struct MouseButtons {
	bool left = false;
	bool right = false;
	bool middle = false;
};

MouseButtons g_mouseButtons;

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

void handleMouseMove(int32_t x, int32_t y) {
	int32_t dx = (int32_t)g_mousePos.x - x;
	int32_t dy = (int32_t)g_mousePos.y - y;
	//std::cout << "dx: " << dx << ", dy: " << dy << std::endl;
	bool handled = false;

	if (handled) {
		g_mousePos = glm::vec2((float)x, (float)y);
		return;
	}

	if (g_mouseButtons.left) {
		glm::vec3 rotationDelta = glm::vec3(dy * g_camera.rotationSpeed, -dx * g_camera.rotationSpeed, 0.0f);
		std::cout << "rotation delta: " << rotationDelta.x << ", " << rotationDelta.y << ", " << rotationDelta.z << std::endl;
		g_camera.rotate(rotationDelta);
	}
	if (g_mouseButtons.right) {
		glm::vec3 translationDelta = glm::vec3(-0.0f, 0.0f, dy * .005f);
		g_camera.translate(translationDelta);
	}
	if (g_mouseButtons.middle) {
		glm::vec3 translationDelta = glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f);
		g_camera.translate(translationDelta);
	}
	g_mousePos = glm::vec2((float)x, (float)y);
}

void print(const glm::vec2 pt) {
	std::cout << "(" << pt.x << ", " << pt.y << ")" << std::endl;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {

	auto& vkrs = VkRenderSystem::getInstance();
	switch (umsg) 
	{
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
			float aspectRatio = (float)ctxInfo.initWidth / (float)ctxInfo.initHeight;
			g_camera.updateAspectRatio(aspectRatio);
			g_camera.translate(glm::vec3(2.0f, 2.0f, 2.0f));
			view.viewmat = g_camera.getViewMatrix();
			view.projmat = g_camera.getProjectionMatrix();
			view.dirty = true;
			vkrs.viewCreate(g_globals.viewID, view, g_globals.ctxID);
			vkrs.viewUpdate(g_globals.viewID, view);
		
			ss::BoundingBox worldBounds(glm::vec4(-0.5f, 0.0f, -0.5f, 1), glm::vec4(0.5f, 0.0f, 0.5f, 1.0f));
			if (g_example) {
				g_example->init(g_exopts, g_globals);
				worldBounds = g_example->getBounds();
			}

			//create grid collection and data
			RScollectionInfo gridCollInfo;
			gridCollInfo.maxInstances = 1;
			gridCollInfo.collectionName = "floor-grid";
			vkrs.collectionCreate(_gridData.collectionID, gridCollInfo);

			float floorSize = worldBounds.getDiagonal();
			_gridData.gridInfo.resolution = 10;
			_gridData.gridInfo.size = floorSize;
			_gridData.gridID = RenderableUtils::GridCreate(_gridData.gridInfo, _gridData.collectionID);
			vkrs.collectionFinalize(_gridData.collectionID, g_globals.ctxID, g_globals.viewID);
			vkrs.viewAddCollection(g_globals.viewID, _gridData.collectionID);

			//Create triad collection and data
			RScollectionInfo triadCollInfo;
			triadCollInfo.maxInstances = 1;
			triadCollInfo.collectionName = "triad";
			vkrs.collectionCreate(_triadData.collectionID, triadCollInfo);

			_triadData.triadInfo.size = worldBounds.getDiagonal() * 0.2f;
			_triadData.triadID = RenderableUtils::triadCreate(_triadData.triadInfo, _triadData.collectionID);
			vkrs.collectionFinalize(_triadData.collectionID, g_globals.ctxID, g_globals.viewID);
			vkrs.viewAddCollection(g_globals.viewID, _triadData.collectionID);

			std::cout << "Window created" << std::endl;
			break;
		}

		case WM_MOUSEMOVE: 
		{
			handleMouseMove(LOWORD(lparam), HIWORD(lparam));
			auto& vkrs = VkRenderSystem::getInstance();
			std::optional<RSview> optview = vkrs.viewGetData(g_globals.viewID);
			if (optview.has_value()) {
				optview->viewmat = g_camera.getViewMatrix();
				vkrs.viewUpdate(g_globals.viewID, *optview);
			}
			InvalidateRect(hwnd, nullptr, FALSE);
			//std::cout << "Moused moved" << std::endl;
			break;
		}

		case WM_LBUTTONDOWN: 
		{
			g_mousePos = glm::vec2((float)LOWORD(lparam), (float)HIWORD(lparam));
			print(g_mousePos);
			g_mouseButtons.left = true;
			break;
		}

		case WM_RBUTTONDOWN: 
		{
			g_mousePos = glm::vec2((float)LOWORD(lparam), (float)HIWORD(lparam));
			g_mouseButtons.right = true;
			break;
		}

		case WM_MBUTTONDOWN: 
		{
			g_mousePos = glm::vec2((float)LOWORD(lparam), (float)HIWORD(lparam));
			g_mouseButtons.middle = true;
			break;
		}

		case WM_LBUTTONUP: 
		{
			g_mouseButtons.left = false;
			break;
		}

		case WM_RBUTTONUP: 
		{
			g_mouseButtons.right = false;
			break;
		}

		case WM_MBUTTONUP: 
		{
			g_mouseButtons.middle = false;
			break;
		}

		case WM_MOUSEWHEEL: 
		{
			short wheelDelta = GET_WHEEL_DELTA_WPARAM(wparam);
			g_camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f));
			break;
		}

		case WM_SIZE: 
		{
			g_globals.width = LOWORD(lparam);
			g_globals.height = HIWORD(lparam);
			float aspectRatio = (float)g_globals.width / (float)g_globals.height;
			std::cout << "new aspect ratio: " << aspectRatio << std::endl;
			g_camera.updateAspectRatio(aspectRatio);
			std::optional<RSview> view = vkrs.viewGetData(g_globals.viewID);
			if (view.has_value()) {
				view->projmat = g_camera.getProjectionMatrix();
				vkrs.viewUpdate(g_globals.viewID, *view);
			}
			if (g_example) { 
				vkrs.contextResized(g_globals.ctxID, g_globals.viewID, g_globals.width, g_globals.height);
			}
			std::cout << "Window resized - width: "<<g_globals.width<<" , height: "<<g_globals.height << std::endl;
			break;
		}

		case WM_PAINT: 
		{
			ValidateRect(hwnd, nullptr);
			auto& vkrs = VkRenderSystem::getInstance();
			//Draw the collections added to the view
			{
				vkrs.contextDrawCollections(g_globals.ctxID, g_globals.viewID);
			}
			if (g_example) {
				g_example->render(g_globals);
			}
			//std::cout << "Window painting " << std::endl;
			break;
		}

		case WM_DESTROY: 
		{
			//dispose the grid data
			RenderableUtils::GridDispose(_gridData.gridID, _gridData.collectionID);
			vkrs.collectionDispose(_gridData.collectionID);
			if (g_example != nullptr) {
				g_example->dispose(g_globals);
			}
			vkrs.viewDispose(g_globals.viewID);
			vkrs.contextDispose(g_globals.ctxID);
			vkrs.renderSystemDispose();

			std::cout << "Window destroying" << std::endl;
			break;
		}
	}

	return (DefWindowProc(hwnd, umsg, wparam, lparam));
}

HWND createWindow(std::wstring appname, std::wstring title, int width, int height) {
	
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

	case RSexampleName::enModelLoad:
		g_example = new ModelLoadExample();
		break;

	case RSexampleName::enQuadric:
		g_example = new QuadricExample();
		break;

	case RSexampleName::enVolumeSlice:
		g_example = new VolumeSliceExample();
		break;

	case RSexampleName::enMax:
	default:
		g_example = new HelloVulkanExample();
	}
	
	HWND window = createWindow(L"RSexamples", L"main", 800, 600);
	renderloop();

	getchar();
	return 0;
}