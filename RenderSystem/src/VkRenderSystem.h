#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include <vector>
#include <unordered_map>
#include <optional>
#include <MakeID.h>
#include "rsids.h"
#include "rsenums.h"
#include "rsexporter.h"

/**
 * @brief VkRenderSystem is an API facade that allows you to describe rendering constructs at a high level and is re-usable for different purposes and different places of the code. This is the heart of the rendering engine that provides abstraction over a view, geometry , materials etc. It is singleton and can be  anywhere by the client. This engine internally manages all memory allocations and provides a way to interact with resources create via IDs. Client is expected to follow CRUD (Creation, Retrieval, Update and Disposal) of the resources via the IDs. The underlying engine uses Vulkan.
 */
class VkRenderSystem final
{

private:
    RSinitInfo iinitInfo;
    VkRSinstance iinstance;
    
    using RSviews = std::unordered_map<RSviewID ,VkRSview, IDHasher<RSviewID>>;
    using RScontexts = std::unordered_map<RScontextID, VkRScontext, IDHasher<RScontextID>>;
    using RScollections = std::unordered_map<RScollectionID, VkRScollection, IDHasher<RScollectionID>> ;
    using RSgeometryDataMaps = std::unordered_map<RSgeometryDataID, VkRSgeometryData, IDHasher<RSgeometryDataID>> ;
    using RSgeometries = std::unordered_map<RSgeometryID, VkRSgeometry, IDHasher<RSgeometryID>>;
    using RStextures = std::unordered_map<RStextureID, VkRStexture, IDHasher<RStextureID>>;
    using RSappearances = std::unordered_map<RSappearanceID, VkRSappearance, IDHasher<RSappearanceID>>;
    using RSspatials = std::unordered_map<RSspatialID, VkRSspatial, IDHasher<RSspatialID>>;
    using RSstates = std::unordered_map<RSstateID, VkRSstate, IDHasher<RSstateID>>;
    
    bool iisRSinited = false;
    MakeID iviewIDpool = MakeID(MAX_IDS);
    MakeID ictxIDpool = MakeID(MAX_IDS);
    MakeID igeomDataIDpool = MakeID(MAX_IDS);
    MakeID iinstanceIDpool = MakeID(MAX_IDS);
    MakeID igeometryIDpool = MakeID(MAX_IDS);
    MakeID itextureIDpool = MakeID(MAX_IDS);
    MakeID istateIDpool = MakeID(MAX_IDS);
    MakeID icollIDpool = MakeID(MAX_IDS);
    MakeID iappearanceIDpool = MakeID(MAX_IDS);
    MakeID ispatialIDpool = MakeID(MAX_IDS);
    
    RSviews iviewMap;
    RScontexts ictxMap;
    RScollections icollectionMap;
    RSgeometryDataMaps igeometryDataMap;
    RSgeometries igeometryMap;
    RStextures itextureMap;
    RSappearances iappearanceMap;
    RSspatials ispatialMap;
    RSstates istateMap;
    
    std::unordered_map<RSshaderTemplate, VkRSshader> ishaderModuleMap;
    RSspatialID _identitySpatialID;
    void pickPhysicalDevice();
    
    VkRSqueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR& vksurface);
    
    void createLogicalDevice(const VkSurfaceKHR& vksurface);
    
    void printPhysicalDeviceInfo(VkPhysicalDevice device);
    
    bool checkValidationLayerSupport() const;
    
    std::vector<const char*> getRequiredExtensions(const RSinitInfo& info) const;
    
    void populateInstanceData(VkRSinstance& inst, const RSinitInfo& info);
    
     void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
    
      void createInstance(const RSinitInfo& info);
     void setupDebugMessenger();
    
#if defined(_WIN32)
    VkSurfaceKHR createDummySurface(const HWND hwnd, const HINSTANCE hinst);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    VkSurfaceKHR createDummySurface(const void* parentSurface);
#endif
    
    
    void disposeDummySurface(const VkSurfaceKHR vksurface);
    void cleanupSwapChain(VkRSview& view);
    void createImageViews(VkRSview& view);
     void createSwapChain(VkRSview& view, VkRScontext& ctx);
     void createFramebuffers(VkRSview& view);
    
     void viewCreateDescriptorSets(VkRSview& view);
    
     void createUniformBuffers(VkRSview& view);
    
     void updateUniformBuffer(VkRSview& view, uint32_t currentFrame);
    
     void createDepthResources(VkRSview& view);
    
      VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
     VkFormat findDepthFormat();
    
     bool hasStencilComponent(VkFormat format);
     void appearanceCreateDescriptorSetLayout(VkRSappearance& vkrsapp);

    void createVolumeSliceUniformBuffers(VkRSappearance& vkrsapp);
    void updateVolumeSliceUniformBuffers(VkRSappearance& vkrsapp);
    void appearanceCreateDescriptorSet(VkRSappearance& vkrsapp, const VkDescriptorPool& descriptorPool);
    void createCommandBuffers(VkRSview& view);
    VkRSswapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, const VkSurfaceKHR& vksurface);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, RSuint surfaceWidth, RSuint surfaceHeight);
    void createSurface(VkRScontext& vkrsctx);
    void createSyncObjects(VkRScontext& ctx);
    void createCommandPool();
     void recordCommandBuffer(const RScollectionID* collections, uint32_t numCollections, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame);
    void disposeContext(VkRScontext& ctx);
     void recreateSwapchain(VkRScontext& ctx, VkRSview& view);
    void disposeView(VkRSview& view);
    VkImageView createImageView(VkImage image, VkImageViewType imageViewType, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, uint32_t depth, VkImageType imageType, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &imageMemory);
     bool createTextureImage(VkRStexture& vkrstex);
    VkFormat getDefaultTextureFormat(const RStextureFormat& texformat);
    VkImageViewType getImageViewType(const RStextureType& textype);
    VkImageType getImageType(const RStextureType& textype);
    uint32_t getTexelSize(const RStextureType& textype, const RStextureFormat& texformat);
    void createTextureImageView(VkRStexture& vkrstex);
    void createTextureSampler(VkRStexture& vkrstex);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth, const RSvolumeChunk& volchunk);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkRSshader createShaderModule(const RSshaderTemplate shaderTemplate);
    static std::vector<char> readFile(const std::string& filename, unsigned int openmode);
     void createRenderpass(VkRSview& view);
     void createGraphicsPipeline(VkRScollection& collection, VkRScollectionInstance& collinst, VkRSdrawCommand& drawcmd);
    void contextDrawCollections(VkRScontext& ctx, VkRSview& view, const RScollectionID* collections, uint32_t numCollections);
     void disposeCollection(VkRScollection& collection);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    std::vector<VkVertexInputBindingDescription> getBindingDescription(const RSvertexAttribsInfo& attribInfo);
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(const RSvertexAttribsInfo& attribInfo);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createDescriptorPool(VkRSview& vkrsview);
    void createAppearanceDescriptorPool();
     VkPrimitiveTopology getPrimitiveType(const RSprimitiveType& ptype);
    VkCompareOp getDepthCompare(const RSdepthFunction depthFunc);
    
public:
    static RS_EXPORT VkRenderSystem& getInstance()
    {
        static VkRenderSystem INSTANCE;
        
        return INSTANCE;
    }
    
    RS_EXPORT VkRSinstance getVkInstanceData();
    
    RS_EXPORT RSresult renderSystemInit(const RSinitInfo& info);
    
    RS_EXPORT bool isRenderSystemInited();
    
    RS_EXPORT RSresult renderSystemDispose();
    
    RS_EXPORT bool viewAvailable(const RSviewID& viewID) const;
    

    RS_EXPORT RSresult viewCreate(RSviewID& outViewID, const RSview& view, const RScontextID& associatedContextID);
    RS_EXPORT RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
    RS_EXPORT void viewHideInstance(const RSviewID& viewID, const RScollectionID& collectionID, const RSinstanceID& instanceID, bool hide);
    RS_EXPORT std::optional<RSview> viewGetData(const RSviewID& viewID);
    RS_EXPORT RSresult viewDispose(const RSviewID& viewID);
    RS_EXPORT bool contextAvailable(const RScontextID& ctxID) const;
    RS_EXPORT RSresult contextCreate(RScontextID& outCtxID, const RScontextInfo& info);
    RS_EXPORT glm::ivec2 contextGetDimensions(const RScontextID& ctxID);
    RS_EXPORT std::optional<RScontextInfo> contextGetData(const RScontextID& ctxID);
    RS_EXPORT RSresult contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID, const RScollectionID* collectionIDs, const uint32_t numCollections);
    RS_EXPORT RSresult contextDispose(const RScontextID& ctxID);
    RS_EXPORT void contextResized(const RScontextID& ctxID, const RSviewID& viewID, uint32_t newWidth, uint32_t newHeight);
    RS_EXPORT bool geometryDataAvailable(const RSgeometryDataID& geomDataID);
    RS_EXPORT RSresult geometryDataCreate(RSgeometryDataID& outgdataID, uint32_t numVertices, uint32_t numIndices, const RSvertexAttribsInfo attributesInfo);
    RS_EXPORT RSresult geometryDataUpdateInterleavedVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data);
    RS_EXPORT RSresult geometryDataUpdateVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, RSvertexAttribute attrib, void* data);
    RS_EXPORT RSresult geometryDataUpdateIndices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data);
    RS_EXPORT RSresult geometryDataFinalize(const RSgeometryDataID& gdataID);
    RS_EXPORT RSresult geometryDataDispose(const RSgeometryDataID& gdataID);
    RS_EXPORT bool geometryAvailable(const RSgeometryID& geomID);
    RS_EXPORT RSresult geometryCreate(RSgeometryID& outgeomID, const RSgeometryInfo& geomInfo);
    RS_EXPORT RSresult geometryDispose(const RSgeometryID& geomID);
    RS_EXPORT bool collectionAvailable(const RScollectionID& colID);
    RS_EXPORT bool collectionInstanceAvailable(const RScollectionID& collID, const RSinstanceID& instanceID);
    RS_EXPORT RSresult collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo);
    RS_EXPORT RSresult collectionInstanceCreate(const RScollectionID& collID, RSinstanceID& instID, const RSinstanceInfo& instInfo);
    RS_EXPORT void collectionInstanceHide(const RScollectionID& collID, const RSinstanceID& instID, bool hide);
    RS_EXPORT RSresult collectionInstanceDispose(const RScollectionID& collID, RSinstanceID& instID);
    RS_EXPORT RSresult collectionFinalize(const RScollectionID& colID);
    RS_EXPORT RSresult collectionDispose(RScollectionID& colID);
    RS_EXPORT bool appearanceAvailable(const RSappearanceID& appID) const;
    RS_EXPORT RSresult appearanceCreate(RSappearanceID& outAppID, const RSappearanceInfo& appInfo);

    RS_EXPORT bool appearanceUpdateVolumeSlice(const RSappearanceID& appID, const RSvolumeSliceAppearance& appInfo);
    RS_EXPORT RSresult appearanceDispose(const RSappearanceID& appID);
    RS_EXPORT bool spatialAvailable(const RSspatialID& spatialID);
    RS_EXPORT RSresult spatialCreate(RSspatialID& outSplID, const RSspatial& splInfo);
    RS_EXPORT bool spatialSetData(RSspatialID& spatialID, const RSspatial& spatial);
    RS_EXPORT RSresult spatialDispose(const RSspatialID& spatialID);
    RS_EXPORT bool textureAvailable(const RStextureID& texID);
    RS_EXPORT RSresult textureCreate(RStextureID& outTexID, const char* absfilepath);
    RS_EXPORT RSresult texture3dCreate(RStextureID& outTexID, const RStextureInfo& texInfo);
    RS_EXPORT RSresult textureCreateFromMemory(RStextureID& outTexID, unsigned char* encodedTexData, uint32_t width, uint32_t height);
    RS_EXPORT RSresult textureDispose(const RStextureID& texID);

    RS_EXPORT bool stateAvailable(const RSstateID& stateID);
    RS_EXPORT RSresult stateCreate(RSstateID& outStateID, const RSstate& state);
    
    RS_EXPORT RSresult stateDispose(const RSstateID& stateID);
};
