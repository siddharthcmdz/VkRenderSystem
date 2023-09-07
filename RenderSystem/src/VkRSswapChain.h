//#pragma once
//
//#include <vulkan/vulkan.h>
//#include <vector>
//
//struct VkRSswapChainBuffer {
//	VkImage image;
//	VkImageView view;
//};
//
//class VulkanSwapChain {
//private:
//	VkInstance iinstance;
//	VkDevice idevice;
//	VkPhysicalDevice iphysicalDevice;
//	VkSurfaceKHR isurface;
//
//public:
//	VkFormat colorFormat;
//	VkColorSpaceKHR colorSpace;
//	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
//	uint32_t imageCount;
//	std::vector<VkImage> images;
//	std::vector<VkRSswapChainBuffer> buffers;
//	uint32_t queueNodeIndex = UINT32_MAX;
//
//#ifdef VK_USE_PLATFORM_WIN32_KHR
//	void initSurface(void* platformHandle, void* platformWindow);
//#endif
//	void connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
//	void create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
//	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
//	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
//	void cleanup();
//};
