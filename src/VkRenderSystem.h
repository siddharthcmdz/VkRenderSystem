#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include <vector>
#include <unordered_map>
#include "MakeID.h"

class VkRenderSystem {

private:
	RSinitInfo iinitInfo;
	VkRSinstance iinstance;
	std::unordered_map<uint32_t ,VkRSview> iviewMap;
	std::unordered_map<uint32_t, VkRScontext> ictxMap;
	std::unordered_map<uint32_t, VkRScollection> icollectionMap;
	MakeID iviewIDpool = MakeID(MAX_IDS);
	MakeID ictxIDpool = MakeID(MAX_IDS);
	MakeID icollIDpool = MakeID(MAX_IDS);
	bool iisRSinited = false;

	//context related helpers
	bool isDeviceSuitable(VkPhysicalDevice device, VkRScontext& ctx);
	VkRSswapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkRScontext& ctx);
	VkRSqueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkRScontext& ctx);
	void createSurface(VkRScontext& vkrsctx);
	void setPhysicalDevice(VkRScontext& ctx);
	void createLogicalDevice(VkRScontext& ctx);
	void createSwapChain(VkRScontext& ctx);
	void createImageViews(VkRScontext& ctx);
	void disposeContext(VkRScontext& ctx);

	//rs global helpers
	void printPhysicalDeviceInfo(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions(const RSinitInfo& info) const;
	void populateInstanceData(VkRSinstance& inst, const RSinitInfo& info);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	void createInstance(const RSinitInfo& info);
	void setupDebugMessenger();

	//collection related helpers
	void createRenderpass(VkRScollection& collection, const VkRScontext& ctx);
	void createGraphicsPipeline(VkRScollection& collection, const VkRScontext& ctx);
	void createCommandPool(VkRScollection& collection, const VkRScontext& ctx);
	void createCommandBuffers(VkRScollection& collection, const VkRScontext& ctx);
	void createSyncObjects(VkRScollection& collection, const VkRScontext& ctx);
	void recordCommandBuffer(const VkRScollection& collection, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame);
	VkShaderModule createShaderModule(const std::vector<char>& code, const VkDevice& device);
	static std::vector<char> readFile(const std::string& filename);
	void contextDrawCollection(const VkRScontext& context, VkRSview& view, const VkRScollection& collection);
	void disposeCollection(VkRScollection& collection);

	//view related helpers
	void createFramebuffers(VkRSview& view, const VkRScontext& ctx, const VkRenderPass& renderPass);
	void disposeView(VkRSview& view);

public:
	static VkRenderSystem& getInstance() {
		static VkRenderSystem INSTANCE;

		return INSTANCE;
	}

	RSresult renderSystemInit(const RSinitInfo& info);
	bool isRenderSystemInit();
	RSresult renderSystemDispose();

	bool contextAvailable(const RScontextID& ctxID) const;
	RSresult contextCreate(RScontextID& outCtxID, const RScontextInfo& info);
	RSresult contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID);
	RSresult contextDispose(const RScontextID& ctxID);
	
	bool viewAvailable(const RSviewID& viewID) const;
	RSresult viewCreate(RSviewID& viewID, const RSview& view);
	//RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
	RSresult viewAddCollection(const RSviewID& viewID, const RScollectionID& colID);
	RSresult viewRemoveCollection(const RSviewID& viewID, const RScollectionID& colID);
	RSresult viewFinalize(const RSviewID& viewID);
	RSresult viewDispose(const RSviewID& viewID);

	bool collectionAvailable(const RScollectionID& colID);
	RSresult collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo);
	RSresult collectionFinalize(const RScollectionID& colID, const RScontextID& ctxID);
	RSresult collectionDispose(const RScollectionID& colID);
};