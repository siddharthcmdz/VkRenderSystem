//// =================================================================================
// Copyright (c) 2023, See All Surgical Inc. All rights reserved.
//
// This software is the property of See All Surgical Inc. The software may not be reproduced,
// modified, distributed, or transferred without the express written permission of See All Surgical Inc.
//
// In no event shall See All Surgical Inc. be liable for any claim, damages or other liability,
// whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software
// or the use or other dealings in the software.
// =================================================================================

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

