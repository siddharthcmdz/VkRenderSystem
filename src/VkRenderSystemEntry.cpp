#include "VkRenderSystemEntry.h"
#include "VkRenderSystem.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>           //for uint32_t
#include <limits>            //for std::numeric_limits
#include <algorithm>         //for std::clamp
#include <fstream>           //for file reading

class VkScene {
private:
	static const inline uint32_t WIDTH = 800, HEIGHT = 600;

	void init();
	void mainLoop();
	void cleanup();

public:
	void run() {
		init();
		mainLoop();
		cleanup();
	}
};

void VkScene::cleanup() {

}

void VkScene::mainLoop() {
	while (!glfwWindowShouldClose(iwindow)) {
		glfwPollEvents();
		drawFrame();
	}
}

void VkScene::init() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	iwindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

}


int RSmain() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "VkRSexample");
	info.enableValidation = true;
	vkrs.renderSystemInit(info);

	return 0;
}