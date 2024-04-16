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
#include <unordered_map>
#include "rsenums.h"

/**
 * @brief A factory for creating vulkan constructs that needs to be created on the fly
 */
class VkRSfactory final
{
private:
    static std::unordered_map<DescriptorLayoutType, VkDescriptorSetLayout> _descriptorLayoutMap;
    static std::unordered_map<RenderPassType, VkRenderPass> _renderPassMap;
    
    static void createViewDefaultDescriptorLayout();
    static void createViewSimpleRenderPass();
    
public:
    /**
     * @brief Creates and gets a cached vulkan descriptor set layout
     * @param dlt the specified descriptor layout type
     */
    static VkDescriptorSetLayout getDescriptorLayout(const DescriptorLayoutType& dlt);
    
    /**
     * @brief Disposes all the descriptor set layouts cached
     */
    static void disposeAllDescriptorLayouts();
    
    /**
     * @brief Creates and gets a cached vulkan renderpass
     * @return rpt the specified vulkan render pass
     */
    static VkRenderPass getRenderPass(const RenderPassType& rpt);
    
    /**
     * @brief Disposes all the render passes.
     */
    static void disposeAllRenderPasses();
};
