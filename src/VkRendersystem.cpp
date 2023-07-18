#include "VkRenderSystem.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <algorithm>

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

	std::vector<const char*> deviceExtensions = iinstance.deviceExtensions;
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

void VkRenderSystem::setPhysicalDevice(VkRScontext& ctx) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(iinstance.instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(iinstance.instance, &deviceCount, devices.data());
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, ctx)) {
			iinstance.physicalDevice = device;
			printPhysicalDeviceInfo(device);
			break;
		}
	}

	if (iinstance.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable card");
	}
}

void VkRenderSystem::createLogicalDevice(VkRScontext& ctx) {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	VkRSqueueFamilyIndices indices = findQueueFamilies(iinstance.physicalDevice, ctx);
	std::set<uint32_t> uniqueQueueFamiles = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.f;
	for (uint32_t queueFamily : uniqueQueueFamiles) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}


	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	VkPhysicalDeviceFeatures deviceFeatures{};
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(iinstance.deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = iinstance.deviceExtensions.data();

	if (iinstance.enableValidation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(iinstance.validationLayers.size());
		createInfo.ppEnabledLayerNames = iinstance.validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	const VkResult result = vkCreateDevice(iinstance.physicalDevice, &createInfo, nullptr, &ctx.device);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device");
	}

	//get the graphics queue handle from the logical device.
	vkGetDeviceQueue(ctx.device, indices.graphicsFamily.value(), 0, &ctx.graphicsQueue);

	//get the present queue handle from the logical device
	vkGetDeviceQueue(ctx.device, indices.presentFamily.value(), 0, &ctx.presentQueue);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

/**
* VK_PRESENT_MODE_IMMEDIATE_KHR : images submitted by the app is transferred to the screen immediately (may result in tearing)
* VK_PRESENT_MODE_FIFO_KHR: vertical sync mode - if the queue is full, the app needs to wait before submitting a new image
* VK_PRESENT_MODE_FIFO_RELAXED_KHR - if the queue is empty, then instead of waiting for the next vertical blank,
*                                 the image is transferred right away.
* VK_PRESENT_MODE_MAILBOX_KHR - if the queue is full, instead of blocking the app from submitting a new image to the queue, the
*								newest image is replaced with "newer" images. AKA triple buffering
*
* VK_PRESENT_MODE_FIFO_KHR  - is guaranteed to be available.
*/
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* wnd) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(wnd, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VkRenderSystem::createSwapChain(VkRScontext& ctx) {
	VkRSswapChainSupportDetails swapChainSupport = querySwapChainSupport(iinstance.physicalDevice, ctx);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, ctx.window);

	//TODO: perhaps use std::clamp here
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = ctx.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.presentMode = presentMode;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; //this is to specify if you're using a stereoscropic 3D application or not.
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //this means that swap chain is treated like a bunch of attachments. if we want to post process, then use VK_IMAGE_USAGE_TRANSFER_DST_BIT  and perform a memory operation to transfer the rendered image to a swap chain image.

	VkRSqueueFamilyIndices indices = findQueueFamilies(iinstance.physicalDevice, ctx);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		//images can be used across multiple queue families without explicit ownership transfers
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		//image is owned by one queue family at a time and ownership must be explicity transferred to another queue.
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	const VkResult result = vkCreateSwapchainKHR(ctx.device, &createInfo, nullptr, &ctx.swapChain);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(ctx.device, ctx.swapChain, &imageCount, nullptr);
	ctx.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(ctx.device, ctx.swapChain, &imageCount, ctx.swapChainImages.data());

	ctx.swapChainImageFormat = surfaceFormat.format;
	ctx.swapChainExtent = extent;
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

void VkRenderSystem::createImageViews(VkRScontext& ctx) {
	ctx.swapChainImageViews.resize(ctx.swapChainImages.size());

	for (size_t i = 0; i < ctx.swapChainImages.size(); ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = ctx.swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = ctx.swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		const VkResult result = vkCreateImageView(ctx.device, &createInfo, nullptr, &ctx.swapChainImageViews[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
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
		setPhysicalDevice(vkrsctx);
		createLogicalDevice(vkrsctx);
		createSwapChain(vkrsctx);
		createImageViews(vkrsctx);
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
