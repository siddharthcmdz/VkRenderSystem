#pragma once
#include <vulkan/vulkan.h>
#include <array>

struct VkRSdrawCommand {
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indicesBuffer = VK_NULL_HANDLE;
	bool isIndexed = true;
	uint32_t numIndices = 0;
	uint32_t numVertices = 0;
	VkDeviceSize vertexOffset = 0;
	VkDeviceSize indicesOffset = 0;
	VkPrimitiveTopology primTopology;
	VkPipelineLayout pipelineLayout{};
	VkPipeline graphicsPipeline{};
	std::array<VkDescriptorSet, 2> viewDescriptors;
	std::array<VkDescriptorSet, 2> materialDescriptors;
	std::array<VkDescriptorSet, 2> spatialDescriptors;
	bool hasMaterialDescriptors = false;
	RSspatial spatial;
};