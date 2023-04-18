#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RSdataTypes.h"
#include <string>

struct VkRSinstance {
	static const inline std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	static const inline std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	std::vector<std::string> gpuSupportedExtension;
	std::vector<std::string> vkRequiredExtensions;
	std::vector<std::string> vkExtensionProps;
	std::vector<std::string> vkSupportedValidationLayers;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
};


struct VkRScontext {
	VkSurfaceKHR surface;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool; //manages the memory where command buffers are allocated from them
	std::vector<VkCommandBuffer> commandBuffers; //gets automatically disposed when command pool is disposed.
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
};

struct VkRSview {
	RSview view;
};