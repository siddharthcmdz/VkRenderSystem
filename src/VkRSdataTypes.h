#pragma once

#include <vulkan/vulkan.h>
#include "RSdataTypes.h"

struct RSinstance {
	VkInstance iinstance;
	VkDebugUtilsMessengerEXT idebugMessenger;
	VkPhysicalDevice iphysicalDevice = VK_NULL_HANDLE;
	VkDevice idevice;
};

struct VkRScontext {
	VkSurfaceKHR isurface;
};

struct VkRSview {
	RSview view;

};