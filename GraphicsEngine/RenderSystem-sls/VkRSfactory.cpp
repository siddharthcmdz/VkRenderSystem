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

#include "VkRSfactory.h"
#include "VkRSutils.h"
#include <stdexcept>
#include "VkRenderSystem.h"

std::unordered_map<DescriptorLayoutType, VkDescriptorSetLayout> VkRSfactory::_descriptorLayoutMap;
std::unordered_map<RenderPassType, VkRenderPass> VkRSfactory::_renderPassMap;


void VkRSfactory::createViewDefaultDescriptorLayout()
{
    auto& vkrs = VkRenderSystem::getInstance();
    VkRSinstance vkrsinst = vkrs.getVkInstanceData();
    
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1; //all view related parameters\data are sourced by one buffer
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout dsl;
    VkResult res = vkCreateDescriptorSetLayout(vkrsinst.device, &layoutInfo, nullptr, &dsl);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create a descriptor set layout");
    }
    
    _descriptorLayoutMap[DescriptorLayoutType::dltViewDefault] = dsl;
}

VkDescriptorSetLayout VkRSfactory::getDescriptorLayout(const DescriptorLayoutType& dlt)
{

    VkDescriptorSetLayout vkdsl{};
    switch (dlt) {
        case DescriptorLayoutType::dltViewDefault:
            if(_descriptorLayoutMap.find(DescriptorLayoutType::dltViewDefault) == _descriptorLayoutMap.end())
            {
                createViewDefaultDescriptorLayout();
            }
            vkdsl = _descriptorLayoutMap.at(DescriptorLayoutType::dltViewDefault);
            break;
            
        case DescriptorLayoutType::dltInvalid:
        default:
            throw std::runtime_error("Invalid descriptor type");
            break;
    }
    
    return vkdsl;
}

void VkRSfactory::createViewSimpleRenderPass()
{
    auto& vkrs = VkRenderSystem::getInstance();
    VkRSinstance vkrsinst = vkrs.getVkInstanceData();
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vkrsinst.surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VkRSutils::findDepthFormat(vkrsinst.physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderpass{};
    const VkResult result = vkCreateRenderPass(vkrsinst.device, &renderPassInfo, nullptr, &renderpass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
    
    _renderPassMap[RenderPassType::rptViewSimple] = renderpass;
}

VkRenderPass VkRSfactory::getRenderPass(const RenderPassType& rpt)
{
    VkRenderPass vkrp;
    switch (rpt) 
    {
        case RenderPassType::rptViewSimple:
            if(_renderPassMap.find(RenderPassType::rptViewSimple) == _renderPassMap.end())
            {
                createViewSimpleRenderPass();
            }
            vkrp = _renderPassMap[RenderPassType::rptViewSimple];
            break;
            
        case RenderPassType::rptInvalid:
        default:
            throw std::runtime_error("failed to create render pass");
            break;
    }
    
    return vkrp;
}

void VkRSfactory::disposeAllDescriptorLayouts()
{
    auto& vkrs = VkRenderSystem::getInstance();
    VkRSinstance vkrsinst = vkrs.getVkInstanceData();
    
    for(const auto& iter : _descriptorLayoutMap)
    {
        const VkDescriptorSetLayout& dsl = iter.second;
        vkDestroyDescriptorSetLayout(vkrsinst.device, dsl, nullptr);
    }
    
    _descriptorLayoutMap.clear();
}

void VkRSfactory::disposeAllRenderPasses()
{
    auto& vkrs = VkRenderSystem::getInstance();
    VkRSinstance vkrsinst = vkrs.getVkInstanceData();

    for(const auto& iter : _renderPassMap)
    {
        const VkRenderPass& renderpass = iter.second;
        vkDestroyRenderPass(vkrsinst.device, renderpass, nullptr);
    }
}
