#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RSdataTypes.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <optional>
#include "rsenums.h"

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
	VkQueue graphicsQueue{};
	VkQueue presentQueue{};
	VkCommandPool commandPool{}; //manages the memory where command buffers are allocated from them
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
	VkSwapchainKHR swapChain{};
	VkFormat swapChainImageFormat{};
	VkExtent2D swapChainExtent{};
};

struct VkRScollection {
	RScollectionInfo info;
	VkRenderPass renderPass{};
	VkPipelineLayout pipelineLayout{};
	VkPipeline graphicsPipeline{};
	std::vector<VkCommandBuffer> commandBuffers; //gets automatically disposed when command pool is disposed.
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	using RSinstances = std::unordered_map<RSinstanceID, VkRSinstanceData, IDHasher<RSinstanceID>>;

	RSinstances instanceMap;
	bool dirty = true;
};

struct AllocationID {
	
	uint32_t offset;
	uint32_t numElements;
	uint16_t elemSize;
};

struct VkRSgeometryData {
	uint32_t numVertices = 0;
	uint32_t numIndices = 0;
	RSvertexAttribsInfo attributesInfo;
	RSbufferUsageHints usageHints = RSbufferUsageHints::buVertices;
	
	VkBuffer vaBuffer = VK_NULL_HANDLE;
	VkBuffer stagingVABuffer = VK_NULL_HANDLE;
	VkDeviceMemory vaBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory stagingVAbufferMemory = VK_NULL_HANDLE;
	void* mappedStagingVAPtr = nullptr;

	VkBuffer indicesBuffer = VK_NULL_HANDLE;
	VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indicesBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory stagingIndexBufferMemory = VK_NULL_HANDLE;
	void* mappedIndexPtr = nullptr;

};

struct VkRSinstanceData {
	RSinstanceInfo instInfo;
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
