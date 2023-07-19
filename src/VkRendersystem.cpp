#include "VkRenderSystem.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>

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

void VkRenderSystem::createFramebuffers(VkRSview& view, const VkRScontext& ctx, const VkRenderPass& renderPass) {
	view.swapChainFramebuffers.resize(ctx.swapChainImageViews.size());

	for (size_t i = 0; i < ctx.swapChainImageViews.size(); ++i) {
		VkImageView attachments[] = { ctx.swapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = ctx.swapChainExtent.width;
		framebufferInfo.height = ctx.swapChainExtent.height;
		framebufferInfo.layers = 1;

		const VkResult fboresult = vkCreateFramebuffer(iinstance.device, &framebufferInfo, nullptr, &view.swapChainFramebuffers[i]);
		if (fboresult != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer");
		}
	}
}

void VkRenderSystem::contextDrawCollection(const VkRScontext& ctx, VkRSview& view, const VkRScollection& collection) {
	const VkDevice& device = iinstance.device;
	uint32_t currentFrame = view.currentFrame;
	VkFence inflightFence = collection.inFlightFences[currentFrame];
	VkSemaphore imageAvailableSemaphore = collection.imageAvailableSemaphores[currentFrame];
	VkSemaphore renderFinishedSemaphore = collection.renderFinishedSemaphores[currentFrame];
	VkCommandBuffer commandBuffer = collection.commandBuffers[currentFrame];

	vkWaitForFences(device, 1, &inflightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inflightFence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, ctx.swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	vkResetCommandBuffer(commandBuffer, 0);

	recordCommandBuffer(collection, view, ctx, imageIndex, currentFrame);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	const VkResult submitRes = vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, inflightFence);
	if (submitRes != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { ctx.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	const VkResult res = vkQueuePresentKHR(ctx.presentQueue, &presentInfo);

	view.currentFrame = (currentFrame + 1) % VkRScontext::MAX_FRAMES_IN_FLIGHT;
}

RSresult VkRenderSystem::contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID) {
	if (iinitInfo.onScreenCanvas) {
		if (contextAvailable(ctxID) && viewAvailable(viewID)) {
			const VkRScontext& ctx = ictxMap[ctxID.id];
			VkRSview& view = iviewMap[viewID.id];
			for (const uint32_t colID : view.view.collectionList) {
				if (collectionAvailable(RScollectionID(colID))) {
					VkRScollection& coll = icollectionMap[colID];
					if (coll.renderPass == nullptr) {
						createRenderpass(coll, ctx);
					}
					if (view.swapChainFramebuffers.empty()) {
						createFramebuffers(view, ctx, coll.renderPass); //hacky renderpass.
					}
					if (coll.commandPool == nullptr) {
						createCommandPool(coll, ctx);
					}
					if (coll.commandBuffers.empty()) {
						createCommandBuffers(coll, ctx);
					}
					if (coll.inFlightFences.empty()) {
						createSyncObjects(coll, ctx);
					}
				}
			}


			while (!glfwWindowShouldClose(ctx.window)) {
				glfwPollEvents();
				RScollectionID collID0(view.view.collectionList[0]);
				const VkRScollection& coll = icollectionMap[collID0.id];
				contextDrawCollection(ctx, view, coll);
			}
			vkDeviceWaitIdle(iinstance.device);
		}
	}

	return RSresult::SUCCESS;
}

VkRSqueueFamilyIndices VkRenderSystem::findQueueFamilies(const VkPhysicalDevice device, const VkRScontext& ctx) {
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

	const VkResult result = vkCreateDevice(iinstance.physicalDevice, &createInfo, nullptr, &iinstance.device);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device");
	}

	//get the graphics queue handle from the logical device.
	vkGetDeviceQueue(iinstance.device, indices.graphicsFamily.value(), 0, &ctx.graphicsQueue);

	//get the present queue handle from the logical device
	vkGetDeviceQueue(iinstance.device, indices.presentFamily.value(), 0, &ctx.presentQueue);
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

	const VkDevice& device = iinstance.device;
	const VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &ctx.swapChain);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(device, ctx.swapChain, &imageCount, nullptr);
	ctx.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, ctx.swapChain, &imageCount, ctx.swapChainImages.data());

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
	vkDestroyDevice(iinstance.device, nullptr);
	if (iinstance.enableValidation) {
		DestroyDebugUtilsMessengerEXT(iinstance.instance, iinstance.debugMessenger, nullptr);
	}
	vkDestroyInstance(iinstance.instance, nullptr);

	glfwTerminate();

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

		const VkResult result = vkCreateImageView(iinstance.device, &createInfo, nullptr, &ctx.swapChainImageViews[i]);
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

bool VkRenderSystem::contextAvailable(const RScontextID& ctxID) const {
	return ctxID.isValid() && ictxMap.find(ctxID.id) != ictxMap.end();
}

void VkRenderSystem::disposeContext(VkRScontext& ctx) {
	
	const VkDevice& device = iinstance.device;
	for (const auto& imageView : ctx.swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, ctx.swapChain, nullptr);
	vkDestroySurfaceKHR(iinstance.instance, ctx.surface, nullptr);

	glfwDestroyWindow(ctx.window);
	ctx.window = nullptr;
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

bool VkRenderSystem::viewAvailable(const RSviewID& viewID) const {
	return viewID.isValid() && iviewMap.find(viewID.id) != iviewMap.end();
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

//RSresult VkRenderSystem::viewUpdate(const RSviewID& viewID, const RSview& view)
//{
//	if (viewID.isValid()) {
//		if (iviewMap.find(viewID.id) != iviewMap.end()) {
//			iviewMap[viewID.id].view = view;
//			iviewMap[viewID.id].view.dirty = true;
//
//			return RSresult::SUCCESS;
//		}
//	}
//
//	return RSresult::FAILURE;
//}

RSresult VkRenderSystem::viewAddCollection(const RSviewID& viewID, const RScollectionID& colID) {
	if (viewID.isValid() && iviewMap.find(viewID.id) != iviewMap.end()) {
		VkRSview& vkrsview = iviewMap[viewID.id];
		//for now support only one collection
		if (vkrsview.view.collectionList.empty()) {
			vkrsview.view.collectionList.push_back(colID.id);
		}
		vkrsview.view.dirty = true;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewRemoveCollection(const RSviewID& viewID, const RScollectionID& colID) {
	if (viewID.isValid() && iviewMap.find(viewID.id) != iviewMap.end()) {
		VkRSview& vkrsview = iviewMap[viewID.id];
		vkrsview.view.collectionList.erase(std::remove(vkrsview.view.collectionList.begin(), vkrsview.view.collectionList.end(), colID.id));
		vkrsview.view.dirty = true;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewFinalize(const RSviewID& viewID) {
	if (viewID.isValid() && iviewMap.find(viewID.id) != iviewMap.end()) {
		VkRSview& vkrsview = iviewMap[viewID.id];
		vkrsview.view.dirty = false;
		
		//finalize the view

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	if (viewID.isValid() && iviewMap.find(viewID.id) != iviewMap.end()) {
		VkRSview& vkrsview = iviewMap[viewID.id];
		//dispose view contents
		disposeView(vkrsview);
		iviewMap.erase(viewID.id);
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

void VkRenderSystem::disposeView(VkRSview& view) {
	
	for (auto framebuffer : view.swapChainFramebuffers) {
		vkDestroyFramebuffer(iinstance.device, framebuffer, nullptr);
	}
}

void VkRenderSystem::createRenderpass(VkRScollection& collection, const VkRScontext& ctx) {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = ctx.swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	const VkResult result = vkCreateRenderPass(iinstance.device, &renderPassInfo, nullptr, &collection.renderPass);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}


bool VkRenderSystem::collectionAvailable(const RScollectionID& colID) {
	return colID.isValid() && icollectionMap.find(colID.id) != icollectionMap.end();
}

RSresult VkRenderSystem::collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo) {
	RSuint id;
	bool success = icollIDpool.CreateID(id);
	assert(success && "failed to create a collection");
	if (success && colID.isValid()) {
		VkRScollection vkrscol;
		vkrscol.info = collInfo;
		icollectionMap[colID.id] = vkrscol;
		colID.id = id;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;

}

std::vector<char> VkRenderSystem::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();

	return buffer;
}

VkShaderModule VkRenderSystem::createShaderModule(const std::vector<char>& code, const VkDevice& device) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	const VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create a shader module!");
	}

	return shaderModule;
}

void VkRenderSystem::createGraphicsPipeline(VkRScollection& collection, const VkRScontext& ctx) {
	auto vertShaderCode = readFile("./src/shaders/spv/sampleshaderVert.spv");
	auto fragShaderCode = readFile("./src/shaders/spv/sampleshaderFrag.spv");
	
	const VkDevice& device = iinstance.device;
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo , fragShaderStageInfo };

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)ctx.swapChainExtent.width;
	viewport.height = (float)ctx.swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = ctx.swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; //optional
	rasterizer.depthBiasClamp = 0.0f; //optional
	rasterizer.depthBiasSlopeFactor = 0.0f; //optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	const VkResult pipelineLayoutResult = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &collection.pipelineLayout);
	if (pipelineLayoutResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = collection.pipelineLayout;

	pipelineInfo.renderPass = collection.renderPass; //hack until we find renderpasses are hierarchical.
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	const VkResult graphicsPipelineResult = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &collection.graphicsPipeline);
	if (graphicsPipelineResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void VkRenderSystem::recordCommandBuffer(const VkRScollection& collection, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	const VkCommandBuffer commandBuffer = collection.commandBuffers[currentFrame];
	const VkResult beginRes = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (beginRes != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffers!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = collection.renderPass;
	renderPassInfo.framebuffer = view.swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = ctx.swapChainExtent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, collection.graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ctx.swapChainExtent.width);
		viewport.height = static_cast<float>(ctx.swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = ctx.swapChainExtent;
		vkCmdSetScissor(collection.commandBuffers[currentFrame], 0, 1, &scissor);

		vkCmdDraw(collection.commandBuffers[currentFrame], 3, 1, 0, 0);
	}
	vkCmdEndRenderPass(collection.commandBuffers[currentFrame]);

	const VkResult endCmdBuffRes = vkEndCommandBuffer(commandBuffer);
	if (endCmdBuffRes != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer");
	}
}

void VkRenderSystem::createCommandPool(VkRScollection& collection, const VkRScontext& ctx) {
	VkRSqueueFamilyIndices queueFamilyIndices = findQueueFamilies(iinstance.physicalDevice, ctx);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	const VkResult cmdpoolResult = vkCreateCommandPool(iinstance.device, &poolInfo, nullptr, &collection.commandPool);
	if (cmdpoolResult != VK_SUCCESS) {
		throw std::runtime_error("faailed to create command pool");
	}
}

void VkRenderSystem::createCommandBuffers(VkRScollection& collection, const VkRScontext& ctx) {
	collection.commandBuffers.resize(ctx.MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = collection.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)collection.commandBuffers.size();

	const VkResult allocCmdBufferRes = vkAllocateCommandBuffers(iinstance.device, &allocInfo, collection.commandBuffers.data());
	if (allocCmdBufferRes != VK_SUCCESS) {
		throw std::runtime_error("failed to allocated command buffers");
	}
}

void VkRenderSystem::createSyncObjects(VkRScollection& collection, const VkRScontext& ctx) {
	collection.imageAvailableSemaphores.resize(ctx.MAX_FRAMES_IN_FLIGHT);
	collection.renderFinishedSemaphores.resize(ctx.MAX_FRAMES_IN_FLIGHT);
	collection.inFlightFences.resize(ctx.MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	const VkDevice& device = iinstance.device;
	for (size_t i = 0; i < ctx.MAX_FRAMES_IN_FLIGHT; ++i) {
		VkResult imgAvailSemaphoreRes = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &collection.imageAvailableSemaphores[i]);
		VkResult renderFinishedSemaphoreRes = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &collection.renderFinishedSemaphores[i]);
		VkResult fenceRes = vkCreateFence(device, &fenceInfo, nullptr, &collection.inFlightFences[i]);

		if (imgAvailSemaphoreRes != VK_SUCCESS || renderFinishedSemaphoreRes != VK_SUCCESS || fenceRes != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores");
		}
	}
}

RSresult VkRenderSystem::collectionFinalize(const RScollectionID& colID, const RScontextID& ctxID) {
	if (collectionAvailable(colID) && contextAvailable(ctxID)) {
		VkRScollection& collection = icollectionMap[colID.id];
		const VkRScontext& ctx = ictxMap[ctxID.id];
		//finalize the collection
		if (collection.dirty) {
			createGraphicsPipeline(collection, ctx);
			createCommandPool(collection, ctx);
			createCommandBuffers(collection, ctx);
			createSyncObjects(collection, ctx);

			collection.dirty = false;
		}
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::collectionDispose(const RScollectionID& colID) {
	if (colID.isValid() && icollectionMap.find(colID.id) != icollectionMap.end()) {
		VkRScollection collection = icollectionMap[colID.id];
		//dispose the collection

		icollectionMap.erase(colID.id);
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

void VkRenderSystem::disposeCollection(VkRScollection& collection) {
	const VkDevice& device = iinstance.device;
	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(device, collection.imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, collection.renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, collection.inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, collection.commandPool, nullptr);

	vkDestroyPipeline(device, collection.graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, collection.pipelineLayout, nullptr);
	vkDestroyRenderPass(device, collection.renderPass, nullptr);
}