
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkRSdataTypes.h"

/**
 * @brief A utility class for vulkan objects
 */
class VkRSutils final
{
private:
    /**
     * @brief Utility method to find the supported format of a depth or color images.
     * @param candidates the specified supportable vk formats
     * @param tiling the specified optimal tiling for images
     * @param features  the specified format feature flags
     * @return the valid and optimal format that vk supports.
     */
    static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, const VkPhysicalDevice& physicalDevice);

public:
    
    /**
     * @brief Gets the default view extents
     * @return the default view extents
     */
    static VkExtent2D getDefaultViewExtent();
    
    /**
     * @brief Utility method that identifies a depth format - support for depth individually or depth + stencil format.
     * @return the valid and optimal format returned by vk.
     */
    static VkFormat findDepthFormat(const VkPhysicalDevice& physicalDevice);
};

