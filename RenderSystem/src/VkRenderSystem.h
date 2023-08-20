#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include <vector>
#include <unordered_map>
#include "MakeID.h"
#include "rsids.h"
#include "rsenums.h"
#include "rsexporter.h"

class VkRenderSystem {

private:
	RSinitInfo iinitInfo;
	VkRSinstance iinstance;
	using RSviews = std::unordered_map<RSviewID ,VkRSview, IDHasher<RSviewID>>;
	using RScontexts = std::unordered_map<RScontextID, VkRScontext, IDHasher<RScontextID>> ;
	using RScollections = std::unordered_map<RScollectionID, VkRScollection, IDHasher<RScollectionID>> ;
	using RSgeometryDataMaps = std::unordered_map<RSgeometryDataID, VkRSgeometryData, IDHasher<RSgeometryDataID>> ;
	using RSgeometries = std::unordered_map<RSgeometryID, VkRSgeometry, IDHasher<RSgeometryID>>;
	using RStextures = std::unordered_map<RStextureID, VkRStexture, IDHasher<RStextureID>>;
	using RSappearances = std::unordered_map<RSappearanceID, VkRSappearance, IDHasher<RSappearanceID>>;
	using RSspatials = std::unordered_map<RSspatialID, VkRSspatial, IDHasher<RSspatialID>>;

	MakeID iviewIDpool = MakeID(MAX_IDS);
	MakeID ictxIDpool = MakeID(MAX_IDS);
	MakeID icollIDpool = MakeID(MAX_IDS);
	MakeID igeomDataIDpool = MakeID(MAX_IDS);
	MakeID iinstanceIDpool = MakeID(MAX_IDS);
	MakeID igeometryIDpool = MakeID(MAX_IDS);
	MakeID itextureIDpool = MakeID(MAX_IDS);
	MakeID istateIDpool = MakeID(MAX_IDS);
	MakeID iappearanceIDpool = MakeID(MAX_IDS);
	MakeID ispatialIDpool = MakeID(MAX_IDS);

	bool iisRSinited = false;

	RSviews iviewMap;
	RScontexts ictxMap;
	RScollections icollectionMap;
	RSgeometryDataMaps igeometryDataMap;
	RSgeometries igeometryMap;
	RStextures itextureMap;
	RSappearances iappearanceMap;
	RSspatials ispatialMap;

	//context related helpers
	bool isDeviceSuitable(VkPhysicalDevice device, const VkSurfaceKHR& vksurface);
	VkRSswapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, const VkSurfaceKHR& vksurface);
	VkRSqueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR& vksurface);
	void setPhysicalDevice(const VkSurfaceKHR& vksurface);
	void createLogicalDevice(const VkSurfaceKHR& vksurface);
	void createSurface(VkRScontext& vkrsctx);
	void createSwapChain(VkRScontext& ctx);
	void createImageViews(VkRScontext& ctx);
	void createCommandPool(const VkRScontext& ctx);
	void createSyncObjects(VkRScontext& ctx);
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
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createDescriptorPool();

	//collection related helpers
	void createRenderpass(VkRScollection& collection, const VkRScontext& ctx);
	void createGraphicsPipeline(const VkRScontext& ctx, const VkRSview& view, VkRScollection& collection, VkRScollectionInstance& collinst, VkRSdrawCommand& drawcmd);
	void createCommandBuffers(VkRScollection& collection, const VkRScontext& ctx);
	void recordCommandBuffer(const VkRScollection& collection, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> readFile(const std::string& filename);
	void contextDrawCollection(VkRScontext& context, VkRSview& view, const VkRScollection& collection);
	void disposeCollection(VkRScollection& collection);

	//collection instance related helpers
	void collectionInstanceCreateDescriptorSetLayout(VkRScollectionInstance& inst);
	void collectionInstanceCreateDescriptorSet(VkRScollectionInstance& inst);
	
	//view related helpers
	void createFramebuffers(VkRSview& view, const VkRScontext& ctx, const VkRenderPass& renderPass);
	void viewCreateDescriptorSetLayout(VkRSview& view);
	void viewCreateDescriptorSets(VkRSview& view);
	void createUniformBuffers(VkRSview& view);
	void updateUniformBuffer(VkRSview& view, VkRScontext& ctx, uint32_t currentFrame);
	void disposeView(VkRSview& view);

	//geometry data related helpers
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkVertexInputBindingDescription getBindingDescription(const RSvertexAttribsInfo& attribInfo);
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(const RSvertexAttribsInfo& attribInfo);

	//texture related
	bool createTextureImage(VkRStexture& vkrstex);
	void createTextureImageView(VkRStexture& vkrstex);
	void createTextureSampler(VkRStexture& vkrstex);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkImageView createImageView(VkImage image, VkFormat format);

public:
	RS_EXPORT static VkRenderSystem& getInstance() {
		static VkRenderSystem INSTANCE;

		return INSTANCE;
	}

	RS_EXPORT RSresult renderSystemInit(const RSinitInfo& info);
	RS_EXPORT bool isRenderSystemInit();
	RS_EXPORT RSresult renderSystemDispose();

	RS_EXPORT bool contextAvailable(const RScontextID& ctxID) const;
	RS_EXPORT RSresult contextCreate(RScontextID& outCtxID, const RScontextInfo& info);
	RS_EXPORT RSresult contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID);
	RS_EXPORT RSresult contextDispose(const RScontextID& ctxID);
	RS_EXPORT void contextResizeSet(const RScontextID& ctxID, bool onOff);
	
	RS_EXPORT bool viewAvailable(const RSviewID& viewID) const;
	RS_EXPORT RSresult viewCreate(RSviewID& viewID, const RSview& view);
	//RS_EXPORT RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
	RS_EXPORT RSresult viewAddCollection(const RSviewID& viewID, const RScollectionID& colID);
	RS_EXPORT RSresult viewRemoveCollection(const RSviewID& viewID, const RScollectionID& colID);
	RS_EXPORT RSresult viewFinalize(const RSviewID& viewID);
	RS_EXPORT RSresult viewDispose(const RSviewID& viewID);

	RS_EXPORT bool collectionAvailable(const RScollectionID& colID);
	RS_EXPORT bool collectionInstanceAvailable(const RScollectionID& collID, const RSinstanceID& instanceID);
	RS_EXPORT RSresult collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo);
	RS_EXPORT RSresult collectionInstanceCreate(RScollectionID& collID, RSinstanceID& instID, const RSinstanceInfo& instInfo);
	RS_EXPORT RSresult collectionInstanceDispose(RScollectionID& collID, RSinstanceID& instID);
	RS_EXPORT RSresult collectionFinalize(const RScollectionID& colID, const RScontextID& ctxID, const RSviewID& viewID);
	RS_EXPORT RSresult collectionDispose(const RScollectionID& colID);

	RS_EXPORT bool geometryDataAvailable(const RSgeometryDataID& geomDataID);
	RS_EXPORT RSresult geometryDataCreate(RSgeometryDataID& outgdataID, uint32_t numVertices, uint32_t numIndices, const RSvertexAttribsInfo attributesInfo, RSbufferUsageHints bufferUsage);
	RS_EXPORT RSresult geometryDataUpdateVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data);
	RS_EXPORT RSresult geometryDataUpdateIndices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data);
	RS_EXPORT RSresult geometryDataFinalize(const RSgeometryDataID& gdataID);
	RS_EXPORT RSresult geometryDataDispose(const RSgeometryDataID& gdataID);

	RS_EXPORT bool geometryAvailable(const RSgeometryID& geomID);
	RS_EXPORT RSresult geometryCreate(RSgeometryID& outgeomID, const RSgeometryInfo& geomInfo);
	RS_EXPORT RSresult geometryDispose(const RSgeometryID& geomID);

	RS_EXPORT bool textureAvailable(const RStextureID& texID);
	RS_EXPORT RSresult textureCreate(RStextureID& outTexID, const char* absfilepath);
	RS_EXPORT RSresult textureDispose(const RStextureID& texID);

	RS_EXPORT bool appearanceAvailable(const RSappearanceID& appID);
	RS_EXPORT RSresult appearanceCreate(RSappearanceID& outAppID, const RSappearanceInfo& appInfo);
	RS_EXPORT RSresult appearanceDispose(const RSappearanceID& appID);

	RS_EXPORT bool spatialAvailable(const RSspatialID& spatialID);
	RS_EXPORT RSresult spatialCreate(RSspatialID& outSplID, const RSspatial& splInfo);
	RS_EXPORT RSresult spatialDispose(const RSspatialID& spatialID);
	
};