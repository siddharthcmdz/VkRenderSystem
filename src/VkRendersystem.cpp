#include "VkRenderSystem.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
#include <set>

#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include "RSVkDebugUtils.h"

bool VkRenderSystem::checkValidationLayerSupport() const {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "\nAvailable layers:" << std::endl;
	for (const char* layerName : VkRSinstance::validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			std::cout << "\tlayername: " << layerProperties.layerName << std::endl;
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> VkRenderSystem::getRequiredExtensions(const RSinitInfo& info) const {

	std::vector<const char*> extensions;
	if (info.onScreenCanvas) {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		extensions = std::vector(glfwExtensions, glfwExtensions + glfwExtensionCount);
	}

	if (info.enableValidation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VkRenderSystem::populateInstanceData(VkRSinstance& inst, const RSinitInfo& info) {
	const std::vector<const char*> exts = getRequiredExtensions(info);
	for (const char* ext : exts) {
		inst.vkRequiredExtensions.push_back(ext);
	}

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
	for (const auto& prop : extensionProperties) {
		inst.vkExtensionProps.push_back(prop.extensionName);
	}
}

void VkRenderSystem::createInstance(const RSinitInfo& info) {
	if (info.enableValidation && !checkValidationLayerSupport()) {
		std::runtime_error("Validation layers requested by not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = info.appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VkRenderSystem";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (info.enableValidation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(VkRSinstance::validationLayers.size());
		createInfo.ppEnabledLayerNames = VkRSinstance::validationLayers.data();
		{
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			debugCreateInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = debugCallback;
			debugCreateInfo.pUserData = nullptr; // Optional
		}
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	auto extensions = getRequiredExtensions(info);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &iinstance.instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	//retrieve a list of supported extensions
	populateInstanceData(iinstance, info);

}

void VkRenderSystem::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}

void VkRenderSystem::setupDebugMessenger() {
	if (!iinstance.enableValidation)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(iinstance.instance, &createInfo, nullptr, &iinstance.debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

}

void VkRenderSystem::createSurface(VkRScontext& vkrsctx) {
	
	const VkResult result = glfwCreateWindowSurface(iinstance.instance, vkrsctx.window, nullptr, &vkrsctx.surface);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create a window surface!");
	}
}

void VkRenderSystem::renderSystemDrawLoop(const RScontextID& ctxID, const RSviewID& viewID) {
	if (iinitInfo.onScreenCanvas) {
		if (ctxID.isValid() && ictxMap.find(ctxID.id) != ictxMap.end()) {
			const VkRScontext& ctx = ictxMap[ctxID.id];
			while (!glfwWindowShouldClose(ctx.window)) {
				glfwPollEvents();
				//drawFrame();
			}
		}
	}

	//vkDeviceWaitIdle(idevice);
}

VkRSqueueFamilyIndices VkRenderSystem::findQueueFamilies(VkPhysicalDevice device, VkRScontext& ctx) {
	VkRSqueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i) {
		const VkQueueFamilyProperties& queueFamilyProp = queueFamilyProperties[i];
		if (queueFamilyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, ctx.surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

VkRSswapChainSupportDetails VkRenderSystem::querySwapChainSupport(VkPhysicalDevice device, VkRScontext& ctx) {
	VkRSswapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, ctx.surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, ctx.surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, ctx.surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, ctx.surface, &presentModeCount, nullptr);
	if (presentModeCount) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, ctx.surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool VkRenderSystem::isDeviceSuitable(VkPhysicalDevice device, VkRScontext& ctx) {
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	VkRSqueueFamilyIndices indices = findQueueFamilies(device, ctx);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		VkRSswapChainSupportDetails swapChainSupport = querySwapChainSupport(device, ctx);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VkRenderSystem::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t availableExtensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void VkRenderSystem::printPhysicalDeviceInfo(VkPhysicalDevice device) {
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceFeatures(device, &features);
	vkGetPhysicalDeviceProperties(device, &props);
	std::cout << "\n" << "Device properties" << std::endl;
	std::cout << "=================" << std::endl;
	std::cout << "\tDevice name: " << props.deviceName << std::endl;
	std::cout << "\tDriver version: " << props.driverVersion << std::endl;
	std::cout << "\tDevice Type: " << props.deviceType << std::endl;
	std::cout << "\nFeatures" << std::endl;
	std::cout << "===========" << std::endl;
	std::cout << "\tmultiDrawIndirect: " << features.multiDrawIndirect << std::endl;
	std::cout << "\tshaderInt64: " << features.shaderInt64 << std::endl;
	std::cout << "\tfragmentStoresAndAtomics: " << features.fragmentStoresAndAtomics << std::endl;
}

void VkRenderSystem::setPhysicalDevice(VkRSinstance& vkrsinst, VkRScontext& ctx) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkrsinst.instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkrsinst.instance, &deviceCount, devices.data());
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, ctx)) {
			vkrsinst.physicalDevice = device;
			printPhysicalDeviceInfo(device);
			break;
		}
	}

	if (vkrsinst.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable card");
	}
}

void VkRenderSystem::createLogicalDevice() {

}

RSresult VkRenderSystem::renderSystemInit(const RSinitInfo& info)
{
	irenderOnscreen = info.onScreenCanvas;
	iinitInfo = info;
	std::vector<const char*> extensions;
	if (info.onScreenCanvas) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	}
	createInstance(info);
	setupDebugMessenger();
	
	createLogicalDevice();
	return RSresult::SUCCESS;
}

bool VkRenderSystem::isRenderSystemInit()
{
	return false;
}

RSresult VkRenderSystem::renderSystemDispose()
{
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::contextCreate(RScontextID& outCtxID, const RScontextInfo& info) {

	RSuint id;
	bool success = ictxIDpool.CreateID(id);
	assert(success && "failed to create a context ID");
	if (success && outCtxID.isValid()) {
		VkRScontext vkrsctx;
		ictxMap[id] = vkrsctx;
		outCtxID.id = id;

		if (irenderOnscreen) {
			vkrsctx.window = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
		}
		createSurface(vkrsctx);
		setPhysicalDevice(iinstance, vkrsctx);
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;

}

RSresult VkRenderSystem::contextDispose(const RScontextID& ctxID) {

	if (ctxID.isValid() && ictxMap.find(ctxID.id) != ictxMap.end()) {
		const VkRScontext& ctx = ictxMap[ctxID.id];

		if (irenderOnscreen && ctx.window != nullptr) {
			glfwDestroyWindow(ctx.window);
			ictxMap.erase(ctxID.id);
		}

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewCreate(RSviewID& viewID, const RSview& view)
{
	RSuint id;
	bool success = iviewIDpool.CreateID(id);
	assert(success && "failed to create a view ID");
	if (success && viewID.isValid()) {
		VkRSview vkrsview;
		vkrsview.view = view;
		iviewMap[id] = vkrsview;
		viewID.id = id;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewUpdate(const RSviewID& viewID, const RSview& view)
{
	if (viewID.isValid()) {
		if (iviewMap.find(viewID.id) != iviewMap.end()) {
			iviewMap[viewID.id].view = view;
			iviewMap[viewID.id].view.dirty = true;

			return RSresult::SUCCESS;
		}
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	return RSresult::SUCCESS;
}
