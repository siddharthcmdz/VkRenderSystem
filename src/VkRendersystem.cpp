#include "VkRenderSystem.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>

#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include "RSVkDebugUtils.h"
#include "VertexData.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	uint32_t version;
	vkEnumerateInstanceVersion(&version);
	inst.majorVersion = VK_API_VERSION_MAJOR(version);
	inst.minorVersion = VK_API_VERSION_MINOR(version);
	inst.patchVersion = VK_API_VERSION_PATCH(version);
	inst.variantVersion = VK_API_VERSION_VARIANT(version);
	std::cout << "Vulkan version: " << inst.majorVersion << " (major) " << inst.minorVersion << " (minor)" << inst.patchVersion << " (patch) " << inst.variantVersion << " (variant)" << std::endl;
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
			debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
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

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
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

void VkRenderSystem::cleanupSwapChain(VkRScontext& ctx, VkRSview& view) {
	for (auto framebuffer : view.swapChainFramebuffers) {
		vkDestroyFramebuffer(iinstance.device, framebuffer, nullptr);
	}

	for (const auto& imageView : ctx.swapChainImageViews) {
		vkDestroyImageView(iinstance.device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(iinstance.device, ctx.swapChain, nullptr);
}

void VkRenderSystem::recreateSwapchain(VkRScontext& ctx, VkRSview& view, const VkRenderPass& renderpass) {
	int width = 0, height = 0;
	glfwGetFramebufferSize(ctx.window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(ctx.window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(iinstance.device);

	cleanupSwapChain(ctx, view);

	createSwapChain(ctx);
	createImageViews(ctx);
	createFramebuffers(view, ctx, renderpass);
}

void VkRenderSystem::contextDrawCollection(VkRScontext& ctx, VkRSview& view, const VkRScollection& collection) {
	const VkDevice& device = iinstance.device;
	uint32_t currentFrame = view.currentFrame;
	const VkFence inflightFence = collection.inFlightFences[currentFrame];
	const VkSemaphore imageAvailableSemaphore = collection.imageAvailableSemaphores[currentFrame];
	const VkSemaphore renderFinishedSemaphore = collection.renderFinishedSemaphores[currentFrame];
	const VkCommandBuffer commandBuffer = collection.commandBuffers[currentFrame];

	vkWaitForFences(device, 1, &inflightFence, VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult res = vkAcquireNextImageKHR(device, ctx.swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain(ctx, view, collection.renderPass);
		return;
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	//only reset the fence if we are submitting work.
	vkResetFences(device, 1, &inflightFence);
	
	updateUniformBuffer(view, ctx, currentFrame);
	
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

	const VkResult submitRes = vkQueueSubmit(iinstance.graphicsQueue, 1, &submitInfo, inflightFence);
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

	res = vkQueuePresentKHR(iinstance.presentQueue, &presentInfo);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || ctx.framebufferResized) {
		ctx.framebufferResized = false;
		recreateSwapchain(ctx, view, collection.renderPass);
	}
	else if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	view.currentFrame = (currentFrame + 1) % VkRScontext::MAX_FRAMES_IN_FLIGHT;
}

RSresult VkRenderSystem::contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID) {
	if (iinitInfo.onScreenCanvas) {
		if (contextAvailable(ctxID) && viewAvailable(viewID)) {
			VkRScontext& ctx = ictxMap[ctxID.id];
			VkRSview& view = iviewMap[viewID];
			for (const uint32_t colID : view.view.collectionList) {
				if (collectionAvailable(RScollectionID(colID))) {
					VkRScollection& coll = icollectionMap[colID];
					if (view.swapChainFramebuffers.empty()) {
						createFramebuffers(view, ctx, coll.renderPass); //hacky renderpass. need to move this to viewCreate(..)
					}
				}
			}


			while (!glfwWindowShouldClose(ctx.window)) {
				glfwPollEvents();
				RScollectionID collID0(view.view.collectionList[0]);
				const VkRScollection& coll = icollectionMap[collID0];
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
	vkGetDeviceQueue(iinstance.device, indices.graphicsFamily.value(), 0, &iinstance.graphicsQueue);

	//get the present queue handle from the logical device
	vkGetDeviceQueue(iinstance.device, indices.presentFamily.value(), 0, &iinstance.presentQueue);
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
	iinitInfo = info;
	std::vector<const char*> extensions;
	if (info.onScreenCanvas) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	}
	createInstance(info);
	setupDebugMessenger();

	iisRSinited = true;
	
	return RSresult::SUCCESS;
}

bool VkRenderSystem::isRenderSystemInit()
{
	return iisRSinited;
}

RSresult VkRenderSystem::renderSystemDispose()
{
	vkDestroyCommandPool(iinstance.device, iinstance.commandPool, nullptr);
	vkDestroyDevice(iinstance.device, nullptr);
	if (iinstance.enableValidation) {
		DestroyDebugUtilsMessengerEXT(iinstance.instance, iinstance.debugMessenger, nullptr);
	}
	vkDestroyInstance(iinstance.instance, nullptr);
	
	if (iinitInfo.onScreenCanvas) {
		glfwTerminate();
	}

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

void VkRenderSystem::contextResizeSet(const RScontextID& ctxID, bool onOff) {
	if (contextAvailable(ctxID)) {
		VkRScontext& ctx = ictxMap[ctxID.id];
		ctx.framebufferResized = onOff;
	}
}

void VkRenderSystem::framebufferResizeCallback(GLFWwindow* window, int widht, int height) {
	uint32_t id = *(reinterpret_cast<uint32_t*>(glfwGetWindowUserPointer(window)));
	RScontextID ctxID(id);
	if (getInstance().contextAvailable(ctxID)) {
		getInstance().contextResizeSet(ctxID, true);
	}
}

RSresult VkRenderSystem::contextCreate(RScontextID& outCtxID, const RScontextInfo& info) {

	RSuint id;
	bool success = ictxIDpool.CreateID(id);
	assert(success && "failed to create a context ID");
	if (success) {
		VkRScontext vkrsctx;

		if (iinitInfo.onScreenCanvas) {
			vkrsctx.window = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
			glfwSetWindowUserPointer(vkrsctx.window, &id);
			glfwSetFramebufferSizeCallback(vkrsctx.window, framebufferResizeCallback);
		}
		createSurface(vkrsctx);
		setPhysicalDevice(vkrsctx);
		createLogicalDevice(vkrsctx);
		createSwapChain(vkrsctx);
		createImageViews(vkrsctx);
		createCommandPool(vkrsctx);
		outCtxID.id = id;
		ictxMap[outCtxID] = vkrsctx;
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
	ctx.swapChainImageViews.clear();
	vkDestroySwapchainKHR(device, ctx.swapChain, nullptr);
	ctx.swapChain = nullptr;
	vkDestroySurfaceKHR(iinstance.instance, ctx.surface, nullptr);
	ctx.surface = nullptr;

	glfwDestroyWindow(ctx.window);
	ctx.window = nullptr;
}

RSresult VkRenderSystem::contextDispose(const RScontextID& ctxID) {

	if (contextAvailable(ctxID)) {
		VkRScontext& ctx = ictxMap[ctxID.id];

		if (iinitInfo.onScreenCanvas && ctx.window != nullptr) {
			disposeContext(ctx);
			ictxMap.erase(ctxID.id);
		}

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

void VkRenderSystem::createDescriptorSetLayout(VkRSview& view) {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;


	VkResult res = vkCreateDescriptorSetLayout(iinstance.device, &layoutInfo, nullptr, &view.descriptorSetLayout);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor set layout");
	}
}

void VkRenderSystem::createDescriptorPool(VkRSview& view) {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);

	VkResult res = vkCreateDescriptorPool(iinstance.device, &poolInfo, nullptr, &view.descriptorPool);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool");
	}
}

void VkRenderSystem::createDescriptorSet(VkRSview& view) {
	std::vector<VkDescriptorSetLayout> layouts(VkRScontext::MAX_FRAMES_IN_FLIGHT, view.descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = view.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	view.descriptorSets.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	VkResult res = vkAllocateDescriptorSets(iinstance.device, &allocInfo, view.descriptorSets.data());
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor sets");
	}

	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = view.uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(VkRSviewDescriptor);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = view.descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(iinstance.device, 1, &descriptorWrite, 0, nullptr);
	}
}

void VkRenderSystem::createUniformBuffers(VkRSview& view) {
	VkDeviceSize buffersize = sizeof(VkRSviewDescriptor);

	view.uniformBuffers.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	view.uniformBuffersMemory.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	view.uniformBuffersMapped.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);


	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) {
		createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, view.uniformBuffers[i], view.uniformBuffersMemory[i]);
		vkMapMemory(iinstance.device, view.uniformBuffersMemory[i], 0, buffersize, 0, &view.uniformBuffersMapped[i]);
	}
}

void VkRenderSystem::updateUniformBuffer(VkRSview& view, VkRScontext& ctx, uint32_t currentFrame) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	VkRSviewDescriptor ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), ((float)ctx.swapChainExtent.width / (float)ctx.swapChainExtent.height), 0.0f, 1.0f);
	ubo.proj[1][1] *= -1;

	memcpy(view.uniformBuffersMapped[currentFrame], &ubo, sizeof(VkRSviewDescriptor));
}

bool VkRenderSystem::viewAvailable(const RSviewID& viewID) const {
	return viewID.isValid() && iviewMap.find(viewID) != iviewMap.end();
}

RSresult VkRenderSystem::viewCreate(RSviewID& viewID, const RSview& view)
{
	RSuint id;
	bool success = iviewIDpool.CreateID(id);
	assert(success && "failed to create a view ID");
	if (success) {
		VkRSview vkrsview;
		vkrsview.view = view;
		createDescriptorSetLayout(vkrsview);
		createUniformBuffers(vkrsview);
		createDescriptorPool(vkrsview);
		createDescriptorSet(vkrsview);

		viewID.id = id;
		iviewMap[viewID] = vkrsview;
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
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
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
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		vkrsview.view.collectionList.erase(std::remove(vkrsview.view.collectionList.begin(), vkrsview.view.collectionList.end(), colID.id));
		vkrsview.view.dirty = true;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewFinalize(const RSviewID& viewID) {
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		vkrsview.view.dirty = false;
		
		//finalize the view

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		//dispose view contents
		disposeView(vkrsview);
		iviewMap.erase(viewID);
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

void VkRenderSystem::disposeView(VkRSview& view) {
	
	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(iinstance.device, view.uniformBuffers[i], nullptr);
		vkFreeMemory(iinstance.device, view.uniformBuffersMemory[i], nullptr);
	}
	vkDestroyDescriptorPool(iinstance.device, view.descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(iinstance.device, view.descriptorSetLayout, nullptr);

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
	return colID.isValid() && (icollectionMap.find(colID) != icollectionMap.end());
}

RSresult VkRenderSystem::collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo) {
	RSuint id;
	bool success = icollIDpool.CreateID(id);
	assert(success && "failed to create a collection");
	if (success) {
		VkRScollection vkrscol;
		vkrscol.info = collInfo;
		colID.id = id;
		icollectionMap[colID] = vkrscol;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;

}

bool VkRenderSystem::collectionInstanceAvailable(const RScollectionID& collID, const RSinstanceID& instanceID) {
	if (collectionAvailable(collID)) {
		const VkRScollection& coll = icollectionMap[collID];
		if (instanceID.isValid() && coll.instanceMap.find(instanceID) != coll.instanceMap.end()) {
			return true;
		}
	}

	return false;
}

RSresult VkRenderSystem::collectionInstanceCreate(RScollectionID& collID, RSinstanceID& outInstID, const RSinstanceInfo& instInfo) {
	if (collectionAvailable(collID)) {

		uint32_t id;
		iinstanceIDpool.CreateID(id);
		outInstID.id = id;
		VkRScollection& coll = icollectionMap[collID];

		VkRSinstanceData inst;
		inst.instInfo = instInfo;

		coll.instanceMap[outInstID] = inst;

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::collectionInstanceDispose(RScollectionID& collID, RSinstanceID& instID) {
	if (collectionInstanceAvailable(collID, instID)) {
		VkRScollection& coll = icollectionMap[collID];
		coll.instanceMap.erase(instID);
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

VkShaderModule VkRenderSystem::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	const VkResult result = vkCreateShaderModule(iinstance.device, &createInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create a shader module!");
	}

	return shaderModule;
}

void VkRenderSystem::createGraphicsPipeline(VkRScollection& collection, const VkRScontext& ctx, const VkRSview& view, VkRSdrawCommand& drawcmd) {
	auto vertShaderCode = readFile("./src/shaders/spv/passthroughVert.spv");
	auto fragShaderCode = readFile("./src/shaders/spv/passthroughFrag.spv");
	
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

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

	auto bindingDescription = getBindingDescription();
	auto attributeDescriptions = getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = drawcmd.primTopology;
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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &view.descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	const VkResult pipelineLayoutResult = vkCreatePipelineLayout(iinstance.device, &pipelineLayoutInfo, nullptr, &drawcmd.pipelineLayout);
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

	pipelineInfo.layout = drawcmd.pipelineLayout;

	pipelineInfo.renderPass = collection.renderPass; //hack until we find renderpasses are hierarchical.
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	const VkResult graphicsPipelineResult = vkCreateGraphicsPipelines(iinstance.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &drawcmd.graphicsPipeline);
	if (graphicsPipelineResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(iinstance.device, vertShaderModule, nullptr);
	vkDestroyShaderModule(iinstance.device, fragShaderModule, nullptr);
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

	VkClearValue clearColor = { {{view.view.clearColor.r, view.view.clearColor.r, view.view.clearColor.r, view.view.clearColor.r}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
	{
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
		
		for (const VkRSdrawCommand& drawcmd : collection.drawCommands) {
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawcmd.graphicsPipeline);

			VkBuffer vertexBuffers[]{ drawcmd.vertexBuffer };
			VkDeviceSize offsets[]{ drawcmd.vertexOffset };
			vkCmdBindVertexBuffers(collection.commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);
			if (drawcmd.isIndexed) {
				vkCmdBindIndexBuffer(collection.commandBuffers[currentFrame], drawcmd.indicesBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
			}
			vkCmdBindDescriptorSets(collection.commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, drawcmd.pipelineLayout, 0, 1, &view.descriptorSets[currentFrame], 0, nullptr);

			if (drawcmd.isIndexed) {
				
				vkCmdDrawIndexed(collection.commandBuffers[currentFrame], drawcmd.numIndices, 1, 0, 0, 0);
			}
			else {
				vkCmdDraw(collection.commandBuffers[currentFrame], drawcmd.numVertices, 1, 0, 0);
			}
		}
	}
	vkCmdEndRenderPass(collection.commandBuffers[currentFrame]);

	const VkResult endCmdBuffRes = vkEndCommandBuffer(commandBuffer);
	if (endCmdBuffRes != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer");
	}
}

void VkRenderSystem::createCommandPool(const VkRScontext& ctx) {
	VkRSqueueFamilyIndices queueFamilyIndices = findQueueFamilies(iinstance.physicalDevice, ctx);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	const VkResult cmdpoolResult = vkCreateCommandPool(iinstance.device, &poolInfo, nullptr, &iinstance.commandPool);
	if (cmdpoolResult != VK_SUCCESS) {
		throw std::runtime_error("faailed to create command pool");
	}
}

void VkRenderSystem::createCommandBuffers(VkRScollection& collection, const VkRScontext& ctx) {
	collection.commandBuffers.resize(ctx.MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = iinstance.commandPool;
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

VkPrimitiveTopology getPrimitiveType(const RSprimitiveType& ptype) {
	switch (ptype) {
	case RSprimitiveType::ptLine:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	case RSprimitiveType::ptLineStrip:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

	case RSprimitiveType::ptPoint:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	case RSprimitiveType::ptTriangle:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	case RSprimitiveType::ptTriangleStrip:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	}

	return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

RSresult VkRenderSystem::collectionFinalize(const RScollectionID& colID, const RScontextID& ctxID, const RSviewID& viewID) {
	if (collectionAvailable(colID) && contextAvailable(ctxID) && viewAvailable(viewID)) {
		VkRScollection& collection = icollectionMap[colID];
		const VkRScontext& ctx = ictxMap[ctxID];
		const VkRSview& view = iviewMap[viewID];
		//finalize the collection
		if (collection.dirty) {
			createRenderpass(collection, ctx);
			createCommandBuffers(collection, ctx);
			createSyncObjects(collection, ctx);

			//create drawcommands
			for (auto& iter : collection.instanceMap) {
				const VkRSinstanceData& inst = iter.second;
				const RSgeometryDataID& gdataID = inst.instInfo.gdataID;
				const RSgeometryID& geomID = inst.instInfo.geomID;
				if (geometryDataAvailable(gdataID) && geometryAvailable(geomID)) {
					const VkRSgeometryData& gdata = igeometryDataMap[gdataID];
					const VkRSgeometry& geom = igeometryMap[geomID];
					VkRSdrawCommand cmd;
					cmd.indicesBuffer = gdata.indicesBuffer;
					cmd.isIndexed = gdata.indicesBuffer != VK_NULL_HANDLE;
					cmd.numIndices = gdata.numIndices;
					cmd.numVertices = gdata.numVertices;
					cmd.vertexBuffer = gdata.vaBuffer;
					cmd.primTopology = getPrimitiveType(geom.geomInfo.primType);
					createGraphicsPipeline(collection, ctx, view, cmd);

					collection.drawCommands.push_back(cmd);
				}
			}
			collection.dirty = false;
		}
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::collectionDispose(const RScollectionID& colID) {
	if (colID.isValid() && icollectionMap.find(colID) != icollectionMap.end()) {
		VkRScollection& collection = icollectionMap[colID];
		//dispose the collection
		disposeCollection(collection);
		icollectionMap.erase(colID);
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


	for (size_t i = 0; i < collection.drawCommands.size(); i++) {
		VkRSdrawCommand& drawcmd = collection.drawCommands[i];
		vkDestroyPipeline(device, drawcmd.graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, drawcmd.pipelineLayout, nullptr);
	}
	vkDestroyRenderPass(device, collection.renderPass, nullptr);
}

uint32_t VkRenderSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(iinstance.physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VkRenderSystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = 0;

	VkResult res = vkCreateBuffer(iinstance.device, &bufferInfo, nullptr, &buffer);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create a buffer!");
	}

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(iinstance.device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memProperties);

	res = vkAllocateMemory(iinstance.device, &allocInfo, nullptr, &bufferMemory);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory");
	}

	res = vkBindBufferMemory(iinstance.device, buffer, bufferMemory, 0);
}

bool VkRenderSystem::geometryDataAvailable(const RSgeometryDataID& geomDataID) {
	return geomDataID.isValid() && (igeometryDataMap.find(geomDataID) != igeometryDataMap.end());
}

void VkRenderSystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = iinstance.commandPool; 
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(iinstance.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	{
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(iinstance.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(iinstance.graphicsQueue);

	vkFreeCommandBuffers(iinstance.device, iinstance.commandPool, 1, &commandBuffer);
}

/**
* vertex input binding description describes what rate to load data from memory throughout the vertices.
*/
VkVertexInputBindingDescription VkRenderSystem::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(rsvd::Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> VkRenderSystem::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(rsvd::Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(rsvd::Vertex, color);

	return attributeDescriptions;
}

RSresult VkRenderSystem::geometryDataCreate(RSgeometryDataID& outgdataID, uint32_t numVertices, uint32_t numIndices, const RSvertexAttribsInfo attributesInfo, RSbufferUsageHints usageHints) {
	RSuint id;
	bool success = igeomDataIDpool.CreateID(id);
	assert(success && "failed to create a geometry data ID");
	if (success) {
		VkRSgeometryData gdata;
		gdata.numVertices = numVertices;
		gdata.numIndices = numIndices;
		gdata.usageHints = usageHints;
		gdata.attributesInfo = attributesInfo;

		//Create buffer for position
		bool foundPosition = false;
		for (uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) {
			if (gdata.attributesInfo.attributes[i] == RSvertexAttribute::vaPosition) {
				foundPosition = true;
				break;
			}
		}
		if (foundPosition == false) {
			return RSresult::FAILURE;
		}

		
		//create buffer for staging position vertex attribs
		VkDeviceSize vaBufferSize = numVertices * attributesInfo.sizeOfAttrib();
		createBuffer(vaBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gdata.stagingVABuffer, gdata.stagingVAbufferMemory);
		vkMapMemory(iinstance.device, gdata.stagingVAbufferMemory, 0, vaBufferSize, 0, &gdata.mappedStagingVAPtr);

		//create buffer for final vertex attributes buffer
		createBuffer(vaBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gdata.vaBuffer, gdata.vaBufferMemory);

		//create buffer for indices
		if (numIndices) {
			VkDeviceSize indexBufferSize = numIndices * sizeof(uint32_t);
			createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gdata.stagingIndexBuffer, gdata.stagingIndexBufferMemory);
			vkMapMemory(iinstance.device, gdata.stagingIndexBufferMemory, 0, indexBufferSize, 0, &gdata.mappedIndexPtr);
		
			createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gdata.indicesBuffer, gdata.indicesBufferMemory);
		}

		outgdataID.id = id;
		igeometryDataMap[outgdataID] = gdata;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeinBytes, void* data) {

	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];
		if (offset != 0) {
			throw std::runtime_error("Unsupported operation!");
		}
		
		assert(gdata.usageHints == RSbufferUsageHints::buVertices && "invalid buffer usage hint");
		//memcpy((char*)gdata.mappedStagingVAPtr + offset, data, sizeinBytes);
		memcpy(gdata.mappedStagingVAPtr, data, sizeinBytes);
		rsvd::Vertex* vertices = static_cast<rsvd::Vertex*>(gdata.mappedStagingVAPtr);

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateIndices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data) {
	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];
		//memcpy((char*)(gdata.mappedStagingVAPtr) + offset, data, sizeInBytes);
		memcpy(gdata.mappedIndexPtr, data, sizeInBytes);
		uint32_t* indices = static_cast<uint32_t*>(gdata.mappedIndexPtr);
		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}


RSresult VkRenderSystem::geometryDataFinalize(const RSgeometryDataID& gdataID) {
	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];

		//unmap the host pointers
		vkUnmapMemory(iinstance.device, gdata.stagingVAbufferMemory);
		gdata.mappedStagingVAPtr = nullptr;
		//copy to device buffer
		VkDeviceSize vaBufferSize = gdata.numVertices * gdata.attributesInfo.sizeOfAttrib();
		copyBuffer(gdata.stagingVABuffer, gdata.vaBuffer, vaBufferSize);
		//destroy the staging buffers
		vkDestroyBuffer(iinstance.device, gdata.stagingVABuffer, nullptr);
		vkFreeMemory(iinstance.device, gdata.stagingVAbufferMemory, nullptr);

		if (gdata.numIndices) {
			vkUnmapMemory(iinstance.device, gdata.stagingIndexBufferMemory);
			gdata.mappedIndexPtr = nullptr;
			//copy to device buffer
			VkDeviceSize indexBufferSize = gdata.numIndices * sizeof(uint32_t);
			copyBuffer(gdata.stagingIndexBuffer, gdata.indicesBuffer, indexBufferSize);
			//destroy the staging buffers
			vkDestroyBuffer(iinstance.device, gdata.stagingIndexBuffer, nullptr);
			vkFreeMemory(iinstance.device, gdata.stagingIndexBufferMemory, nullptr);
		}

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataDispose(const RSgeometryDataID& gdataID) {
	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];
		vkDestroyBuffer(iinstance.device, gdata.vaBuffer, nullptr);
		vkFreeMemory(iinstance.device, gdata.vaBufferMemory, nullptr);

		if (gdata.numIndices) {
			vkDestroyBuffer(iinstance.device, gdata.indicesBuffer, nullptr);
			vkFreeMemory(iinstance.device, gdata.indicesBufferMemory, nullptr);
		}
		igeometryDataMap.erase(gdataID);

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

bool VkRenderSystem::geometryAvailable(const RSgeometryID& geomID) {
	return geomID.isValid() && igeometryMap.find(geomID) != igeometryMap.end();
}

RSresult VkRenderSystem::geometryCreate(RSgeometryID& outgeomID, const RSgeometryInfo& geomInfo) {
	RSuint id;
	bool success = igeometryIDpool.CreateID(id);
	assert(success && "failed to create a view ID");
	if (success) {
		VkRSgeometry vkrsgeom;
		vkrsgeom.geomInfo = geomInfo;

		outgeomID.id = id;
		igeometryMap[outgeomID] = vkrsgeom;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;

}

RSresult VkRenderSystem::geometryDispose(const RSgeometryID& geomID) {
	if (geometryAvailable(geomID)) {
		igeometryMap.erase(geomID);

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

