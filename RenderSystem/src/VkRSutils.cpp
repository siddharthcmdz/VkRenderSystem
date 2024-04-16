
#include "VkRSutils.h"
#include <stdexcept>

VkExtent2D VkRSutils::getDefaultViewExtent()
{
    return VkExtent2D {800, 600};
}

VkFormat VkRSutils::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, const VkPhysicalDevice& physicalDevice)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VkRSutils::findDepthFormat(const VkPhysicalDevice& physicalDevice)
{
    return findSupportedFormat(
           {
               VK_FORMAT_D32_SFLOAT,
               VK_FORMAT_D32_SFLOAT_S8_UINT,
               VK_FORMAT_D24_UNORM_S8_UINT
           },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
       physicalDevice
    );
}
