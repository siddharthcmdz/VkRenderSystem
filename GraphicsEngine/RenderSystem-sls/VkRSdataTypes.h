#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "RSdataTypes.h"

#include <string>
#include <optional>
#include <array>
#include <unordered_set>
#include "rsenums.h"
#include "DrawCommand.h"
#include "TextureLoader.h"
#include "VkRSbuffer.h"

struct VkRSqueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() 
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct VkRSinstance 
{
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
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkCommandPool commandPool{}; //manages the memory where command buffers are allocated from them
    VkRSqueueFamilyIndices queueFamilyIndices;
    VkSurfaceFormatKHR surfaceFormat;
    
    VkDescriptorPool appearanceDescriptorPool = VK_NULL_HANDLE;

    uint32_t majorVersion = ~0;
    uint32_t minorVersion = ~0;
    uint32_t patchVersion = ~0;
    uint32_t variantVersion = ~0;
    uint32_t maxUniformDescriptorSets = ~0;
    uint32_t maxCombinedSamplerDescriptorSets = ~0;
    uint32_t maxBoundDescriptorSets = ~0;
    VkDeviceSize maxMemoryAllocationSize{};
};

struct VkRSshader 
{
    VkShaderModule vert{};
    VkShaderModule frag{};
    std::string shadernName;
    std::string vertShaderContent;
    std::string fragShaderContent;
};

struct VkRScontext 
{
    static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    VkSurfaceKHR surface{};
    bool resized = false;
    uint32_t width = 0;
    uint32_t height = 0;
    RScontextInfo info;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
};


struct VkRSview 
{
    VkRenderPass renderPass{};
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkSwapchainKHR swapChain{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    std::vector<VkCommandBuffer> commandBuffers; //gets automatically disposed when command pool is disposed.
    std::unordered_set<RScollectionID, IDHasher<RScollectionID>> collectionIDlist;
    
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout{};
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    
    std::unordered_map<RScollectionID, std::vector<RSinstanceID>, IDHasher<RScollectionID>> hiddenInstances;

    RSview view;
    uint32_t currentFrame = 0;
};

struct VkRSswapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> presentModes{};
};

struct VkRStexture 
{
    RStextureInfo texinfo;
    std::string absPath;
    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;
};

struct VkRSinterleavedGeomBuffers 
{
    VkBuffer vaBuffer = VK_NULL_HANDLE;
    VkBuffer stagingVABuffer = VK_NULL_HANDLE;
    VkDeviceMemory vaBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory stagingVAbufferMemory = VK_NULL_HANDLE;
    void* mappedStagingVAPtr = nullptr;
};

struct VkRSseparateGeomBuffers 
{
    std::array<VkRSbuffer, 4> stagingBuffers;
    std::array<VkRSbuffer, 4> buffers;
};

struct VkRSindicesBuffers 
{
    VkBuffer indicesBuffer = VK_NULL_HANDLE;
    VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indicesBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory stagingIndexBufferMemory = VK_NULL_HANDLE;
    void* mappedIndexPtr = nullptr;
};

struct VkRSgeometryData 
{
    uint32_t numVertices = 0;
    uint32_t numIndices = 0;
    RSvertexAttribsInfo attributesInfo;
    
    VkRSinterleavedGeomBuffers interleaved;
    VkRSseparateGeomBuffers separate;
    VkRSindicesBuffers indices;
};

struct VkRSgeometry 
{
    RSgeometryInfo geomInfo;
};

struct VkRScollectionInstance 
{
    RSinstanceInfo instInfo;
    bool hide = false;
};

struct VkRScollection 
{
    RScollectionInfo info;
    
    using DrawCommands = std::unordered_map<RSinstanceID, VkRSdrawCommand, IDHasher<RSinstanceID>>;
    using RSinstances = std::unordered_map<RSinstanceID, VkRScollectionInstance, IDHasher<RSinstanceID>>;

    DrawCommands drawCommands;
    RSinstances instanceMap;
    bool dirty = true;
};

struct VkRSviewDescriptor 
{
    glm::mat4 view{};
    glm::mat4 proj{};
    glm::vec4 lightPos{};
};

struct VkRSvolumeSliceDescriptor
{
    glm::vec2 window;
    glm::vec2 rescale;
};

struct VkRSappearance 
{
    RSappearanceInfo appInfo;

    VkDescriptorSetLayout descriptorSetLayout{};
    std::vector<VkDescriptorSet> descriptorSets;

    //used for volume slices currently - but can be generatlized later when we have different appearance types
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
};

struct VkRSspatial 
{
    RSspatial spatial;
};

struct VkRSstate 
{
    RSstate state{};
};

VkFormat getVkFormat(const RSvertexAttribute& va);
uint32_t getOffset(uint32_t numAttribs, uint32_t idx);
