#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include <vector>
#include <unordered_map>
#include "MakeID.h"

class VkRenderSystem {

private:
	bool irenderOnscreen = true;
	RSinitInfo iinitInfo;
	VkRSinstance iinstance;
	std::unordered_map<uint32_t ,VkRSview> iviewMap;
	std::unordered_map<uint32_t, VkRScontext> ictxMap;
	MakeID iviewIDpool = MakeID(MAX_IDS);
	MakeID ictxIDpool = MakeID(MAX_IDS);
	
	//context related helpers
	bool isDeviceSuitable(VkPhysicalDevice device, VkRScontext& ctx);
	VkRSswapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkRScontext& ctx);
	VkRSqueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkRScontext& ctx);
	void createSurface(VkRScontext& vkrsctx);
	void setPhysicalDevice(VkRScontext& ctx);
	void createLogicalDevice(VkRScontext& ctx);
	void createSwapChain(VkRScontext& ctx);
	void createImageViews(VkRScontext& ctx);

	//rs global helpers
	void printPhysicalDeviceInfo(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions(const RSinitInfo& info) const;
	void populateInstanceData(VkRSinstance& inst, const RSinitInfo& info);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	void createInstance(const RSinitInfo& info);
	void setupDebugMessenger();

public:
	static VkRenderSystem& getInstance() {
		static VkRenderSystem INSTANCE;

		return INSTANCE;
	}

	RSresult renderSystemInit(const RSinitInfo& info);
	bool isRenderSystemInit();
	void renderSystemDrawLoop(const RScontextID& ctxID, const RSviewID& viewID);
	RSresult renderSystemDispose();

	RSresult contextCreate(RScontextID& outCtxID, const RScontextInfo& info);
	RSresult contextDispose(const RScontextID& ctxID);
	RSresult viewCreate(RSviewID& viewID, const RSview& view);
	RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
	RSresult viewDispose(const RSviewID& viewID);
};