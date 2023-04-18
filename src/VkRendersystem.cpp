#include "VkRenderSystem.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
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
	for (const auto& ext : info.requiredExtensions) {
		extensions.push_back(ext);
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

void VkRenderSystem::initInstance(const RSinitInfo& info) {
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

RSresult VkRenderSystem::renderSystemInit(const RSinitInfo& info)
{
	initInstance(info);
	return RSresult::SUCCESS;
}

bool VkRenderSystem::isRenderSystemInit()
{
	return false;
}

RSresult VkRenderSystem::renderSystemDispose()
{
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewCreate(const RSview& view, RSviewID& viewID)
{
	RSuint id;
	bool success = iviewIDpool.CreateID(id);
	assert(success && "failed to create a view ID");
	if (success && viewID.isValid()) {
		iviewMap[id] = view;
		viewID.id = id;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewUpdate(const RSviewID& viewID, const RSview& view)
{
	if (viewID.isValid()) {
		if (iviewMap.find(viewID.id) != iviewMap.end()) {
			iviewMap[viewID.id] = view;
			iviewMap[viewID.id].dirty = true;

			return RSresult::SUCCESS;
		}
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewDraw(const RSviewID& viewID)
{
	return RSresult::SUCCESS;
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	return RSresult::SUCCESS;
}
