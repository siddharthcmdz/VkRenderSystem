#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RSdataTypes.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <optional>

struct VkRSinstance {
	static const inline std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	static const inline std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	std::vector<std::string> gpuSupportedExtension;
	std::vector<std::string> vkRequiredExtensions;
	std::vector<std::string> vkExtensionProps;
	std::vector<std::string> vkSupportedValidationLayers;
	bool enableValidation = true;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	uint32_t majorVersion = ~0;
	uint32_t minorVersion = ~0;
	uint32_t patchVersion = ~0;
	uint32_t variantVersion = ~0;
};


struct VkRScontext {
	static const int MAX_FRAMES_IN_FLIGHT = 2;
	bool framebufferResized = false;
	GLFWwindow* window = nullptr;
	VkSurfaceKHR surface{};
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkQueue graphicsQueue{};
	VkQueue presentQueue{};
	VkSwapchainKHR swapChain{};
	VkFormat swapChainImageFormat{};
	VkExtent2D swapChainExtent{};
};

struct VkRScollection {
	RScollectionInfo info;
	VkRenderPass renderPass{};
	VkPipelineLayout pipelineLayout{};
	VkPipeline graphicsPipeline{};
	VkCommandPool commandPool{}; //manages the memory where command buffers are allocated from them
	std::vector<VkCommandBuffer> commandBuffers; //gets automatically disposed when command pool is disposed.
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	bool dirty = true;
};

struct VkRSgeometryData {
	VkBuffer buffer;
	VkDeviceMemory deviceMemory;
};

struct VkRSview {
	std::vector<VkFramebuffer> swapChainFramebuffers;
	uint32_t currentFrame = 0;
	RSview view;
};

struct VkRSqueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct VkRSswapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
