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

#include "TextureLoader.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

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

	std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	};

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
	appInfo.apiVersion = VK_API_VERSION_1_2;

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
	if (!iinitInfo.enableValidation)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(iinstance.instance, &createInfo, nullptr, &iinstance.debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

}

void VkRenderSystem::createSurface(VkRScontext& vkrsctx) {
	
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = vkrsctx.info.hinst;
	surfaceCreateInfo.hwnd = vkrsctx.info.hwnd;
	VkResult res = vkCreateWin32SurfaceKHR(iinstance.instance, &surfaceCreateInfo, nullptr, &vkrsctx.surface);

	if (res!= VK_SUCCESS) {
		throw std::runtime_error("failed to create a window surface!");
	}
}

void VkRenderSystem::createFramebuffers(VkRSview& view) {
	view.swapChainFramebuffers.resize(view.swapChainImageViews.size());

	for (size_t i = 0; i < view.swapChainImageViews.size(); ++i) {
		std::array<VkImageView, 2> attachments = { view.swapChainImageViews[i] , view.depthImageView };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = view.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = view.swapChainExtent.width;
		framebufferInfo.height = view.swapChainExtent.height;
		framebufferInfo.layers = 1;

		const VkResult fboresult = vkCreateFramebuffer(iinstance.device, &framebufferInfo, nullptr, &view.swapChainFramebuffers[i]);
		if (fboresult != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer");
		}
	}
}

void VkRenderSystem::cleanupSwapChain(VkRSview& view) {
	vkDestroyImageView(iinstance.device, view.depthImageView, nullptr);
	vkDestroyImage(iinstance.device, view.depthImage, nullptr);
	vkFreeMemory(iinstance.device, view.depthImageMemory, nullptr);
	view.depthImageView = VK_NULL_HANDLE;
	view.depthImage = VK_NULL_HANDLE;
	view.depthImageMemory = VK_NULL_HANDLE;

	for (auto framebuffer : view.swapChainFramebuffers) {
		vkDestroyFramebuffer(iinstance.device, framebuffer, nullptr);
	}

	for (const auto& imageView : view.swapChainImageViews) {
		vkDestroyImageView(iinstance.device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(iinstance.device, view.swapChain, nullptr);
}

void VkRenderSystem::recreateSwapchain(VkRScontext& ctx, VkRSview& view) {
	if (ctx.resized) {
		vkDeviceWaitIdle(iinstance.device);

		cleanupSwapChain(view);

		createSwapChain(view, ctx);
		createImageViews(view);
		createDepthResources(view);
		createFramebuffers(view);
		
		vkDeviceWaitIdle(iinstance.device);

		ctx.resized = false;
	}
}

void VkRenderSystem::contextDrawCollections(VkRScontext& ctx, VkRSview& view, const VkRScollection* collections, uint32_t numCollections) {
	const VkDevice& device = iinstance.device;
	uint32_t currentFrame = view.currentFrame;
	const VkFence inflightFence = ctx.inFlightFences[currentFrame];
	const VkSemaphore imageAvailableSemaphore = ctx.imageAvailableSemaphores[currentFrame];
	const VkSemaphore renderFinishedSemaphore = ctx.renderFinishedSemaphores[currentFrame];

	vkWaitForFences(device, 1, &inflightFence, VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult res = vkAcquireNextImageKHR(device, view.swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	//only reset the fence if we are submitting work.
	vkResetFences(device, 1, &inflightFence);
	
	updateUniformBuffer(view, currentFrame);
	
	const VkCommandBuffer commandBuffer = view.commandBuffers[currentFrame];
	vkResetCommandBuffer(commandBuffer, 0);

	recordCommandBuffer(collections, numCollections, view, ctx, imageIndex, currentFrame);
		
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

	VkSwapchainKHR swapChains[] = { view.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	res = vkQueuePresentKHR(iinstance.presentQueue, &presentInfo);

	view.currentFrame = (currentFrame + 1) % VkRScontext::MAX_FRAMES_IN_FLIGHT;
}

RSresult VkRenderSystem::contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID) {
	assert(ctxID.isValid() && "invalid input context ID");
	assert(viewID.isValid() && "input viewID is not valid");

	if (iinitInfo.onScreenCanvas) {
		if (contextAvailable(ctxID) && viewAvailable(viewID)) {
			VkRScontext& ctx = ictxMap[ctxID.id];
			VkRSview& view = iviewMap[viewID];
			if (view.swapChainFramebuffers.empty()) {
				createDepthResources(view);
				createFramebuffers(view);
			}

			std::vector<VkRScollection> collections;
			for (const auto& collID : view.collectionIDlist) {
				const VkRScollection& coll = icollectionMap[collID];
				collections.push_back(coll);
			}

			contextDrawCollections(ctx, view, collections.data(), static_cast<uint32_t>(collections.size()));

			VkResult res = vkDeviceWaitIdle(iinstance.device);
			if (res != VK_SUCCESS) {
				throw std::runtime_error("Failed to wait for device to become idle");
			}
		}
	}

	return RSresult::SUCCESS;
}

VkRSqueueFamilyIndices VkRenderSystem::findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR& surface) {
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

VkRSswapChainSupportDetails VkRenderSystem::querySwapChainSupport(VkPhysicalDevice device, const VkSurfaceKHR& vksurface) {
	VkRSswapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vksurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vksurface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vksurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vksurface, &presentModeCount, nullptr);
	if (presentModeCount) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vksurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool VkRenderSystem::isDeviceSuitable(VkPhysicalDevice device, const VkSurfaceKHR& vksurface) {
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	VkRSqueueFamilyIndices indices = findQueueFamilies(device, vksurface);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		VkRSswapChainSupportDetails swapChainSupport = querySwapChainSupport(device, vksurface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);


	return indices.isComplete() && extensionsSupported && swapChainAdequate && 
		supportedFeatures.samplerAnisotropy && 
		supportedFeatures.wideLines && 
		supportedFeatures.fillModeNonSolid ;
	
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
	iinstance.maxBoundDescriptorSets = props.limits.maxBoundDescriptorSets;
	iinstance.maxCombinedSamplerDescriptorSets = props.limits.maxDescriptorSetSampledImages;
	iinstance.maxUniformDescriptorSets = props.limits.maxDescriptorSetUniformBuffers;
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
	std::cout << "\tmaxBoundDescriptorSets: " << iinstance.maxBoundDescriptorSets << std::endl;
	std::cout << "\tmaxCombinedSamplerDescriptorSets: "<< iinstance.maxCombinedSamplerDescriptorSets << std::endl;
	std::cout << "\tmaxUniformDescriptorSets: " << iinstance.maxUniformDescriptorSets << std::endl;
	std::cout << "\tfillModeNonSolid: " << features.fillModeNonSolid << std::endl;
	std::cout << "\twideLines: " << features.wideLines << std::endl;
}

void VkRenderSystem::pickPhysicalDevice() {
	//pick just the first device. TODO: if this fails, we need to allow users to pick the GPU for us.
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(iinstance.instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(iinstance.instance, &deviceCount, devices.data());
	if (!devices.empty()) {
		const VkPhysicalDevice device = devices[0];
		iinstance.physicalDevice = device;
		printPhysicalDeviceInfo(device);
	}

	if (iinstance.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable card");
	}
}

void VkRenderSystem::createLogicalDevice(const VkSurfaceKHR& vksurface) {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	VkRSqueueFamilyIndices indices = findQueueFamilies(iinstance.physicalDevice, vksurface);
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
	//enable sampler support for anisotropy filtering
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(iinstance.deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = iinstance.deviceExtensions.data();

	if (iinitInfo.enableValidation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(iinstance.validationLayers.size());
		createInfo.ppEnabledLayerNames = iinstance.validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	VkPhysicalDeviceRobustness2FeaturesEXT robustness2{};
	robustness2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
	robustness2.nullDescriptor = VK_TRUE;
	
	createInfo.pNext = &robustness2;

	const VkResult result = vkCreateDevice(iinstance.physicalDevice, &createInfo, nullptr, &iinstance.device);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device");
	}
	
	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext = &robustness2;
	vkGetPhysicalDeviceFeatures2(iinstance.physicalDevice, &features2);
	

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

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, HWND wnd) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		RECT rect;
		if (GetWindowRect(wnd, &rect)) {
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
		}

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VkRenderSystem::createSwapChain(VkRSview& view, VkRScontext& ctx) {
	VkRSswapChainSupportDetails swapChainSupport = querySwapChainSupport(iinstance.physicalDevice, ctx.surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, ctx.info.hwnd);

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

	VkRSqueueFamilyIndices indices = findQueueFamilies(iinstance.physicalDevice, ctx.surface);
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
	const VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &view.swapChain);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(device, view.swapChain, &imageCount, nullptr);
	view.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, view.swapChain, &imageCount, view.swapChainImages.data());

	view.swapChainImageFormat = surfaceFormat.format;
	view.swapChainExtent = extent;
}

VkSurfaceKHR VkRenderSystem::createDummySurface(const HWND hwnd, const HINSTANCE hinst) {
	VkSurfaceKHR surface;
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hinst;
	surfaceCreateInfo.hwnd = hwnd;
	VkResult res = vkCreateWin32SurfaceKHR(iinstance.instance, &surfaceCreateInfo, nullptr, &surface);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create a dummy surface!");
	}

	return surface;
}

void VkRenderSystem::disposeDummySurface(const VkSurfaceKHR surface) {
	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(iinstance.instance, surface, nullptr);
	}
}

RSresult VkRenderSystem::renderSystemInit(const RSinitInfo& info)
{
	iinitInfo = info;

	createInstance(info);
	setupDebugMessenger();

	pickPhysicalDevice();
	VkSurfaceKHR dummySurface = createDummySurface(info.parentHwnd, info.parentHinst);
	createLogicalDevice(dummySurface);
	disposeDummySurface(dummySurface);

	createDescriptorPool();

	ishaderModuleMap[RSshaderTemplate::stPassthrough] = createShaderModule(RSshaderTemplate::stPassthrough);
    ishaderModuleMap[RSshaderTemplate::stTextured] = createShaderModule(RSshaderTemplate::stTextured);
	ishaderModuleMap[RSshaderTemplate::stSimpleLit] = createShaderModule(RSshaderTemplate::stSimpleLit);

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
	vkDestroyDescriptorPool(iinstance.device, iinstance.descriptorPool, nullptr);
	vkDestroyDevice(iinstance.device, nullptr);
	if (iinitInfo.enableValidation) {
		DestroyDebugUtilsMessengerEXT(iinstance.instance, iinstance.debugMessenger, nullptr);
	}

	vkDestroyInstance(iinstance.instance, nullptr);
	
	return RSresult::FAILURE;
}

void VkRenderSystem::createImageViews(VkRSview& view) {
	view.swapChainImageViews.resize(view.swapChainImages.size());

	for (size_t i = 0; i < view.swapChainImages.size(); ++i) {
		view.swapChainImageViews[i] = createImageView(view.swapChainImages[i], view.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void VkRenderSystem::createDepthResources(VkRSview& view) {
	VkFormat depthFormat = findDepthFormat();
	createImage(view.swapChainExtent.width, view.swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, view.depthImage, view.depthImageMemory);
	view.depthImageView = createImageView(view.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

bool VkRenderSystem::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat VkRenderSystem::findDepthFormat() {
	return findSupportedFormat(
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat VkRenderSystem::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(iinstance.physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

void VkRenderSystem::contextResized(const RScontextID& ctxID, const RSviewID& viewID, uint32_t newWidth, uint32_t newHeight) {
	assert(ctxID.isValid() && "input context ID is not valid");
	assert(viewID.isValid() && "input view ID is not valid");
	if (contextAvailable(ctxID) && viewAvailable(viewID)) {
		VkRScontext& ctx = ictxMap[ctxID];
		VkRSview& view = iviewMap[viewID];
		ctx.resized = true;
		ctx.width = newWidth;
		ctx.height = newHeight;
		recreateSwapchain(ctx, view);
	}
}

RSresult VkRenderSystem::contextCreate(RScontextID& outCtxID, const RScontextInfo& info) {

	RSuint id;
	bool success = ictxIDpool.CreateID(id);
	assert(success && "failed to create a context ID");
	if (success) {
		VkRScontext vkrsctx;
		vkrsctx.info = info;

		createSurface(vkrsctx);
		createCommandPool(vkrsctx);
		createSyncObjects(vkrsctx);
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
	vkDestroySurfaceKHR(iinstance.instance, ctx.surface, nullptr);
	ctx.surface = nullptr;

	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(device, ctx.imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, ctx.renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, ctx.inFlightFences[i], nullptr);
	}

	ctx.info.hwnd = nullptr;
}

RSresult VkRenderSystem::contextDispose(const RScontextID& ctxID) {

	assert(ctxID.isValid() && "input context ID is not valid");

	if (contextAvailable(ctxID)) {
		VkRScontext& ctx = ictxMap[ctxID.id];

		disposeContext(ctx);
		ictxMap.erase(ctxID.id);

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

void VkRenderSystem::viewCreateDescriptorSetLayout(VkRSview& view) {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1; //all view related parameters\data are sourced by one buffer
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

void VkRenderSystem::createDescriptorPool() {
	// counted number of individual variables in the shader. update this if we want more
	const uint32_t MAX_FRAMES_IN_FLIGHT = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	const uint32_t maxUniformDescriptors = 4 * MAX_FRAMES_IN_FLIGHT;
	const uint32_t maxSamplerDescriptors = 1 * MAX_FRAMES_IN_FLIGHT;
	const uint32_t maxDescriptorSets = 3 * MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolSize uniformPoolSize{};
	uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformPoolSize.descriptorCount = maxUniformDescriptors;

	VkDescriptorPoolSize samplerPoolSize{};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = maxSamplerDescriptors;
		
	std::array<VkDescriptorPoolSize, 2> poolSizes = { uniformPoolSize, samplerPoolSize };

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = maxDescriptorSets;

	VkResult res = vkCreateDescriptorPool(iinstance.device, &poolInfo, nullptr, &iinstance.descriptorPool);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool");
	}
}

void VkRenderSystem::viewCreateDescriptorSets(VkRSview& view) {
	std::vector<VkDescriptorSetLayout> layouts(VkRScontext::MAX_FRAMES_IN_FLIGHT, view.descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = iinstance.descriptorPool;
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

void VkRenderSystem::updateUniformBuffer(VkRSview& view, uint32_t currentFrame) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	VkRSviewDescriptor ubo{};
	//glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = view.view.projmat;
	ubo.view = view.view.viewmat;
	//std::string newprojstr = glm::to_string(view.view.projmat);
	//std::string newviewstr = glm::to_string(view.view.viewmat);
	//std::cout << "new proj: \n" << newprojstr << std::endl;
	//std::cout << "new view: \n" << newviewstr << std::endl;

	//glm::mat4 oldview = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//glm::mat4 oldproj = glm::perspective(glm::radians(45.0f), ((float)view.swapChainExtent.width / (float)view.swapChainExtent.height), 0.01f, 1000.0f);
	//oldproj[1][1] *= -1;
	////ubo.proj = oldproj;
	////ubo.view = oldview;
	//std::string oldprojstr = glm::to_string(oldproj);
	//std::string oldviewstr = glm::to_string(oldview);
	//std::cout << "old aspect ratio: " << ((float)view.swapChainExtent.width / (float)view.swapChainExtent.height) << std::endl;
	//std::cout << "old proj: \n" << oldprojstr << std::endl;
	//std::cout << "old view: \n" << oldviewstr << std::endl;
	//std::cout << std::endl << std::endl;
	memcpy(view.uniformBuffersMapped[currentFrame], &ubo, sizeof(VkRSviewDescriptor));
}

bool VkRenderSystem::viewAvailable(const RSviewID& viewID) const {
	return viewID.isValid() && iviewMap.find(viewID) != iviewMap.end();
}

RSresult VkRenderSystem::viewCreate(RSviewID& outViewID, const RSview& view, const RScontextID& ctxID) {
	
	assert(ctxID.isValid() && "input contextID is not valid");
	assert(contextAvailable(ctxID) && "invalid context ID");

	if (contextAvailable(ctxID)) {
		RSuint id;
		bool success = iviewIDpool.CreateID(id);
		assert(success && "failed to create a view ID");
		if (success) {
			VkRSview vkrsview;
			vkrsview.view = view;
		
			VkRScontext& vkrsctx = ictxMap[ctxID];
			createSwapChain(vkrsview, vkrsctx);
			createImageViews(vkrsview);
			createRenderpass(vkrsview);

			viewCreateDescriptorSetLayout(vkrsview);
			createUniformBuffers(vkrsview);
			viewCreateDescriptorSets(vkrsview);
			createCommandBuffers(vkrsview);

			outViewID.id = id;
			iviewMap[outViewID] = vkrsview;
			return RSresult::SUCCESS;
		}
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

RSresult VkRenderSystem::viewAddCollection(const RSviewID& viewID, const RScollectionID& colID) {
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		vkrsview.collectionIDlist.push_back(colID);
		vkrsview.view.dirty = true;
		return RSresult::SUCCESS;
	}
	
	assert(viewID.isValid() && "input viewID is not valid");
	assert(colID.isValid() && "input collectionID is not valid");

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewRemoveCollection(const RSviewID& viewID, const RScollectionID& colID) {

	assert(viewID.isValid() && "input viewID is not valid");
	assert(colID.isValid() && "input collectionID is not valid");

	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		vkrsview.collectionIDlist.erase(std::remove(vkrsview.collectionIDlist.begin(), vkrsview.collectionIDlist.end(), colID.id));
		vkrsview.view.dirty = true;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

std::optional<RSview> VkRenderSystem::viewGetData(const RSviewID& viewID) {
	std::optional<RSview> optionalView;
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		optionalView = vkrsview.view;
	}

	return optionalView;
}

//RSresult VkRenderSystem::viewFinalize(const RSviewID& viewID) {
//	if (viewAvailable(viewID)) {
//		VkRSview& vkrsview = iviewMap[viewID];
//		vkrsview.view.dirty = false;
//		
//		//finalize the view
//
//		return RSresult::SUCCESS;
//	}
//
//	assert(viewID.isValid() && "input viewID is not valid");
//
//	return RSresult::FAILURE;
//}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	if (viewAvailable(viewID)) {
		VkRSview& vkrsview = iviewMap[viewID];
		//dispose view contents
		disposeView(vkrsview);
		iviewMap.erase(viewID);
		return RSresult::SUCCESS;
	}

	assert(viewID.isValid() && "input viewID is not valid");

	return RSresult::FAILURE;
}

void VkRenderSystem::disposeView(VkRSview& view) {
	
	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(iinstance.device, view.uniformBuffers[i], nullptr);
		vkFreeMemory(iinstance.device, view.uniformBuffersMemory[i], nullptr);
	}
	vkDestroyDescriptorSetLayout(iinstance.device, view.descriptorSetLayout, nullptr);
	vkDestroyRenderPass(iinstance.device, view.renderPass, nullptr);
	
	for (const auto& imageView : view.swapChainImageViews) {
		vkDestroyImageView(iinstance.device, imageView, nullptr);
	}
	view.swapChainImageViews.clear();
	vkDestroySwapchainKHR(iinstance.device, view.swapChain, nullptr);
	view.swapChain = nullptr;

	for (auto framebuffer : view.swapChainFramebuffers) {
		vkDestroyFramebuffer(iinstance.device, framebuffer, nullptr);
	}
}

void VkRenderSystem::createRenderpass(VkRSview& view) {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = view.swapChainImageFormat;
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

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	const VkResult result = vkCreateRenderPass(iinstance.device, &renderPassInfo, nullptr, &view.renderPass);
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

	assert(collID.isValid() && "input collection ID is not valid");
	assert(instanceID.isValid() && "input collection instance ID is not valid");

	if (collectionAvailable(collID)) {
		const VkRScollection& coll = icollectionMap[collID];
		if (instanceID.isValid() && coll.instanceMap.find(instanceID) != coll.instanceMap.end()) {
			return true;
		}
	}

	return false;
}

bool VkRenderSystem::needsMaterialDescriptor(VkRScollectionInstance& inst) {
	if (appearanceAvailable(inst.instInfo.appID)) {
		const VkRSappearance& app = iappearanceMap[inst.instInfo.appID];
		//put in the list of shaders that dont need a discriptor set here.
		if (app.appInfo.shaderTemplate == stPassthrough) {
			return false;
		}
	}

	return true;
}

RSresult VkRenderSystem::collectionInstanceCreate(RScollectionID& collID, RSinstanceID& outInstID, const RSinstanceInfo& instInfo) {
	
	assert(collID.isValid() && "input collection ID is not valid");
	
	if (!appearanceAvailable(instInfo.appID) || !geometryDataAvailable(instInfo.gdataID) || !geometryAvailable(instInfo.geomID) || !spatialAvailable(instInfo.spatialID)) {
		return RSresult::FAILURE;
	}

	if (collectionAvailable(collID)) {
		uint32_t id;
		iinstanceIDpool.CreateID(id);
		outInstID.id = id;
		VkRScollection& coll = icollectionMap[collID];

		VkRScollectionInstance inst;
		inst.instInfo = instInfo;

		if (needsMaterialDescriptor(inst)) {
			collectionInstanceCreateDescriptorSetLayout(inst);
			collectionInstanceCreateDescriptorSet(inst);
		}
		coll.instanceMap[outInstID] = inst;

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::collectionInstanceDispose(RScollectionID& collID, RSinstanceID& instID) {

	assert(collID.isValid() && "input collection ID is not valid");
	assert(instID.isValid() && "input collection instance ID is not valid");

	if (collectionInstanceAvailable(collID, instID)) {
		VkRScollection& coll = icollectionMap[collID];
		VkRScollectionInstance& vkrsinst = coll.instanceMap[instID];
		vkDestroyDescriptorSetLayout(iinstance.device, vkrsinst.descriptorSetLayout, nullptr);

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

VKRSshader VkRenderSystem::createShaderModule(const RSshaderTemplate shaderTemplate) {
    
    const std::string spvfileName = getShaderStr(shaderTemplate);
    std::string shaderPathStr = iinitInfo.shaderPath;
    
    //shaderPathStr ends with a "/"
    std::string vertShaderPath = shaderPathStr + "/" + spvfileName + "_vert.spv";
    std::string fragShaderPath = shaderPathStr + "/" + spvfileName + "_frag.spv";
    const std::vector<char>& vertfile = readFile(vertShaderPath);
    const std::vector<char>& fragfile = readFile(fragShaderPath);
    VKRSshader vkrsshader;
    vkrsshader.shadernName = spvfileName;
	vkrsshader.vertShaderContent = vertfile.data();
	vkrsshader.fragShaderContent = fragfile.data();

    VkShaderModuleCreateInfo vertShaderModuleCI{};
    vertShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderModuleCI.codeSize = vertfile.size();
    vertShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(vertfile.data());

    VkResult result = vkCreateShaderModule(iinstance.device, &vertShaderModuleCI, nullptr, &vkrsshader.vert);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create a shader module!");
    }
    
    VkShaderModuleCreateInfo fragShaderModuleCI{};
    fragShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderModuleCI.codeSize = fragfile.size();
    fragShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(fragfile.data());

    result = vkCreateShaderModule(iinstance.device, &fragShaderModuleCI, nullptr, &vkrsshader.frag);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create a shader module!");
    }

    return vkrsshader;
}

void VkRenderSystem::createGraphicsPipeline(const VkRScontext& ctx, const VkRSview& view, VkRScollection& collection, VkRScollectionInstance& collinst, VkRSdrawCommand& drawcmd) {
	const RSappearanceID& appID = collinst.instInfo.appID;
	if (!appearanceAvailable(appID)) {
		throw std::runtime_error("invalid appearance");
	}
	const VkRSappearance& vkrsapp = iappearanceMap[appID];
    const RSshaderTemplate shaderTemplate = vkrsapp.appInfo.shaderTemplate;

    const VKRSshader& vkrsshader = ishaderModuleMap[shaderTemplate];
    VkShaderModule vertShaderModule = vkrsshader.vert;
    VkShaderModule fragShaderModule = vkrsshader.frag;

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

	VkRSgeometryData& gdata = igeometryDataMap[collinst.instInfo.gdataID];
	
	auto bindingDescriptions = getBindingDescription(gdata.attributesInfo);
	auto attributeDescriptions = getAttributeDescriptions(gdata.attributesInfo);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	const std::vector<VkVertexInputBindingDescription> bindings = {
		{0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX},
		{1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX},
		{2, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX},
		{3, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
	};
	const std::vector<VkVertexInputAttributeDescription> attribs = {
		{0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
		{1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
		{2, 2, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
		{3, 3, VK_FORMAT_R32G32_SFLOAT, 0}
	};
	vertexInputInfo.vertexBindingDescriptionCount = 4;
	vertexInputInfo.pVertexBindingDescriptions = bindings.data();
	vertexInputInfo.vertexAttributeDescriptionCount = 4;
	vertexInputInfo.pVertexAttributeDescriptions = attribs.data();

	//vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	//vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	//vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	//vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = drawcmd.primTopology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)view.swapChainExtent.width;
	viewport.height = (float)view.swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = view.swapChainExtent;

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
	if (drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP || drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST) {
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	}
	else if (drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
		rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
	}
	else {
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	}
	rasterizer.lineWidth = drawcmd.lineWidth;
	if (drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN || drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP) {
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	}
	else {
		rasterizer.cullMode = VK_CULL_MODE_NONE;
	}
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
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.push_back(view.descriptorSetLayout);
	if (collinst.descriptorSetLayout != VK_NULL_HANDLE) {
		descriptorSetLayouts.push_back(collinst.descriptorSetLayout);
	}
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	const RSspatialID& spatialID = collinst.instInfo.spatialID;
	if (spatialAvailable(spatialID)) {
		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(RSspatial);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	}

	const VkResult pipelineLayoutResult = vkCreatePipelineLayout(iinstance.device, &pipelineLayoutInfo, nullptr, &drawcmd.pipelineLayout);
	if (pipelineLayoutResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = drawcmd.pipelineLayout;

	pipelineInfo.renderPass = view.renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	const VkResult graphicsPipelineResult = vkCreateGraphicsPipelines(iinstance.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &drawcmd.graphicsPipeline);
	if (graphicsPipelineResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void VkRenderSystem::recordCommandBuffer(const VkRScollection* collections, uint32_t numCollections, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	const VkCommandBuffer commandBuffer = view.commandBuffers[currentFrame];
	const VkResult beginRes = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (beginRes != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffers!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = view.renderPass;
	renderPassInfo.framebuffer = view.swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = view.swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {view.view.clearColor.r, view.view.clearColor.g, view.view.clearColor.b, view.view.clearColor.a} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
	{
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(view.swapChainExtent.width);
		viewport.height = static_cast<float>(view.swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = view.swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		for (uint32_t i = 0; i < numCollections; i++) {
			const VkRScollection& collection = collections[i];
			for (const VkRSdrawCommand& drawcmd : collection.drawCommands) {
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawcmd.graphicsPipeline);

				std::vector<VkDeviceSize> offsets;
				switch (drawcmd.attribSetting) {
				case RSvertexAttributeSettings::vasInterleaved: {
					VkBuffer vertexBuffers[]{ drawcmd.vertexBuffers[0]};
					offsets = { drawcmd.vertexOffset };
					vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets.data());
					break;
				}

				case RSvertexAttributeSettings::vasSeparate: {
					offsets = { 0, 0, 0, 0 };
					vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(drawcmd.vertexBuffers.size()), drawcmd.vertexBuffers.data(), offsets.data());
					break;
				}
				}

				if (drawcmd.isIndexed) {
					vkCmdBindIndexBuffer(commandBuffer, drawcmd.indicesBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
				}

				//bind the descriptor sets
				std::vector<VkDescriptorSet> descriptorSets;
				descriptorSets.push_back(drawcmd.viewDescriptors[currentFrame]);
				if (drawcmd.hasMaterialDescriptors) {
					descriptorSets.push_back(drawcmd.materialDescriptors[currentFrame]);
				}
				uint32_t numDescriptorSets = static_cast<uint32_t>(descriptorSets.size());
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawcmd.pipelineLayout, 0, numDescriptorSets, descriptorSets.data(), 0, nullptr);
				vkCmdPushConstants(commandBuffer, drawcmd.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RSspatial), &drawcmd.spatial);

				if (drawcmd.isIndexed) {
					vkCmdDrawIndexed(commandBuffer, drawcmd.numIndices, 1, 0, 0, 0);
				}
				else {
					vkCmdDraw(commandBuffer, drawcmd.numVertices, 1, 0, 0);
				}
			}
		}
	}
	vkCmdEndRenderPass(commandBuffer);

	const VkResult endCmdBuffRes = vkEndCommandBuffer(commandBuffer);
	if (endCmdBuffRes != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer");
	}
}

void VkRenderSystem::createCommandPool(const VkRScontext& ctx) {
	VkRSqueueFamilyIndices queueFamilyIndices = findQueueFamilies(iinstance.physicalDevice, ctx.surface);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	const VkResult cmdpoolResult = vkCreateCommandPool(iinstance.device, &poolInfo, nullptr, &iinstance.commandPool);
	if (cmdpoolResult != VK_SUCCESS) {
		throw std::runtime_error("faailed to create command pool");
	}
}

void VkRenderSystem::createCommandBuffers(VkRSview& view) {
	view.commandBuffers.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = iinstance.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)view.commandBuffers.size();

	const VkResult allocCmdBufferRes = vkAllocateCommandBuffers(iinstance.device, &allocInfo, view.commandBuffers.data());
	if (allocCmdBufferRes != VK_SUCCESS) {
		throw std::runtime_error("failed to allocated command buffers");
	}
}

void VkRenderSystem::createSyncObjects(VkRScontext& ctx) {
	ctx.imageAvailableSemaphores.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	ctx.renderFinishedSemaphores.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
	ctx.inFlightFences.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	const VkDevice& device = iinstance.device;
	for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; ++i) {
		VkResult imgAvailSemaphoreRes = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &ctx.imageAvailableSemaphores[i]);
		VkResult renderFinishedSemaphoreRes = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &ctx.renderFinishedSemaphores[i]);
		VkResult fenceRes = vkCreateFence(device, &fenceInfo, nullptr, &ctx.inFlightFences[i]);

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

void VkRenderSystem::collectionInstanceCreateDescriptorSetLayout(VkRScollectionInstance& inst) {
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerLayoutBinding;

	VkResult res = vkCreateDescriptorSetLayout(iinstance.device, &layoutInfo, nullptr, &inst.descriptorSetLayout);
	(res);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void VkRenderSystem::collectionInstanceCreateDescriptorSet(VkRScollectionInstance& inst) {
	if (appearanceAvailable(inst.instInfo.appID)) {
		VkRSappearance& vksrsapp = iappearanceMap[inst.instInfo.appID];
		const RStextureID difftexID = vksrsapp.appInfo.diffuseTexture;
		if (textureAvailable(difftexID)) {
			VkRStexture& vkrstex = itextureMap[difftexID];

			std::vector<VkDescriptorSetLayout> layouts(VkRScontext::MAX_FRAMES_IN_FLIGHT, inst.descriptorSetLayout);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = iinstance.descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);
			allocInfo.pSetLayouts = layouts.data();

			inst.descriptorSets.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
			VkResult res = vkAllocateDescriptorSets(iinstance.device, &allocInfo, inst.descriptorSets.data());
			if (res != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) {
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = vkrstex.textureImageView;
				imageInfo.sampler = vkrstex.textureSampler;

				VkWriteDescriptorSet descriptorWrites{};
				descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites.dstSet = inst.descriptorSets[i];
				descriptorWrites.dstBinding = 0;
				descriptorWrites.dstArrayElement = 0;
				descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites.descriptorCount = 1;
				descriptorWrites.pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(iinstance.device, 1, &descriptorWrites, 0, nullptr);
			}
		}
	}
}

RSresult VkRenderSystem::collectionFinalize(const RScollectionID& colID, const RScontextID& ctxID, const RSviewID& viewID) {

	assert(colID.isValid() && "input collection ID is not valid");
	assert(ctxID.isValid() && "input context ID is not valid");
	assert(viewID.isValid() && "input viewID is not valid");

	if (collectionAvailable(colID) && contextAvailable(ctxID) && viewAvailable(viewID)) {
		VkRScollection& collection = icollectionMap[colID];
		VkRScontext& ctx = ictxMap[ctxID];
		VkRSview& view = iviewMap[viewID];
		//finalize the collection
		if (collection.dirty) {


			//create drawcommands for each collection instance
			for (auto& iter : collection.instanceMap) {
				VkRScollectionInstance& collinst = iter.second;
				const RSgeometryDataID& gdataID = collinst.instInfo.gdataID;
				const RSgeometryID& geomID = collinst.instInfo.geomID;
				if (geometryDataAvailable(gdataID) && geometryAvailable(geomID)) {
					const VkRSgeometryData& gdata = igeometryDataMap[gdataID];
					const VkRSgeometry& geom = igeometryMap[geomID];
					VkRSdrawCommand cmd;
					cmd.indicesBuffer = gdata.indices.indicesBuffer;
					cmd.isIndexed = gdata.indices.indicesBuffer != VK_NULL_HANDLE;
					cmd.numIndices = gdata.numIndices;
					cmd.numVertices = gdata.numVertices;
					cmd.attribSetting = gdata.attributesInfo.settings;
					if (gdata.attributesInfo.settings == RSvertexAttributeSettings::vasInterleaved) {
						cmd.vertexBuffers = { gdata.interleaved.vaBuffer };
					} else {
						for (uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) {
							RSvertexAttribute attrib = gdata.attributesInfo.attributes[i];
							uint32_t attribIdx = static_cast<uint32_t>(attrib);
							cmd.vertexBuffers[attribIdx] = gdata.separate.buffers[attribIdx].buffer;
						}
					}
					cmd.primTopology = getPrimitiveType(geom.geomInfo.primType);
					if (stateAvailable(collinst.instInfo.stateID)) {
						VkRSstate& state = istateMap[collinst.instInfo.stateID];
						cmd.lineWidth = state.state.lnstate.lineWidth;
					}
					for (uint32_t i = 0; i < view.descriptorSets.size(); i++) {
						cmd.viewDescriptors[i] = view.descriptorSets[i];
					}
					for (uint32_t i = 0; i < collinst.descriptorSets.size(); i++) {
						cmd.materialDescriptors[i] = collinst.descriptorSets[i];
					}
					cmd.hasMaterialDescriptors = !collinst.descriptorSets.empty();
					const VkRSspatial& vkrsspatial = ispatialMap[collinst.instInfo.spatialID];
					cmd.spatial = vkrsspatial.spatial;
					//create descriptorsets for instances
					createGraphicsPipeline(ctx, view, collection, collinst, cmd);

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
	
	assert(colID.isValid() && "input collection ID is not valid");

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

	for (size_t i = 0; i < collection.drawCommands.size(); i++) {
		VkRSdrawCommand& drawcmd = collection.drawCommands[i];
		vkDestroyPipeline(device, drawcmd.graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, drawcmd.pipelineLayout, nullptr);
	}
	
}

bool VkRenderSystem::textureAvailable(const RStextureID& texID) {
	return texID.isValid() && itextureMap.find(texID) != itextureMap.end();
}

RSresult VkRenderSystem::textureCreate(RStextureID& outTexID, const char* absfilepath) {
	RSuint id;
	bool success = itextureIDpool.CreateID(id);
	assert(success && "failed to create a texture ID");
	if (success) {
		VkRStexture vkrstex;
		vkrstex.absPath = absfilepath;
		vkrstex.texinfo = TextureLoader::readTexture(absfilepath);
		createTextureImage(vkrstex);
		createTextureImageView(vkrstex);
		createTextureSampler(vkrstex);
		outTexID.id = id;
		itextureMap[outTexID] = vkrstex;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::textureCreateFromMemory(RStextureID& outTexID, unsigned char* encodedTexData, uint32_t width, uint32_t height) {
	RSuint id;
	bool success = itextureIDpool.CreateID(id);
	assert(success && "failed to create a texture ID");
	if (success) {
		VkRStexture vkrstex;
		vkrstex.texinfo = TextureLoader::readFromMemory(encodedTexData, width, height);
		createTextureImage(vkrstex);
		createTextureImageView(vkrstex);
		createTextureSampler(vkrstex);
		outTexID.id = id;
		itextureMap[outTexID] = vkrstex;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}


void VkRenderSystem::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}
	endSingleTimeCommands(commandBuffer);
}

VkImageView VkRenderSystem::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags; 
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageView imageView{};
	VkResult res = vkCreateImageView(iinstance.device, &viewInfo, nullptr, &imageView);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

bool VkRenderSystem::createTextureImage(VkRStexture& vkrstex) {
	RStextureInfo& texinfo = vkrstex.texinfo;
	VkDeviceSize imageSize = texinfo.texWidth * texinfo.texHeight * 4;
	if (texinfo.texels == nullptr) {
		return false;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(iinstance.device, stagingBufferMemory, 0, imageSize, 0, &data);
	unsigned char* texels = static_cast<unsigned char*>(texinfo.texels);
	memcpy(data, texinfo.texels, static_cast<size_t>(imageSize));
	vkUnmapMemory(iinstance.device, stagingBufferMemory);

	createImage(texinfo.texWidth, texinfo.texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkrstex.textureImage, vkrstex.textureImageMemory);

	transitionImageLayout(vkrstex.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, vkrstex.textureImage, static_cast<uint32_t>(texinfo.texWidth), static_cast<uint32_t>(texinfo.texHeight));
	transitionImageLayout(vkrstex.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(iinstance.device, stagingBuffer, nullptr);
	vkFreeMemory(iinstance.device, stagingBufferMemory, nullptr);

	return true;
}

void VkRenderSystem::createTextureImageView(VkRStexture& vkrstex) {
	//TODO: add more formats in future if needed
	vkrstex.textureImageView = createImageView(vkrstex.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VkRenderSystem::createTextureSampler(VkRStexture& vkrstex) {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(iinstance.physicalDevice, &properties);
	
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkResult res = vkCreateSampler(iinstance.device, &samplerInfo, nullptr, &vkrstex.textureSampler);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create a texture sampler");
	}
}

void VkRenderSystem::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = 0;


	VkResult res = vkCreateImage(iinstance.device, &imageInfo, nullptr, &image);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create image");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(iinstance.device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	res = vkAllocateMemory(iinstance.device, &allocInfo, nullptr, &imageMemory);

	vkBindImageMemory(iinstance.device, image, imageMemory, 0);
}

VkCommandBuffer VkRenderSystem::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = iinstance.commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer{};
	vkAllocateCommandBuffers(iinstance.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VkRenderSystem::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(iinstance.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(iinstance.graphicsQueue);

	vkFreeCommandBuffers(iinstance.device, iinstance.commandPool, 1, &commandBuffer);
}

void VkRenderSystem::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	endSingleTimeCommands(commandBuffer);
}


RSresult VkRenderSystem::textureDispose(const RStextureID& texID) {
	
	assert(texID.isValid() && "input texture ID is invalid");
	
	if (textureAvailable(texID)) {
		VkRStexture& vkrstex = itextureMap[texID];
		vkrstex.texinfo.dispose();
		
		vkDestroyImage(iinstance.device, vkrstex.textureImage, nullptr);
		vkFreeMemory(iinstance.device, vkrstex.textureImageMemory, nullptr);
		vkDestroySampler(iinstance.device, vkrstex.textureSampler, nullptr);
		vkDestroyImageView(iinstance.device, vkrstex.textureImageView, nullptr);

		itextureMap.erase(texID);

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

bool VkRenderSystem::appearanceAvailable(const RSappearanceID& appID) const {
	return appID.isValid() && iappearanceMap.find(appID) != iappearanceMap.end();
}

RSresult VkRenderSystem::appearanceCreate(RSappearanceID& outAppID, const RSappearanceInfo& appInfo) {
	RSuint id;
	bool success = iappearanceIDpool.CreateID(id);
	assert(success && "failed to create a appearance ID");
	if (success) {
		VkRSappearance vkrsapp;
		vkrsapp.appInfo = appInfo;

		outAppID.id = id;
		iappearanceMap[outAppID] = vkrsapp;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::appearanceDispose(const RSappearanceID& appID) {

	assert(appID.isValid() && "input appearance ID is invalid");

	if (appearanceAvailable(appID)) {
		iappearanceMap.erase(appID);
		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
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
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}
	endSingleTimeCommands(commandBuffer);
}

/**
* vertex input binding description describes what rate to load data from memory throughout the vertices.
*/
std::vector<VkVertexInputBindingDescription> VkRenderSystem::getBindingDescription(const RSvertexAttribsInfo& attribInfo) {
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	switch (attribInfo.settings) {
	case RSvertexAttributeSettings::vasInterleaved: {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = attribInfo.sizeOfInterleavedAttrib();//sizeof(rsvd::VertexPC);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions.push_back(bindingDescription);
		break;
	}

	case RSvertexAttributeSettings::vasSeparate: {
		for (uint32_t i = 0; i < attribInfo.numVertexAttribs; i++) {
			RSvertexAttribute attrib = attribInfo.attributes[i];
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = attribInfo.getBindingPoint(attrib);
			bindingDescription.stride = attribInfo.sizeOfAttrib(attrib);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			bindingDescriptions.push_back(bindingDescription);
		}
		break;
	}
	}

	
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VkRenderSystem::getAttributeDescriptions(const RSvertexAttribsInfo& attribInfo) {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	
	attributeDescriptions.resize(attribInfo.numVertexAttribs);
	for (uint32_t i = 0; i < attribInfo.numVertexAttribs; i++) {
		RSvertexAttribute attrib = attribInfo.attributes[i];
		if (attribInfo.settings == RSvertexAttributeSettings::vasInterleaved) {
			attributeDescriptions[i].binding = 0;
			attributeDescriptions[i].offset = getOffset(attribInfo.numVertexAttribs, i);
		}
		else {
			attributeDescriptions[i].binding = attribInfo.getBindingPoint(attrib);
			attributeDescriptions[i].offset = 0;
		}
		attributeDescriptions[i].location = attribInfo.getBindingPoint(attrib);
		attributeDescriptions[i].format = getVkFormat(attribInfo.attributes[i]);
	}

	return attributeDescriptions;
}

RSresult VkRenderSystem::geometryDataCreate(RSgeometryDataID& outgdataID, uint32_t numVertices, uint32_t numIndices, const RSvertexAttribsInfo attributesInfo) {
	RSuint id;
	bool success = igeomDataIDpool.CreateID(id);
	assert(success && "failed to create a geometry data ID");
	if (success) {
		VkRSgeometryData gdata;
		gdata.numVertices = numVertices;
		gdata.numIndices = numIndices;
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

		switch (gdata.attributesInfo.settings) {
		case RSvertexAttributeSettings::vasInterleaved: {
			//create buffer for staging position vertex attribs
			VkDeviceSize vaBufferSize = numVertices * attributesInfo.sizeOfInterleavedAttrib();
			createBuffer(vaBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gdata.interleaved.stagingVABuffer, gdata.interleaved.stagingVAbufferMemory);
			vkMapMemory(iinstance.device, gdata.interleaved.stagingVAbufferMemory, 0, vaBufferSize, 0, &gdata.interleaved.mappedStagingVAPtr);

			//create buffer for final vertex attributes buffer
			createBuffer(vaBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gdata.interleaved.vaBuffer, gdata.interleaved.vaBufferMemory);
			break;
		}

		case RSvertexAttributeSettings::vasSeparate: {
			for (uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) {
				RSvertexAttribute attrib = gdata.attributesInfo.attributes[i];
				VkDeviceSize bufferSize = numVertices * attributesInfo.sizeOfAttrib(attrib);
				
				//create staging buffer.
				uint32_t attribidx = static_cast<uint32_t>(attrib);
				VkRSbuffer& stageBuffer = gdata.separate.stagingBuffers[attribidx];
				createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stageBuffer.buffer, stageBuffer.memory);
				vkMapMemory(iinstance.device, stageBuffer.memory, 0, bufferSize, 0, &stageBuffer.mapped);

				//create the device buffer
				VkRSbuffer& deviceBuffer = gdata.separate.buffers[attribidx];
				createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceBuffer.buffer, deviceBuffer.memory);
			}
			break;
		}
		}

		//create buffer for indices
		if (numIndices) {
			VkDeviceSize indexBufferSize = numIndices * sizeof(uint32_t);
			createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gdata.indices.stagingIndexBuffer, gdata.indices.stagingIndexBufferMemory);
			vkMapMemory(iinstance.device, gdata.indices.stagingIndexBufferMemory, 0, indexBufferSize, 0, &gdata.indices.mappedIndexPtr);
		
			createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gdata.indices.indicesBuffer, gdata.indices.indicesBufferMemory);
		}

		outgdataID.id = id;
		igeometryDataMap[outgdataID] = gdata;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateInterleavedVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeinBytes, void* data) {

	assert(gdataID.isValid() && "input geometry data ID is invalid");

	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];
		assert(gdata.attributesInfo.settings == RSvertexAttributeSettings::vasInterleaved && "attributes are not interleaved");
		if (gdata.attributesInfo.settings != RSvertexAttributeSettings::vasInterleaved) {
			return RSresult::FAILURE;
		}

		if (offset != 0) {
			throw std::runtime_error("Unsupported operation!");
		}
		
		//memcpy((char*)gdata.mappedStagingVAPtr + offset, data, sizeinBytes);
		memcpy(gdata.interleaved.mappedStagingVAPtr, data, sizeinBytes);
		//rsvd::VertexPC* vertices = static_cast<rsvd::VertexPC*>(gdata.interleaved.mappedStagingVAPtr);

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, RSvertexAttribute attrib, void* data) {

	assert(gdataID.isValid() && "input geometry data ID is invalid");

	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];

		assert(gdata.attributesInfo.settings == RSvertexAttributeSettings::vasSeparate && "attributes are not separate");
		if (gdata.attributesInfo.settings != RSvertexAttributeSettings::vasSeparate) {
			return RSresult::FAILURE;
		}
		uint32_t attribIdx = static_cast<uint32_t>(attrib);
		memcpy(gdata.separate.stagingBuffers[attribIdx].mapped, data, sizeInBytes);

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateIndices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data) {

	assert(gdataID.isValid() && "input geometry data ID is invalid");

	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];
		//memcpy((char*)(gdata.mappedStagingVAPtr) + offset, data, sizeInBytes);
		memcpy(gdata.indices.mappedIndexPtr, data, sizeInBytes);
		uint32_t* indices = static_cast<uint32_t*>(gdata.indices.mappedIndexPtr);
		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}


RSresult VkRenderSystem::geometryDataFinalize(const RSgeometryDataID& gdataID) {

	assert(gdataID.isValid() && "input geometry data ID is invalid");

	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];

		//unmap the host pointers
		switch (gdata.attributesInfo.settings) {
		case RSvertexAttributeSettings::vasInterleaved: {
			vkUnmapMemory(iinstance.device, gdata.interleaved.stagingVAbufferMemory);
			gdata.interleaved.mappedStagingVAPtr = nullptr;
			
			//copy to device buffer
			VkDeviceSize vaBufferSize = gdata.numVertices * gdata.attributesInfo.sizeOfInterleavedAttrib();
			copyBuffer(gdata.interleaved.stagingVABuffer, gdata.interleaved.vaBuffer, vaBufferSize);
			
			//destroy the staging buffers
			vkDestroyBuffer(iinstance.device, gdata.interleaved.stagingVABuffer, nullptr);
			vkFreeMemory(iinstance.device, gdata.interleaved.stagingVAbufferMemory, nullptr);
			break;
		}

		case RSvertexAttributeSettings::vasSeparate: {
			for(uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) {
				RSvertexAttribute attrib = gdata.attributesInfo.attributes[i];
				uint32_t attribIdx = static_cast<uint32_t>(attrib);
				VkRSbuffer& stagingBuffer = gdata.separate.stagingBuffers[attribIdx];
				VkRSbuffer& deviceBuffer = gdata.separate.buffers[attribIdx];
				stagingBuffer.mapped = nullptr;

				//copy to device buffer
				VkDeviceSize bufferSize = gdata.numVertices * gdata.attributesInfo.sizeOfAttrib(attrib);
				copyBuffer(stagingBuffer.buffer, deviceBuffer.buffer, bufferSize);

				//destroy the staging buffers
				vkDestroyBuffer(iinstance.device, stagingBuffer.buffer, nullptr);
				vkFreeMemory(iinstance.device, stagingBuffer.memory, nullptr);
			}
			break;
		}
		}

		if (gdata.numIndices) {
			vkUnmapMemory(iinstance.device, gdata.indices.stagingIndexBufferMemory);
			gdata.indices.mappedIndexPtr = nullptr;
			//copy to device buffer
			VkDeviceSize indexBufferSize = gdata.numIndices * sizeof(uint32_t);
			copyBuffer(gdata.indices.stagingIndexBuffer, gdata.indices.indicesBuffer, indexBufferSize);
			//destroy the staging buffers
			vkDestroyBuffer(iinstance.device, gdata.indices.stagingIndexBuffer, nullptr);
			vkFreeMemory(iinstance.device, gdata.indices.stagingIndexBufferMemory, nullptr);
		}

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataDispose(const RSgeometryDataID& gdataID) {
	
	assert(gdataID.isValid() && "input geometry data ID is invalid");
	
	if (geometryDataAvailable(gdataID)) {
		VkRSgeometryData gdata = igeometryDataMap[gdataID];
		vkDestroyBuffer(iinstance.device, gdata.interleaved.vaBuffer, nullptr);
		vkFreeMemory(iinstance.device, gdata.interleaved.vaBufferMemory, nullptr);

		if (gdata.numIndices) {
			vkDestroyBuffer(iinstance.device, gdata.indices.indicesBuffer, nullptr);
			vkFreeMemory(iinstance.device, gdata.indices.indicesBufferMemory, nullptr);
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

	assert(geomID.isValid() && "input geometry ID is invalid");

	if (geometryAvailable(geomID)) {
		igeometryMap.erase(geomID);

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

bool VkRenderSystem::spatialAvailable(const RSspatialID& spatialID) {
	return spatialID.isValid() && ispatialMap.find(spatialID) != ispatialMap.end();
}

RSresult VkRenderSystem::spatialCreate(RSspatialID& outSplID, const RSspatial& splInfo) {
	RSuint id;
	bool success = ispatialIDpool.CreateID(id);
	assert(success && "failed to create a spatial ID");
	if (success) {
		VkRSspatial vkrsspatial;
		vkrsspatial.spatial = splInfo;

		outSplID.id = id;
		ispatialMap[outSplID] = vkrsspatial;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::spatialDispose(const RSspatialID& spatialID) {

	assert(spatialID.isValid() && "input spatial ID is invalid");

	if (spatialAvailable(spatialID)) {
		ispatialMap.erase(spatialID);

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}


bool VkRenderSystem::stateAvailable(const RSstateID& stateID) {
	return stateID.isValid() && istateMap.find(stateID) != istateMap.end();
}

RSresult VkRenderSystem::stateCreate(RSstateID& outStateID, const RSstate& state) {
	RSuint id;
	bool success = istateIDpool.CreateID(id);
	assert(success && "failed to create a state ID");
	if (success) {
		VkRSstate vkrsstate;
		vkrsstate.state= state;

		outStateID.id = id;
		istateMap[outStateID] = vkrsstate;

		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::stateDispose(const RSstateID& stateID) {

	assert(stateID.isValid() && "input state ID is invalid");

	if (stateAvailable(stateID)) {
		istateMap.erase(stateID);

		return RSresult::SUCCESS;
	}
	return RSresult::FAILURE;
}
