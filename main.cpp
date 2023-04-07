#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>           //for uint32_t
#include <limits>            //for std::numeric_limits
#include <algorithm>         //for std::clamp

class HelloTriangleApplication {
private:
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	std::vector<const char*> ivalidationLayers = { "VK_LAYER_KHRONOS_validation" };
	std::vector<const char*> ideviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<VkImage> iswapChainImages;
	std::vector<VkImageView> iswapChainImageViews;

	GLFWwindow* iwindow;
	VkInstance iinstance;
	VkDebugUtilsMessengerEXT idebugMessenger;
	VkPhysicalDevice iphysicalDevice = VK_NULL_HANDLE;
	VkDevice idevice;
	VkQueue igraphicsQueue;
	VkQueue ipresentQueue;
	VkSurfaceKHR isurface;
	VkSwapchainKHR iswapChain;
	VkFormat iswapChainImageFormat;
	VkExtent2D iswapChainExtent;

#ifdef NDEBUG
	const bool ienableValidationLayers = false;
#else
	const bool ienableValidationLayers = true;
#endif // NDEBUG

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
									   VkDebugUtilsMessengerEXT debugMessenger, 
									   const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
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

	void setupDebugMessenger() {
		if (!ienableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(iinstance, &createInfo, nullptr, &idebugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::cout << "\nAvailable layers:" << std::endl;
		for (const char* layerName : ivalidationLayers) {
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

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (ienableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void printPhysicalDeviceInfo(VkPhysicalDevice device) {
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

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(iwindow, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t availableExtensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(ideviceExtensions.begin(), ideviceExtensions.end());
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(iinstance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(iinstance, &deviceCount, devices.data());
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				iphysicalDevice = device;
				printPhysicalDeviceInfo(device);
				break;
			}
		}

		if (iphysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable card");
		}
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, isurface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
		}

		return indices;
	}

	void createInstance() {
		if (ienableValidationLayers && !checkValidationLayerSupport()) {
			std::runtime_error("Validation layers requested by not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (ienableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(ivalidationLayers.size());
			createInfo.ppEnabledLayerNames = ivalidationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkResult result = vkCreateInstance(&createInfo, nullptr, &iinstance);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

		//retrieve a list of supported extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensionProperties(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
		std::cout << "\nInstance extensions: " << std::endl;
		for (const auto& extension : extensionProperties) {
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, isurface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, isurface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, isurface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, isurface, &presentModeCount, nullptr);
		if (presentModeCount) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, isurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void createLogicalDevice() {
		//create a collection of queue create infos to set all at once on device create info.
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		QueueFamilyIndices indices = findQueueFamilies(iphysicalDevice);
		std::set<uint32_t> uniqueQueueFamiles = { indices.graphicsFamily.value(), indices.presentFamily.value()};

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
		createInfo.enabledExtensionCount = static_cast<uint32_t>(ideviceExtensions.size());
		createInfo.ppEnabledExtensionNames = ideviceExtensions.data();

		if (ienableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(ivalidationLayers.size());
			createInfo.ppEnabledLayerNames = ivalidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		const VkResult result = vkCreateDevice(iphysicalDevice, &createInfo, nullptr, &idevice);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device");
		}

		//get the graphics queue handle from the logical device.
		vkGetDeviceQueue(idevice, indices.graphicsFamily.value(), 0, &igraphicsQueue);

		//get the present queue handle from the logical device
		vkGetDeviceQueue(idevice, indices.presentFamily.value(), 0, &ipresentQueue);
	}

	void createSurface() {
		const VkResult result = glfwCreateWindowSurface(iinstance, iwindow, nullptr, &isurface);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create a window surface!");
		}

	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(iphysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//TODO: perhaps use std::clamp here
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = isurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.presentMode = presentMode;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; //this is to specify if you're using a stereoscropic 3D application or not.
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //this means that swap chain is treated like a bunch of attachments. if we want to post process, then use VK_IMAGE_USAGE_TRANSFER_DST_BIT  and perform a memory operation to transfer the rendered image to a swap chain image.

		QueueFamilyIndices indices = findQueueFamilies(iphysicalDevice);
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

		const VkResult result = vkCreateSwapchainKHR(idevice, &createInfo, nullptr, &iswapChain);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create swapchain!");
		}

		vkGetSwapchainImagesKHR(idevice, iswapChain, &imageCount, nullptr);
		iswapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(idevice, iswapChain, &imageCount, iswapChainImages.data());

		iswapChainImageFormat = surfaceFormat.format;
		iswapChainExtent = extent;
	}

	void createImageViews() {
		iswapChainImageViews.resize(iswapChainImages.size());

		for (size_t i = 0; i < iswapChainImages.size(); ++i) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = iswapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = iswapChainImageFormat;
			
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			const VkResult result = vkCreateImageView(idevice, &createInfo, nullptr, &iswapChainImageViews[i]);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(iwindow)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		for (const auto& imageView : iswapChainImageViews) {
			vkDestroyImageView(idevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(idevice, iswapChain, nullptr);
		vkDestroyDevice(idevice, nullptr);
		if (ienableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(iinstance, idebugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(iinstance, isurface, nullptr);
		vkDestroyInstance(iinstance, nullptr);

		glfwDestroyWindow(iwindow);
		
		glfwTerminate();
	}

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		iwindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

public:
	HelloTriangleApplication() : iwindow(nullptr) {}

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}