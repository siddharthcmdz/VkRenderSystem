#pragma once

struct VkRSdrawCommand
{
    RSvertexAttributeSettings attribSetting;
    std::vector<VkBuffer> vertexBuffers{};
    VkBuffer indicesBuffer = VK_NULL_HANDLE;
    bool isIndexed = true;
    uint32_t numIndices = 0;
    uint32_t numVertices = 0;
    VkDeviceSize vertexOffset = 0;
    VkDeviceSize indicesOffset = 0;
    RSappearanceID appID;
    
    VkPrimitiveTopology primTopology;
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};
    RSspatialID spatialID;
    RSstate state;
};
