#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include <vector>
#include <unordered_map>
#include "MakeID.h"
#include "rsids.h"
#include "rsenums.h"

class VkRenderSystem {

private:
	RSinitInfo iinitInfo;
	VkRSinstance iinstance;
	using RSviews = std::unordered_map<RSviewID ,VkRSview, IDHasher<RSviewID>>;
	using RScontexts = std::unordered_map<RScontextID, VkRScontext, IDHasher<RScontextID>> ;
	using RScollections = std::unordered_map<RScollectionID, VkRScollection, IDHasher<RScollectionID>> ;
	using RSgeometryDataMaps = std::unordered_map<RSgeometryDataID, VkRSgeometryData, IDHasher<RSgeometryDataID>> ;

	MakeID iviewIDpool = MakeID(MAX_IDS);
	MakeID ictxIDpool = MakeID(MAX_IDS);
	MakeID icollIDpool = MakeID(MAX_IDS);
	MakeID igeomDataIDpool = MakeID(MAX_IDS);
	MakeID iinstanceIDpool = MakeID(MAX_IDS);

	bool iisRSinited = false;

	RSviews iviewMap;
	RScontexts ictxMap;
	RScollections icollectionMap;
	RSgeometryDataMaps igeometryDataMap;

	//context related helpers
	bool isDeviceSuitable(VkPhysicalDevice device, VkRScontext& ctx);
	VkRSswapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkRScontext& ctx);
	VkRSqueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkRScontext& ctx);
	void createSurface(VkRScontext& vkrsctx);
	void setPhysicalDevice(VkRScontext& ctx);
	void createLogicalDevice(VkRScontext& ctx);
	void createSwapChain(VkRScontext& ctx);
	void createImageViews(VkRScontext& ctx);
	void createCommandPool(const VkRScontext& ctx);
	void disposeContext(VkRScontext& ctx);
	void cleanupSwapChain(VkRScontext& ctx, VkRSview& view);

	//rs global helpers
	void printPhysicalDeviceInfo(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions(const RSinitInfo& info) const;
	void populateInstanceData(VkRSinstance& inst, const RSinitInfo& info);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	void createInstance(const RSinitInfo& info);
	void setupDebugMessenger();
	void recreateSwapchain(VkRScontext& ctx, VkRSview& view, const VkRenderPass& renderpass);
	static void framebufferResizeCallback(GLFWwindow* window, int widht, int height);

	//collection related helpers
	void createRenderpass(VkRScollection& collection, const VkRScontext& ctx);
	void createGraphicsPipeline(VkRScollection& collection, const VkRScontext& ctx);
	void createCommandBuffers(VkRScollection& collection, const VkRScontext& ctx);
	void createSyncObjects(VkRScollection& collection, const VkRScontext& ctx);
	void recordCommandBuffer(const VkRScollection& collection, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame);
	VkShaderModule createShaderModule(const std::vector<char>& code, const VkDevice& device);
	static std::vector<char> readFile(const std::string& filename);
	void contextDrawCollection(VkRScontext& context, VkRSview& view, const VkRScollection& collection);
	void disposeCollection(VkRScollection& collection);

	//view related helpers
	void createFramebuffers(VkRSview& view, const VkRScontext& ctx, const VkRenderPass& renderPass);
	void createDescriptorSetLayout(VkRSview& view);
	void createDescriptorPool(VkRSview& view);
	void createDescriptorSet(VkRSview& view);
	void createUniformBuffers(VkRSview& view);
	void updateUniformBuffer(VkRSview& view, VkRScontext& ctx, uint32_t currentFrame);
	void disposeView(VkRSview& view);

	//geometry data related helpers
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkVertexInputBindingDescription getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

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
	void contextResizeSet(const RScontextID& ctxID, bool onOff);
	
	bool viewAvailable(const RSviewID& viewID) const;
	RSresult viewCreate(RSviewID& viewID, const RSview& view);
	//RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
	RSresult viewAddCollection(const RSviewID& viewID, const RScollectionID& colID);
	RSresult viewRemoveCollection(const RSviewID& viewID, const RScollectionID& colID);
	RSresult viewFinalize(const RSviewID& viewID);
	RSresult viewDispose(const RSviewID& viewID);

	bool collectionAvailable(const RScollectionID& colID);
	bool collectionInstanceAvailable(const RScollectionID& collID, const RSinstanceID& instanceID);
	RSresult collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo);
	RSresult collectionInstanceCreate(RScollectionID& collID, RSinstanceID& instID, const RSinstanceInfo& instInfo);
	RSresult collectionInstanceDispose(RScollectionID& collID, RSinstanceID& instID);
	RSresult collectionFinalize(const RScollectionID& colID, const RScontextID& ctxID);
	RSresult collectionDispose(const RScollectionID& colID);

	bool geometryDataAvailable(const RSgeometryDataID& geomDataID);
	RSresult geometryDataCreate(RSgeometryDataID& outgdataID, uint32_t numVertices, uint32_t numIndices, const RSvertexAttribsInfo attributesInfo, RSbufferUsageHints bufferUsage);
	RSresult geometryDataUpdateVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data);
	RSresult geometryDataUpdateIndices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data);
	RSresult geometryDataFinalize(const RSgeometryDataID& gdataID);
	RSresult geometryDataDispose(const RSgeometryDataID& gdataID);


};