//#include "VkRSswapChain.h"
//
//#ifdef VK_USE_PLATFORM_WIN32_KHR
//#include <Windows.h>
//#endif
//
//#include <stdexcept>
//#include <assert.h>
//
//#ifdef VK_USE_PLATFORM_WIN32_KHR
//void VulkanSwapChain::initSurface(void* platformHandle, void* platformWindow) {
//	VkWin32SurfaceCreateInfoKHR surfaceCI{};
//	surfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//	surfaceCI.hinstance = (HINSTANCE)platformHandle;
//	surfaceCI.hwnd = (HWND)platformWindow;
//	
//	VkResult res = vkCreateWin32SurfaceKHR(iinstance, &surfaceCI, nullptr, &isurface);
//	if (res != VK_SUCCESS) {
//		throw std::runtime_error("failed to create a win32 surface");
//	}
//
//	//get available queue family properties
//	uint32_t queueCount;
//	vkGetPhysicalDeviceQueueFamilyProperties(iphysicalDevice, &queueCount, nullptr);
//	assert(queueCount >= 1 && "Insufficient number of queues");
//
//	std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(iphysicalDevice, &queueCount, queueProperties.data());
//
//
//}
//#endif
//
//void VulkanSwapChain::connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
//
//}
//
//void VulkanSwapChain::create(uint32_t* width, uint32_t* height, bool vsync, bool fullscreen) {
//
//}
//
//VkResult VulkanSwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) {
//
//}
//
//VkResult VulkanSwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
//
//}
//
//void VulkanSwapChain::cleanup() {
//
//}
//
