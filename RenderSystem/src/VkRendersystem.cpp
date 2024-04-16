#include "VkRenderSystem.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "RSVkDebugUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include "VkRSutils.h"
#include "VkRSfactory.h"

RSresult VkRenderSystem::renderSystemInit(const RSinitInfo& info)
{
    if(iisRSinited) 
    {
        return RSresult::FAILURE;
    }
    
    iinitInfo = info;

    createInstance(info);
    setupDebugMessenger();

    pickPhysicalDevice();
#if defined(_WIN32)
    VkSurfaceKHR dummySurface = createDummySurface(info.parentHwnd, info.parentHinst);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    VkSurfaceKHR dummySurface = createDummySurface(info.parentView);
#endif

    createLogicalDevice(dummySurface);
    createCommandPool();
    disposeDummySurface(dummySurface);
    
    createAppearanceDescriptorPool();
    
    ishaderModuleMap[RSshaderTemplate::stOneTriangle] = createShaderModule(RSshaderTemplate::stOneTriangle);
    ishaderModuleMap[RSshaderTemplate::stPassthrough] = createShaderModule(RSshaderTemplate::stPassthrough);
    ishaderModuleMap[RSshaderTemplate::stSimpleLit]  = createShaderModule(RSshaderTemplate::stSimpleLit);
    ishaderModuleMap[RSshaderTemplate::stSimpleTextured] = createShaderModule(RSshaderTemplate::stSimpleTextured);
    ishaderModuleMap[RSshaderTemplate::stVolumeSlice] = createShaderModule(RSshaderTemplate::stVolumeSlice);
    ishaderModuleMap[RSshaderTemplate::stLines] = createShaderModule(RSshaderTemplate::stLines);

    RSspatial identitySpl;
    spatialCreate(_identitySpatialID, identitySpl);
    
    iisRSinited = true;
    
    return RSresult::SUCCESS;
}

VkRSinstance VkRenderSystem::getVkInstanceData()
{
    return iinstance;
}

void VkRenderSystem::createAppearanceDescriptorPool()
{
    //TODO: implement better heuristics based on system capacity and application needs
    const uint32_t MAX_APPEARANCES = 100;
    const uint32_t MAX_FRAMES_IN_FLIGHT = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    const uint32_t maxUniformDescriptors = MAX_APPEARANCES * MAX_FRAMES_IN_FLIGHT;
    const uint32_t maxSamplerDescriptors = MAX_APPEARANCES * MAX_FRAMES_IN_FLIGHT; //one texture + one LUT
    const uint32_t maxDescriptorSets = 10 * MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolSize uniformPoolSize{};
    uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformPoolSize.descriptorCount = maxUniformDescriptors;

    VkDescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = maxSamplerDescriptors;
        
    std::array<VkDescriptorPoolSize, 2> poolSizes = { uniformPoolSize, samplerPoolSize };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxDescriptorSets;

    VkResult res = vkCreateDescriptorPool(iinstance.device, &poolInfo, nullptr, &iinstance.appearanceDescriptorPool);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
}

void VkRenderSystem::createDescriptorPool(VkRSview& vkrsview) 
{
    // counted number of individual variables in the shader. update this if we want more
    const uint32_t MAX_NUM_VIEWS = 4;
    const uint32_t MAX_FRAMES_IN_FLIGHT = VkRScontext::MAX_FRAMES_IN_FLIGHT;
    //TODO: this doesn't seem right..ideally we would need just num_entries * num_frames_in_flight, we should remove max num view- needs investigation
    const uint32_t maxUniformDescriptors = 10 * MAX_FRAMES_IN_FLIGHT * MAX_NUM_VIEWS;
    const uint32_t maxDescriptorSets = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolSize uniformPoolSize{};
    uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformPoolSize.descriptorCount = maxUniformDescriptors;

    std::array<VkDescriptorPoolSize, 1> poolSizes = { uniformPoolSize};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxDescriptorSets;

    VkResult res = vkCreateDescriptorPool(iinstance.device, &poolInfo, nullptr, &vkrsview.descriptorPool);
    if (res != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create descriptor pool");
    }
}


bool VkRenderSystem::isRenderSystemInited() 
{
    return iisRSinited;
}

RSresult VkRenderSystem::renderSystemDispose() 
{
    VkRSfactory::disposeAllDescriptorLayouts();
    VkRSfactory::disposeAllRenderPasses();

    vkDestroyCommandPool(iinstance.device, iinstance.commandPool, nullptr);
    vkDestroyDevice(iinstance.device, nullptr);
    if (iinitInfo.enableValidation) 
    {
        DestroyDebugUtilsMessengerEXT(iinstance.instance, iinstance.debugMessenger, nullptr);
    }

    vkDestroyInstance(iinstance.instance, nullptr);

    return RSresult::SUCCESS;
}

std::vector<const char*> VkRenderSystem::getRequiredExtensions(const RSinitInfo& info) const 
{
    std::vector<const char*> extensions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
    };
    
#if defined(_WIN32)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
    
#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
//    extensions.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
//    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    if (info.enableValidation) 
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

#if defined(_WIN32)
VkSurfaceKHR VkRenderSystem::createDummySurface(const HWND hwnd, const HINSTANCE hinst) 
{
    VkSurfaceKHR surface;
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = hinst;
    surfaceCreateInfo.hwnd = hwnd;
    VkResult res = vkCreateWin32SurfaceKHR(iinstance.instance, &surfaceCreateInfo, nullptr, &surface);

    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create a dummy surface!");
    }

    return surface;
}
#elif defined(VK_USE_PLATFORM_IOS_MVK)
VkSurfaceKHR VkRenderSystem::createDummySurface(const void* parentSurface) 
{
    VkSurfaceKHR surface;
    VkMetalSurfaceCreateInfoEXT surfaceCI{};
    surfaceCI.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceCI.pLayer = parentSurface;
    
    VkResult res = vkCreateMetalSurfaceEXT(iinstance.instance, &surfaceCI, nullptr, &surface);
    if(res != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create metal surface");
    }

    return surface;
}
#endif


void VkRenderSystem::disposeDummySurface(const VkSurfaceKHR vksurface) 
{
    if(vksurface != VK_NULL_HANDLE) 
    {
        vkDestroySurfaceKHR(iinstance.instance, vksurface, nullptr);
    }
}


void VkRenderSystem::populateInstanceData(VkRSinstance& inst, const RSinitInfo& info) 
{
    const std::vector<const char*> exts = getRequiredExtensions(info);
    for (const char* ext : exts) 
    {
        inst.vkRequiredExtensions.push_back(ext);
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
    for (const auto& prop : extensionProperties) 
    {
        inst.vkExtensionProps.push_back(prop.extensionName);
    }
    
    uint32_t version;
    vkEnumerateInstanceVersion(&version);
    inst.majorVersion = VK_API_VERSION_MAJOR(version);
    inst.minorVersion = VK_API_VERSION_MINOR(version);
    inst.patchVersion = VK_API_VERSION_PATCH(version);
    inst.variantVersion = VK_API_VERSION_VARIANT(version);
    std::cout << "Vulkan version: " << inst.majorVersion << " (major) " << inst.minorVersion << " (minor)" << inst.patchVersion << " (patch) " << inst.variantVersion << " (variant)" << std::endl;
}

bool VkRenderSystem::checkValidationLayerSupport() const 
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::cout << "\nAvailable layers:" << std::endl;
    for (const char* layerName : VkRSinstance::validationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            std::cout << "\tlayername: " << layerProperties.layerName << std::endl;
            if (strcmp(layerName, layerProperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) 
        {
            return false;
        }
    }

    return true;
}

void VkRenderSystem::createInstance(const RSinitInfo& info) 
{
    if (info.enableValidation && !checkValidationLayerSupport()) 
    {
        std::runtime_error("Validation layers requested by not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = info.appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VkRenderSystem";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (info.enableValidation) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VkRSinstance::validationLayers.size());
        createInfo.ppEnabledLayerNames = VkRSinstance::validationLayers.data();
        {
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            debugCreateInfo.pUserData = nullptr; // Optional
        }
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    auto extensions = getRequiredExtensions(info);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &iinstance.instance);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create instance!");
    }

    //retrieve a list of supported extensions
    populateInstanceData(iinstance, info);
}

void VkRenderSystem::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const 
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
}

void VkRenderSystem::setupDebugMessenger() 
{
    if (!iinitInfo.enableValidation)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(iinstance.instance, &createInfo, nullptr, &iinstance.debugMessenger) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VkRenderSystem::printPhysicalDeviceInfo(VkPhysicalDevice device) 
{
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceFeatures(device, &features);
    vkGetPhysicalDeviceProperties(device, &props);
    iinstance.maxBoundDescriptorSets = props.limits.maxBoundDescriptorSets;
    iinstance.maxCombinedSamplerDescriptorSets = props.limits.maxDescriptorSetSampledImages;
    iinstance.maxUniformDescriptorSets = props.limits.maxDescriptorSetUniformBuffers;
    
    VkPhysicalDeviceMaintenance3Properties maintainence3Props{};
    maintainence3Props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
    
    VkPhysicalDeviceProperties2 deviceProps2;
    deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProps2.pNext = &maintainence3Props;
    
    vkGetPhysicalDeviceProperties2(device, &deviceProps2);
    iinstance.maxMemoryAllocationSize = maintainence3Props.maxMemoryAllocationSize;

    std::cout << "\n" << "Device properties" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << "\tDevice name: " << props.deviceName << std::endl;
    std::cout << "\tDriver version: " << props.driverVersion << std::endl;
    std::cout << "\tDevice Type: " << props.deviceType << std::endl;
    std::cout << "\tmaxImageDimension3D: " << props.limits.maxImageDimension3D << " - along any one dimension" << std::endl;
    std::cout << "\tmaxImageDimension2D: " << props.limits.maxImageDimension2D << " - along any one dimension" << std::endl;
    std::cout << "\tmaxImageDimension1D: " << props.limits.maxImageDimension1D << " - along any one dimension" << std::endl;
    std::cout << "\tmaxPushConstantsSize: " << props.limits.maxPushConstantsSize<< " bytes" << std::endl;
    std::cout << "\tmaxMemoryAllocationCount: "<<props.limits.maxMemoryAllocationCount<<std::endl;

    std::cout << "\nFeatures" << std::endl;
    std::cout << "===========" << std::endl;
    std::cout << "\tmultiDrawIndirect: " << features.multiDrawIndirect << std::endl;
    std::cout << "\tshaderInt64: " << features.shaderInt64 << std::endl;
    std::cout << "\tfragmentStoresAndAtomics: " << features.fragmentStoresAndAtomics << std::endl;
    std::cout << "\tmaxBoundDescriptorSets: " << iinstance.maxBoundDescriptorSets << std::endl;
    std::cout << "\tmaxCombinedSamplerDescriptorSets: "<< iinstance.maxCombinedSamplerDescriptorSets << std::endl;
    std::cout << "\tmaxUniformDescriptorSets: " << iinstance.maxUniformDescriptorSets << std::endl;
    std::cout << "\tfillModeNonSolid: " << features.fillModeNonSolid << std::endl;
    std::cout << "\twideLines: " << features.wideLines << std::endl;
    std::cout << "\trobustBufferAccess: "<< features.robustBufferAccess<<std::endl;
}

void VkRenderSystem::pickPhysicalDevice() 
{
    //pick just the first device. TODO: if this fails, we need to allow users to pick the GPU for us.
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(iinstance.instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(iinstance.instance, &deviceCount, devices.data());
    if (!devices.empty()) 
    {
        const VkPhysicalDevice device = devices[0];
        iinstance.physicalDevice = device;
        printPhysicalDeviceInfo(device);
    }

    if (iinstance.physicalDevice == VK_NULL_HANDLE) 
    {
        throw std::runtime_error("failed to find a suitable card");
    }
}

void VkRenderSystem::createLogicalDevice(const VkSurfaceKHR& vksurface) 
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    iinstance.queueFamilyIndices = findQueueFamilies(iinstance.physicalDevice, vksurface);
    std::set<uint32_t> uniqueQueueFamiles = { iinstance.queueFamilyIndices.graphicsFamily.value(), iinstance.queueFamilyIndices.presentFamily.value() };

    float queuePriority = 1.f;
    for (uint32_t queueFamily : uniqueQueueFamiles) 
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    VkPhysicalDeviceFeatures deviceFeatures{};
    //enable sampler support for anisotropy filtering
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

#if defined(VK_USE_PLATFORM_IOS_MVK)
    //widelines is not supported on moltenVK
    deviceFeatures.wideLines = VK_FALSE;
    deviceFeatures.robustBufferAccess = VK_TRUE;
#else
    deviceFeatures.wideLines = VK_TRUE;
#endif
    
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(iinstance.deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = iinstance.deviceExtensions.data();

    if (iinitInfo.enableValidation)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(iinstance.validationLayers.size());
        createInfo.ppEnabledLayerNames = iinstance.validationLayers.data();
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
    }
    
    //This extension support for nullDescriptors is not supported on MoltenVK since Metal disallows setting nil as buffer handles.
    //For more information on what robustness and nullDescriptors are, please refer: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/robustness.adoc
#ifdef _WIN32
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2{};
    robustness2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    robustness2.nullDescriptor = VK_TRUE;

    createInfo.pNext = &robustness2;
#endif

    const VkResult result = vkCreateDevice(iinstance.physicalDevice, &createInfo, nullptr, &iinstance.device);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create logical device");
    }
    
#ifdef _WIN32
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &robustness2;
    vkGetPhysicalDeviceFeatures2(iinstance.physicalDevice, &features2);
#endif
    
    //get the graphics queue handle from the logical device.
    vkGetDeviceQueue(iinstance.device, iinstance.queueFamilyIndices.graphicsFamily.value(), 0, &iinstance.graphicsQueue);

    //get the present queue handle from the logical device
    vkGetDeviceQueue(iinstance.device, iinstance.queueFamilyIndices.presentFamily.value(), 0, &iinstance.presentQueue);
}

VkRSqueueFamilyIndices VkRenderSystem::findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR& surface) 
{
    VkRSqueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i) {
        const VkQueueFamilyProperties& queueFamilyProp = queueFamilyProperties[i];
        if (queueFamilyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

bool VkRenderSystem::viewAvailable(const RSviewID& viewID) const 
{
    return viewID.isValid() && iviewMap.find(viewID) != iviewMap.end();
}

void VkRenderSystem::viewCreateDescriptorSets(VkRSview& view) 
{
    std::vector<VkDescriptorSetLayout> layouts(VkRScontext::MAX_FRAMES_IN_FLIGHT, view.descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = view.descriptorPool;
    allocInfo.descriptorSetCount = VkRScontext::MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    view.descriptorSets.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    VkResult res = vkAllocateDescriptorSets(iinstance.device, &allocInfo, view.descriptorSets.data());
    if (res != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create descriptor sets");
    }

    for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) 
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = view.uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VkRSviewDescriptor);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = view.descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(iinstance.device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VkRenderSystem::createUniformBuffers(VkRSview& view) 
{
    VkDeviceSize buffersize = sizeof(VkRSviewDescriptor);

    view.uniformBuffers.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    view.uniformBuffersMemory.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    view.uniformBuffersMapped.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);


    for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) 
    {
        createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, view.uniformBuffers[i], view.uniformBuffersMemory[i]);
        vkMapMemory(iinstance.device, view.uniformBuffersMemory[i], 0, buffersize, 0, &view.uniformBuffersMapped[i]);
    }
}

void VkRenderSystem::updateUniformBuffer(VkRSview& view, uint32_t currentFrame) 
{
    VkRSviewDescriptor ubo{};
    ubo.proj = view.view.projmat;
    ubo.view = view.view.viewmat;
    ubo.lightPos = view.view.lightPos;
    memcpy(view.uniformBuffersMapped[currentFrame], &ubo, sizeof(VkRSviewDescriptor));
}

RSresult VkRenderSystem::viewCreate(RSviewID& outViewID, const RSview& view, const RScontextID& ctxID) 
{
    assert(ctxID.isValid() && "input contextID is not valid");
    assert(contextAvailable(ctxID) && "invalid context ID");

    if (contextAvailable(ctxID)) 
    {
        RSuint id;
        bool success = iviewIDpool.CreateID(id);
        assert(success && "failed to create a view ID");
        if (success) {
            VkRSview vkrsview;
            vkrsview.view = view;
        
            VkRScontext& vkrsctx = ictxMap[ctxID];
            createSwapChain(vkrsview, vkrsctx);
            createImageViews(vkrsview);
            createRenderpass(vkrsview);
            createDepthResources(vkrsview);
            createFramebuffers(vkrsview);
            
            createDescriptorPool(vkrsview);
            vkrsview.descriptorSetLayout = VkRSfactory::getDescriptorLayout(DescriptorLayoutType::dltViewDefault);
            createUniformBuffers(vkrsview);
            viewCreateDescriptorSets(vkrsview);
            createCommandBuffers(vkrsview);

            outViewID.id = id;
            iviewMap[outViewID] = vkrsview;
            return RSresult::SUCCESS;
        }
    }

    return RSresult::FAILURE;
}

void VkRenderSystem::viewHideInstance(const RSviewID& viewID, const RScollectionID& collectionID, const RSinstanceID& instanceID, bool hide)
{
    assert(viewID.isValid() && "invalid view ID");
    assert(collectionID.isValid() && "invalid collection ID");
    assert(instanceID.isValid() && "invalid instance ID");
    
    if(viewID.isValid() && collectionID.isValid() && instanceID.isValid())
    {
        VkRSview& vkrsview = iviewMap[viewID];
        
        if(hide)
        {
            vkrsview.hiddenInstances[collectionID].push_back(instanceID);
        }
        else
        {
            if(vkrsview.hiddenInstances.find(collectionID) != vkrsview.hiddenInstances.end())
            {
                std::vector<RSinstanceID>& hiddenInstanceList = vkrsview.hiddenInstances[collectionID];
                const auto& iter = std::find(hiddenInstanceList.begin(), hiddenInstanceList.end(), instanceID);
                if(iter != hiddenInstanceList.end())
                {
                    hiddenInstanceList.erase(iter);
                }
            }
        }
    }
}

RSresult VkRenderSystem::viewUpdate(const RSviewID& viewID, const RSview& view) 
{
    if (viewID.isValid()) 
    {
        if (iviewMap.find(viewID.id) != iviewMap.end()) 
        {
            iviewMap[viewID.id].view = view;
            iviewMap[viewID.id].view.dirty = true;

            return RSresult::SUCCESS;
        }
    }

    return RSresult::FAILURE;
}

std::optional<RSview> VkRenderSystem::viewGetData(const RSviewID& viewID) 
{
    std::optional<RSview> optionalView;
    if (viewAvailable(viewID)) 
    {
        VkRSview& vkrsview = iviewMap[viewID];
        optionalView = vkrsview.view;
    }

    return optionalView;
}

void VkRenderSystem::createDepthResources(VkRSview& view) 
{
    VkFormat depthFormat = findDepthFormat();
    createImage(view.swapChainExtent.width, view.swapChainExtent.height, 1, VK_IMAGE_TYPE_2D, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, view.depthImage, view.depthImageMemory);
    view.depthImageView = createImageView(view.depthImage, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VkRenderSystem::createImage(uint32_t width, uint32_t height, uint32_t depth, VkImageType imageType, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &imageMemory) 
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    
    VkResult res = vkCreateImage(iinstance.device, &imageInfo, nullptr, &image);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(iinstance.device, image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    res = vkAllocateMemory(iinstance.device, &allocInfo, nullptr, &imageMemory);
    
    vkBindImageMemory(iinstance.device, image, imageMemory, 0);
}


bool VkRenderSystem::createTextureImage(VkRStexture& vkrstex) 
{
    RStextureInfo &texinfo = vkrstex.texinfo;
    
    if (texinfo.texels == nullptr)
    {
        return false;
    }

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};

    VkImageType imageType = getImageType(vkrstex.texinfo.textureType);
    VkFormat imageFormat = getDefaultTextureFormat(vkrstex.texinfo.texelFormat);
    uint32_t texelSz = getTexelSize(vkrstex.texinfo.textureType, vkrstex.texinfo.texelFormat);

    VkDeviceSize imageSize = texinfo.width * texinfo.height * texinfo.depth * texinfo.numChannels * texelSz;
    VkDeviceSize stagingImageSize = imageSize > iinstance.maxMemoryAllocationSize ? iinstance.maxMemoryAllocationSize : imageSize;
    
    createBuffer(stagingImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    createImage(texinfo.width, texinfo.height, texinfo.depth, imageType, imageFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vkrstex.textureImage, vkrstex.textureImageMemory);
    
    switch (vkrstex.texinfo.textureType)
    {
    case RStextureType::ttTexture1D:
        assert(texinfo.height == 1 && texinfo.depth == 1 && "invalid depth setting for 1D textures");
        break;

    case RStextureType::ttTexture2D:
        assert(texinfo.depth == 1 && "invalid depth setting for 2D textures");
        break;

    case RStextureType::ttTexture3D:
        assert(texinfo.depth >= 1 && "invalid depth setting for 2D textures");
        break;
    }
    //calculate the number of RSvolumeChunks needed
    std::vector<RSvolumeChunk> chunkList;
    float numVoxelsInSlice = static_cast<float>(texinfo.width * texinfo.height);
    float numVoxelsInChunk = static_cast<float>(iinstance.maxMemoryAllocationSize) / static_cast<float>(texelSz);
    float numTotalVoxels = static_cast<float>(texinfo.width * texinfo.height * texinfo.depth);
    float numTotalSlices = static_cast<float>(texinfo.depth);
    
    float numSlicesInChunkf = std::ceil(numVoxelsInChunk / numVoxelsInSlice);
    float numChunksf = std::ceil(numTotalSlices / numSlicesInChunkf);

    std::cout<<"num chunks: "<<numChunksf<<std::endl;
    float prevZoffset =0;
    float zcount = 0;
    for(uint32_t i = 0; i < numChunksf; i++)
    {
        RSvolumeChunk chunk;
        chunk.xoffset = 0;
        chunk.yoffset = 0;
        chunk.zoffset = prevZoffset;
        
        chunk.xcount = texinfo.width;
        chunk.ycount = texinfo.height;
        chunk.zcount =  (i == numChunksf - 1) ? (numTotalSlices - zcount) : numSlicesInChunkf;
        
        prevZoffset = prevZoffset + numSlicesInChunkf;
        zcount = zcount + numSlicesInChunkf;
        
        chunkList.push_back(chunk);
    }
    
//    //TODO: for debugging purposes
//    for(size_t i = 0; i < chunkList.size(); i++)
//    {
//        RSvolumeChunk volchunk = chunkList[i];
//        std::cout<<"xoffset: "<<volchunk.xoffset<<", yoffset: "<<volchunk.yoffset<<", zoffset: "<<volchunk.zoffset<<std::endl;
//        std::cout<<"xcount : "<<volchunk.xcount<<", ycount: "<<volchunk.ycount<<", zcount: "<<volchunk.zcount<<std::endl;
//    }
    
    for(size_t i = 0; i < chunkList.size(); i++)
    {
        const RSvolumeChunk volchunk = chunkList[i];
        
        void *data;
        vkMapMemory(iinstance.device, stagingBufferMemory, 0, stagingImageSize, 0, &data);
        size_t offset = volchunk.xcount * volchunk.ycount * volchunk.zoffset * texelSz;
        size_t copysize = volchunk.xcount * volchunk.ycount * volchunk.zcount * texelSz;
        memcpy(data, static_cast<unsigned char*>(texinfo.texels)+offset, static_cast<size_t>(copysize));
        vkUnmapMemory(iinstance.device, stagingBufferMemory);

        transitionImageLayout(vkrstex.textureImage, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, vkrstex.textureImage, texinfo.width, texinfo.height, texinfo.depth, volchunk);
        transitionImageLayout(vkrstex.textureImage, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    vkDestroyBuffer(iinstance.device, stagingBuffer, nullptr);
    vkFreeMemory(iinstance.device, stagingBufferMemory, nullptr);

    return true;
}

uint32_t VkRenderSystem::getTexelSize(const RStextureType& textype, const RStextureFormat& texformat)
{
    uint32_t texelSize = 0;
    switch (textype)
    {
    case RStextureType::ttTexture3D:
        if (texformat == RStextureFormat::tfUnsignedBytes)
        {
            texelSize = sizeof(uint8_t);
        }
        else if (texformat == RStextureFormat::tfUnsignedShort)
        {
            texelSize = sizeof(uint16_t);
        }
        break;
    
    case RStextureType::ttTexture2D:
    default:
            texelSize = 4;
        break;
    }
    
    return texelSize;
}

VkFormat VkRenderSystem::getDefaultTextureFormat(const RStextureFormat& texformat)
{
    VkFormat vkformat = VkFormat::VK_FORMAT_UNDEFINED;
    switch(texformat)
    {
        case RStextureFormat::tfUnsignedShort:
//            vkformat = VkFormat::VK_FORMAT_R16_UNORM;
            vkformat = VkFormat::VK_FORMAT_R16_UINT;
            break;
            
        case RStextureFormat::tfUnsignedBytes:
//            vkformat = VkFormat::VK_FORMAT_R8_UNORM;
            vkformat = VkFormat::VK_FORMAT_R8_UINT;
            break;
            
        case RStextureFormat::tfRGBA8:
            vkformat = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
            break;
            
        default:
            vkformat = VkFormat::VK_FORMAT_UNDEFINED;
    }
    
    return vkformat;
}

VkImageViewType VkRenderSystem::getImageViewType(const RStextureType& textype)
{
    VkImageViewType ivt;
    switch (textype) {
        case RStextureType::ttTexture3D:
            ivt = VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
            break;
            
        case RStextureType::ttTexture2D:
            ivt = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
            break;
            
        default:
            ivt = VkImageViewType::VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
    
    return ivt;
}

VkImageType VkRenderSystem::getImageType(const RStextureType& textype)
{
    VkImageType it;
    switch (textype) {
        case RStextureType::ttTexture2D:
            it = VkImageType::VK_IMAGE_TYPE_2D;
            break;
            
        case RStextureType::ttTexture3D:
            it = VkImageType::VK_IMAGE_TYPE_3D;
            break;
            
        default:
            it = VkImageType::VK_IMAGE_TYPE_MAX_ENUM;
    }
    
    return it;
}

void VkRenderSystem::createTextureImageView(VkRStexture& vkrstex)
{
    VkFormat format = getDefaultTextureFormat(vkrstex.texinfo.texelFormat);
    VkImageViewType imageViewType = getImageViewType(vkrstex.texinfo.textureType);
    
    //TODO: add more formats in future if needed
    vkrstex.textureImageView = createImageView(vkrstex.textureImage, imageViewType, format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VkRenderSystem::createTextureSampler(VkRStexture& vkrstex) 
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(iinstance.physicalDevice, &properties);

    VkSamplerAddressMode samplerAddressMode;
    VkBool32 enableAnisotropy;
    VkCompareOp compareOp;
    switch (vkrstex.texinfo.textureType) {

    case RStextureType::ttTexture3D:
        samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        enableAnisotropy = VK_FALSE;
        compareOp = VK_COMPARE_OP_NEVER;
        break;
    
    case RStextureType::ttTexture2D:
    default:
        samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        enableAnisotropy = VK_TRUE;
        compareOp = VK_COMPARE_OP_ALWAYS;
        break;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = samplerAddressMode;
    samplerInfo.addressModeV = samplerAddressMode;
    samplerInfo.addressModeW = samplerAddressMode;
    samplerInfo.anisotropyEnable = enableAnisotropy;
    samplerInfo.maxAnisotropy = enableAnisotropy ? properties.limits.maxSamplerAnisotropy : 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = compareOp;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult res = vkCreateSampler(iinstance.device, &samplerInfo, nullptr, &vkrstex.textureSampler);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create a texture sampler");
    }
}

void VkRenderSystem::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth, const RSvolumeChunk& volchunk) 
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = 
        {
            volchunk.xoffset,
            volchunk.yoffset,
            volchunk.zoffset
        };
        
        region.imageExtent = 
        {
            volchunk.xcount,
            volchunk.ycount,
            volchunk.zcount
        };
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }
    endSingleTimeCommands(commandBuffer);
}

void VkRenderSystem::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) 
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
        {

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else 
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }
    endSingleTimeCommands(commandBuffer);
}

VkFormat VkRenderSystem::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
    for (VkFormat format : candidates) 
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(iinstance.physicalDevice, format, &props);
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

VkFormat VkRenderSystem::findDepthFormat() 
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool VkRenderSystem::hasStencilComponent(VkFormat format) 
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VkRenderSystem::disposeView(VkRSview& view) 
{

    vkDestroyDescriptorPool(iinstance.device, view.descriptorPool, nullptr);

    for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++) 
    {
        vkDestroyBuffer(iinstance.device, view.uniformBuffers[i], nullptr);
        vkFreeMemory(iinstance.device, view.uniformBuffersMemory[i], nullptr);
    }

    for (const auto& imageView : view.swapChainImageViews) 
    {
        vkDestroyImageView(iinstance.device, imageView, nullptr);
    }
    view.swapChainImageViews.clear();
    vkDestroySwapchainKHR(iinstance.device, view.swapChain, nullptr);
    view.swapChain = nullptr;

    for (auto framebuffer : view.swapChainFramebuffers) 
    {
        vkDestroyFramebuffer(iinstance.device, framebuffer, nullptr);
    }
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID) 
{
    if (viewAvailable(viewID)) 
    {
        VkRSview& vkrsview = iviewMap[viewID];
        //dispose view contents
        disposeView(vkrsview);
        iviewMap.erase(viewID);
        iviewIDpool.DestroyID(viewID.id);
        return RSresult::SUCCESS;
    }

    assert(viewID.isValid() && "input viewID is not valid");

    return RSresult::FAILURE;
}

void VkRenderSystem::createCommandBuffers(VkRSview& view) 
{
    view.commandBuffers.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = iinstance.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(view.commandBuffers.size());

    const VkResult allocCmdBufferRes = vkAllocateCommandBuffers(iinstance.device, &allocInfo, view.commandBuffers.data());
    if (allocCmdBufferRes != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocated command buffers");
    }
}

bool VkRenderSystem::contextAvailable(const RScontextID& ctxID) const 
{
    return ctxID.isValid() && ictxMap.find(ctxID.id) != ictxMap.end();
}

void VkRenderSystem::createSurface(VkRScontext& vkrsctx) 
{

    VkResult res;
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = vkrsctx.info.hinst;
    surfaceCreateInfo.hwnd = vkrsctx.info.hwnd;
    res = vkCreateWin32SurfaceKHR(iinstance.instance, &surfaceCreateInfo, nullptr, &vkrsctx.surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    VkMetalSurfaceCreateInfoEXT surfaceCI{};
    surfaceCI.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceCI.pLayer = vkrsctx.info.metallayer;
    
    res = vkCreateMetalSurfaceEXT(iinstance.instance, &surfaceCI, nullptr, &vkrsctx.surface);
#endif

    if (res!= VK_SUCCESS) {
        throw std::runtime_error("failed to create a window surface!");
    }
}

void VkRenderSystem::createSyncObjects(VkRScontext& ctx) 
{
    ctx.imageAvailableSemaphores.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    ctx.renderFinishedSemaphores.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    ctx.inFlightFences.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    const VkDevice& device = iinstance.device;
    for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        VkResult imgAvailSemaphoreRes = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &ctx.imageAvailableSemaphores[i]);
        VkResult renderFinishedSemaphoreRes = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &ctx.renderFinishedSemaphores[i]);
        VkResult fenceRes = vkCreateFence(device, &fenceInfo, nullptr, &ctx.inFlightFences[i]);
        
        if (imgAvailSemaphoreRes != VK_SUCCESS || renderFinishedSemaphoreRes != VK_SUCCESS || fenceRes != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create semaphores");
        }
    }

}

void VkRenderSystem::disposeContext(VkRScontext& ctx) 
{
    const VkDevice& device = iinstance.device;
    vkDestroySurfaceKHR(iinstance.instance, ctx.surface, nullptr);
    ctx.surface = nullptr;

    for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        vkDestroySemaphore(device, ctx.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, ctx.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, ctx.inFlightFences[i], nullptr);
    }

#if defined(_WIN32)
    ctx.info.hwnd = nullptr;
    ctx.info.hinst = nullptr;
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    ctx.info.metallayer = nullptr;
#endif
}

RSresult VkRenderSystem::contextCreate(RScontextID& outCtxID, const RScontextInfo& info) 
{
    RSuint id;
    bool success = ictxIDpool.CreateID(id);
    assert(success && "failed to create a context ID");
    if (success) 
    {
        VkRScontext vkrsctx;
        vkrsctx.info = info;
        vkrsctx.width = info.initWidth;
        vkrsctx.height = info.initHeight;
        
        createSurface(vkrsctx);
        createSyncObjects(vkrsctx);
        outCtxID.id = id;
        ictxMap[outCtxID] = vkrsctx;
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

glm::ivec2 VkRenderSystem::contextGetDimensions(const RScontextID& ctxID) 
{
    assert(ctxID.isValid() && "invalid context ID");
    
    glm::ivec2 dim(-1, -1);
    if(contextAvailable(ctxID)) 
    {
        const VkRScontext& vkrsctx = ictxMap[ctxID];
        dim = glm::ivec2(vkrsctx.width, vkrsctx.height);
    }
    
    return dim;
}

std::optional<RScontextInfo> VkRenderSystem::contextGetData(const RScontextID& ctxID) 
{
    std::optional<RScontextInfo> ctxinfo;
    if(contextAvailable(ctxID)) 
    {
        ctxinfo = ictxMap[ctxID].info;
    }
    
    return ctxinfo;
}

void VkRenderSystem::createCommandPool() 
{

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = iinstance.queueFamilyIndices.graphicsFamily.value();
    const VkResult cmdpoolResult = vkCreateCommandPool(iinstance.device, &poolInfo, nullptr, &iinstance.commandPool);
    if (cmdpoolResult != VK_SUCCESS) 
    {
        throw std::runtime_error("faailed to create command pool");
    }
}

void VkRenderSystem::recordCommandBuffer(const RScollectionID* collections, uint32_t numCollections, const VkRSview& view, const VkRScontext& ctx, uint32_t imageIndex, uint32_t currentFrame)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    const VkCommandBuffer commandBuffer = view.commandBuffers[currentFrame];
    const VkResult beginRes = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (beginRes != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to begin recording command buffers!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = view.renderPass;
    renderPassInfo.framebuffer = view.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = view.swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{view.view.clearColor.r, view.view.clearColor.g, view.view.clearColor.b, view.view.clearColor.a}};
    clearValues[1].depthStencil = {1.0f, 0};
    
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(view.swapChainExtent.width);
        viewport.height = static_cast<float>(view.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = view.swapChainExtent;
        vkCmdSetScissor(view.commandBuffers[currentFrame], 0, 1, &scissor);
        
        for (uint32_t i = 0; i < numCollections; i++) 
        {
            const RScollectionID collectionID = collections[i];
            const VkRScollection& collection = icollectionMap[collectionID];
            for (const auto& iter : collection.drawCommands)
            {
                if(view.hiddenInstances.find(collectionID) != view.hiddenInstances.end())
                {
                    const std::vector<RSinstanceID>& instanceList = view.hiddenInstances.at(collectionID);
                    if(std::find(instanceList.begin(), instanceList.end(), iter.first) != instanceList.end())
                    {
                        continue;
                    }
                }
                
                const VkRScollectionInstance& vkrsci = collection.instanceMap.at(iter.first);

                if(vkrsci.hide)
                {
//                    std::cout<<"Hidden: "<<vkrsci.instInfo.name<<std::endl;
                    continue;
                }
                
                const VkRSdrawCommand& drawcmd = iter.second;
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawcmd.graphicsPipeline);

                std::vector<VkDeviceSize> offsets;
                switch (drawcmd.attribSetting) 
                {
                    case RSvertexAttributeSettings::vasInterleaved: 
                    {
                        VkBuffer vertexBuffers[]{ drawcmd.vertexBuffers[0]};
                        offsets = { drawcmd.vertexOffset };
                        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets.data());
                        break;
                    }

                    case RSvertexAttributeSettings::vasSeparate: 
                    {
                        offsets = { 0, 0, 0, 0 };
                        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(drawcmd.vertexBuffers.size()), drawcmd.vertexBuffers.data(), offsets.data());
                        break;
                    }
                }

                if (drawcmd.isIndexed) 
                {
                    vkCmdBindIndexBuffer(commandBuffer, drawcmd.indicesBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
                }
                
                //bind the descriptor sets
                std::vector<VkDescriptorSet> descriptorSets;
                descriptorSets.push_back(view.descriptorSets[currentFrame]);
                if(drawcmd.appID.isValid())
                {
                    const VkRSappearance& vkrsapp = iappearanceMap[drawcmd.appID];
                    if(vkrsapp.descriptorSetLayout != VK_NULL_HANDLE)
                    {
                        descriptorSets.push_back(vkrsapp.descriptorSets[currentFrame]);
                    }
                }
                uint32_t numDescriptorSets = static_cast<uint32_t>(descriptorSets.size());
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawcmd.pipelineLayout, 0, numDescriptorSets, descriptorSets.data(), 0, nullptr);
                const RSspatial& spatial = ispatialMap[drawcmd.spatialID].spatial;
                vkCmdPushConstants(commandBuffer, drawcmd.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RSspatial), &spatial);

                if (drawcmd.isIndexed) 
                {
                    vkCmdDrawIndexed(commandBuffer, drawcmd.numIndices, 1, 0, 0, 0);
                }
                else 
                {
                    vkCmdDraw(commandBuffer, drawcmd.numVertices, 1, 0, 0);
                }
            }
        }
    }
    vkCmdEndRenderPass(view.commandBuffers[currentFrame]);
        

    const VkResult endCmdBuffRes = vkEndCommandBuffer(commandBuffer);
    if (endCmdBuffRes != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to record command buffer");
    }
}

void VkRenderSystem::contextDrawCollections(VkRScontext& ctx, VkRSview& view, const RScollectionID* collectionIDs, uint32_t numCollections)
{
    //TODO: debugging purpose
    //std::cout<<"rendering view name: "<<view.view.name<<std::endl;
    const VkDevice& device = iinstance.device;
    uint32_t currentFrame = view.currentFrame;
    const VkFence inflightFence = ctx.inFlightFences[currentFrame];
    const VkSemaphore imageAvailableSemaphore = ctx.imageAvailableSemaphores[currentFrame];
    const VkSemaphore renderFinishedSemaphore = ctx.renderFinishedSemaphores[currentFrame];

    vkWaitForFences(device, 1, &inflightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(device, view.swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    //only reset the fence if we are submitting work.
    vkResetFences(device, 1, &inflightFence);
    
    updateUniformBuffer(view, currentFrame);
    
    const VkCommandBuffer commandBuffer = view.commandBuffers[currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    recordCommandBuffer(collectionIDs, numCollections, view, ctx, imageIndex, currentFrame);
        
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    const VkResult submitRes = vkQueueSubmit(iinstance.graphicsQueue, 1, &submitInfo, inflightFence);
    if (submitRes != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { view.swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    res = vkQueuePresentKHR(iinstance.presentQueue, &presentInfo);
    view.currentFrame = (currentFrame + 1) % VkRScontext::MAX_FRAMES_IN_FLIGHT;
}

RSresult VkRenderSystem::contextDrawCollections(const RScontextID& ctxID, const RSviewID& viewID, const RScollectionID* collectionIDs, const uint32_t numCollections)
{
    assert(ctxID.isValid() && "invalid input context ID");
    assert(viewID.isValid() && "input viewID is not valid");
    
    if (iinitInfo.onScreenCanvas) 
    {
        if (contextAvailable(ctxID) && viewAvailable(viewID)) 
        {
            VkRScontext& ctx = ictxMap[ctxID.id];
            VkRSview& view = iviewMap[viewID];
            
            contextDrawCollections(ctx, view, collectionIDs, numCollections);

            VkResult res = vkDeviceWaitIdle(iinstance.device);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("Failed to wait for device to become idle");
            }
        }
    }
    
    return RSresult::SUCCESS;
}

RSresult VkRenderSystem::contextDispose(const RScontextID& ctxID) {
    assert(ctxID.isValid() && "input context ID is not valid");

    if (contextAvailable(ctxID)) 
    {
        VkRScontext& ctx = ictxMap[ctxID.id];
        
        disposeContext(ctx);
        ictxMap.erase(ctxID.id);
        ictxIDpool.DestroyID(ctxID.id);

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

void VkRenderSystem::cleanupSwapChain(VkRSview& view)
{

    vkDestroyImageView(iinstance.device, view.depthImageView, nullptr);
    vkDestroyImage(iinstance.device, view.depthImage, nullptr);
    vkFreeMemory(iinstance.device, view.depthImageMemory, nullptr);
    view.depthImageView = VK_NULL_HANDLE;
    view.depthImage = VK_NULL_HANDLE;
    view.depthImageMemory = VK_NULL_HANDLE;

    for (auto framebuffer : view.swapChainFramebuffers) 
    {
        vkDestroyFramebuffer(iinstance.device, framebuffer, nullptr);
    }

    for (const auto& imageView : view.swapChainImageViews) 
    {
        vkDestroyImageView(iinstance.device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(iinstance.device, view.swapChain, nullptr);
}

VkImageView VkRenderSystem::createImageView(VkImage image, VkImageViewType imageViewType, VkFormat format, VkImageAspectFlags aspectFlags) 
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = imageViewType;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (imageViewType == VK_IMAGE_VIEW_TYPE_2D) 
    {
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    }

    VkImageView imageView{};
    VkResult res = vkCreateImageView(iinstance.device, &viewInfo, nullptr, &imageView);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void VkRenderSystem::createImageViews(VkRSview& view) 
{
    view.swapChainImageViews.resize(view.swapChainImages.size());

    for (size_t i = 0; i < view.swapChainImages.size(); ++i) 
    {
        view.swapChainImageViews[i] = createImageView(view.swapChainImages[i], VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, view.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkSurfaceFormatKHR VkRenderSystem::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) 
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

/**
* VK_PRESENT_MODE_IMMEDIATE_KHR : images submitted by the app is transferred to the screen immediately (may result in tearing)
* VK_PRESENT_MODE_FIFO_KHR: vertical sync mode - if the queue is full, the app needs to wait before submitting a new image
* VK_PRESENT_MODE_FIFO_RELAXED_KHR - if the queue is empty, then instead of waiting for the next vertical blank,
*                                 the image is transferred right away.
* VK_PRESENT_MODE_MAILBOX_KHR - if the queue is full, instead of blocking the app from submitting a new image to the queue, the
*                                newest image is replaced with "newer" images. AKA triple buffering
*
* VK_PRESENT_MODE_FIFO_KHR  - is guaranteed to be available.
*/
VkPresentModeKHR VkRenderSystem::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
{
    for (const auto& availablePresentMode : availablePresentModes) 
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkRenderSystem::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, RSuint surfaceWidth, RSuint surfaceHeight) 
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        VkExtent2D actualExtent = {surfaceWidth, surfaceHeight};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkRSswapChainSupportDetails VkRenderSystem::querySwapChainSupport(VkPhysicalDevice device, const VkSurfaceKHR& vksurface) 
{
    VkRSswapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vksurface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vksurface, &formatCount, nullptr);
    if (formatCount != 0) 
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vksurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vksurface, &presentModeCount, nullptr);
    if (presentModeCount) 
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vksurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void VkRenderSystem::createSwapChain(VkRSview& view, VkRScontext& ctx) 
{
    VkRSswapChainSupportDetails swapChainSupport = querySwapChainSupport(iinstance.physicalDevice, ctx.surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, ctx.width, ctx.height);

    //TODO: perhaps use std::clamp here
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ctx.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.presentMode = presentMode;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; //this is to specify if you're using a stereoscropic 3D application or not.
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //this means that swap chain is treated like a bunch of attachments. if we want to post process, then use VK_IMAGE_USAGE_TRANSFER_DST_BIT  and perform a memory operation to transfer the rendered image to a swap chain image.

//    VkRSqueueFamilyIndices indices = findQueueFamilies(iinstance.physicalDevice, ctx.surface);
    const VkRSqueueFamilyIndices indices = iinstance.queueFamilyIndices;
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) 
    {
        //images can be used across multiple queue families without explicit ownership transfers
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
    {
        //image is owned by one queue family at a time and ownership must be explicity transferred to another queue.
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    const VkDevice& device = iinstance.device;
    const VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &view.swapChain);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    vkGetSwapchainImagesKHR(device, view.swapChain, &imageCount, nullptr);
    view.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, view.swapChain, &imageCount, view.swapChainImages.data());

    view.swapChainImageFormat = surfaceFormat.format;
    view.swapChainExtent = extent;
    
    //optionally setting the surface format on the instance, since its going to be same for all contexts.
    iinstance.surfaceFormat = surfaceFormat;
}

void VkRenderSystem::createFramebuffers(VkRSview& view) 
{
    view.swapChainFramebuffers.resize(view.swapChainImageViews.size());

    for (size_t i = 0; i < view.swapChainImageViews.size(); ++i) 
    {
        std::array<VkImageView, 2> attachments= { view.swapChainImageViews[i], view.depthImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = view.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = view.swapChainExtent.width;
        framebufferInfo.height = view.swapChainExtent.height;
        framebufferInfo.layers = 1;

        const VkResult fboresult = vkCreateFramebuffer(iinstance.device, &framebufferInfo, nullptr, &view.swapChainFramebuffers[i]);
        if (fboresult != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void VkRenderSystem::recreateSwapchain(VkRScontext& ctx, VkRSview& view) 
{
    if (ctx.resized) 
    {
        vkDeviceWaitIdle(iinstance.device);
        
        cleanupSwapChain(view);
        
        createSwapChain(view, ctx);
        createImageViews(view);
        createDepthResources(view);
        createFramebuffers(view);
        
        vkDeviceWaitIdle(iinstance.device);
        
        ctx.resized = false;
    }
}

void VkRenderSystem::contextResized(const RScontextID& ctxID, const RSviewID& viewID, uint32_t newWidth, uint32_t newHeight) 
{
    assert(ctxID.isValid() && "input context ID is not valid");
    assert(viewID.isValid() && "input view ID is not valid");
    if (contextAvailable(ctxID) && viewAvailable(viewID)) 
    {
        VkRScontext& ctx = ictxMap[ctxID];
        VkRSview& view = iviewMap[viewID];
        ctx.resized = true;
        ctx.width = newWidth;
        ctx.height = newHeight;
        recreateSwapchain(ctx, view);
    }
}

VkRSshader VkRenderSystem::createShaderModule(const RSshaderTemplate shaderTemplate) 
{
    
    const std::string shaderFileName = getShaderStr(shaderTemplate);
    std::string shaderPathStr = iinitInfo.shaderPath;
    
    //shaderPathStr ends with a "/"
    std::string vertShaderPath = shaderPathStr + shaderFileName + "_vert.spv";
    std::string fragShaderPath = shaderPathStr + shaderFileName + "_frag.spv";
    const std::vector<char>& vertfile = readFile(vertShaderPath, std::ios::ate | std::ios::binary);
    const std::vector<char>& fragfile = readFile(fragShaderPath, std::ios::ate | std::ios::binary);
    
    std::string vertShaderSrcPath = shaderPathStr + shaderFileName + ".vert";
    std::string fragShaderSrcPath = shaderPathStr + shaderFileName + ".frag";
    const std::vector<char>& vertSrcContent = readFile(vertShaderSrcPath, std::ios::ate);
    const std::vector<char>& fragSrcContent = readFile(vertShaderSrcPath, std::ios::ate);
    VkRSshader vkrsshader;
    vkrsshader.shadernName = shaderFileName;
    vkrsshader.vertShaderContent = vertSrcContent.data();
    vkrsshader.fragShaderContent = fragSrcContent.data();
    
    VkShaderModuleCreateInfo vertShaderModuleCI{};
    vertShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderModuleCI.codeSize = vertfile.size();
    vertShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(vertfile.data());

    VkResult result = vkCreateShaderModule(iinstance.device, &vertShaderModuleCI, nullptr, &vkrsshader.vert);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create a shader module!");
    }
    
    VkShaderModuleCreateInfo fragShaderModuleCI{};
    fragShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderModuleCI.codeSize = fragfile.size();
    fragShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(fragfile.data());

    result = vkCreateShaderModule(iinstance.device, &fragShaderModuleCI, nullptr, &vkrsshader.frag);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create a shader module!");
    }

    return vkrsshader;
}

 std::vector<char> VkRenderSystem::readFile(const std::string& filename, unsigned int openmode) 
{
     std::ifstream file(filename, openmode);

    if (!file.is_open()) 
    {
        std::string errorMsg = "failed to open file - "+filename;
        throw std::runtime_error(errorMsg);
    }

    size_t filesize = (size_t)file.tellg();
    std::vector<char> buffer(filesize);

    file.seekg(0);
    file.read(buffer.data(), filesize);

    file.close();

    return buffer;
}

void VkRenderSystem::createRenderpass(VkRSview& view) 
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = view.swapChainImageFormat;
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
    depthAttachment.format = findDepthFormat();
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

    const VkResult result = vkCreateRenderPass(iinstance.device, &renderPassInfo, nullptr, &view.renderPass);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VkRenderSystem::createGraphicsPipeline(VkRScollection& collection, VkRScollectionInstance& collinst, VkRSdrawCommand& drawcmd) 
{
    if(!appearanceAvailable(drawcmd.appID))
    {
        throw std::runtime_error("invalid appearance ID");
        return;
    }
    
    VkRSappearance& vkrsapp = iappearanceMap[drawcmd.appID];
    const RSshaderTemplate shaderTemplate = vkrsapp.appInfo.shaderTemplate;
    
    const VkRSshader& vkrsshader = ishaderModuleMap[shaderTemplate];
    VkShaderModule vertShaderModule = vkrsshader.vert;
    VkShaderModule fragShaderModule = vkrsshader.frag;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo , fragShaderStageInfo };

    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    VkRSgeometryData& gdata = igeometryDataMap[collinst.instInfo.gdataID];
    
    //At a minimum all shaders must have the following vertex attributes defined and corresponding buffers bound.
    const std::vector<VkVertexInputBindingDescription> bindings = getBindingDescription(gdata.attributesInfo);
    const std::vector<VkVertexInputAttributeDescription> attribs = getAttributeDescriptions(gdata.attributesInfo);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attribs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = drawcmd.primTopology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    static const VkExtent2D DEFAULT_SWAPCHAIN_EXTENT = {800, 600};
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = DEFAULT_SWAPCHAIN_EXTENT.width;
    viewport.height = DEFAULT_SWAPCHAIN_EXTENT.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = DEFAULT_SWAPCHAIN_EXTENT;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    if (drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP || drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST) 
    {
        rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
    }
    else if (drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST) 
    {
        rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
    }
    else 
    {
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    }
    rasterizer.lineWidth = drawcmd.state.lnstate.lineWidth;
    if (drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN || drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || drawcmd.primTopology == VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP) 
    {
        rasterizer.cullMode = VK_CULL_MODE_NONE;
    }
    else 
    {
        rasterizer.cullMode = VK_CULL_MODE_NONE;
    }
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; //optional
    rasterizer.depthBiasClamp = 0.0f; //optional
    rasterizer.depthBiasSlopeFactor = 0.0f; //optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VkDescriptorSetLayout viewDSL = VkRSfactory::getDescriptorLayout(DescriptorLayoutType::dltViewDefault);
    descriptorSetLayouts.push_back(viewDSL);
    if(vkrsapp.descriptorSetLayout != VK_NULL_HANDLE)
    {
        descriptorSetLayouts.push_back(vkrsapp.descriptorSetLayout);
    }

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    const RSspatialID& spatialID = collinst.instInfo.spatialID;
    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(RSspatial);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    if (spatialAvailable(spatialID)) 
    {
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    } 
    else
    {
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }
    
    VkResult pipelineLayoutResult = vkCreatePipelineLayout(iinstance.device, &pipelineLayoutInfo, nullptr, &drawcmd.pipelineLayout);
    if (pipelineLayoutResult != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = getDepthCompare(drawcmd.state.depthState.depthFunc);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = drawcmd.pipelineLayout;

    pipelineInfo.renderPass = VkRSfactory::getRenderPass(RenderPassType::rptViewSimple);
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    const VkResult graphicsPipelineResult = vkCreateGraphicsPipelines(iinstance.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &drawcmd.graphicsPipeline);
    if (graphicsPipelineResult != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void VkRenderSystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.flags = 0;

    VkResult res = vkCreateBuffer(iinstance.device, &bufferInfo, nullptr, &buffer);
    if (res != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create a buffer!");
    }

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(iinstance.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memProperties);

    res = vkAllocateMemory(iinstance.device, &allocInfo, nullptr, &bufferMemory);
    if (res != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }

    res = vkBindBufferMemory(iinstance.device, buffer, bufferMemory, 0);
}

uint32_t VkRenderSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(iinstance.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkCommandBuffer VkRenderSystem::beginSingleTimeCommands() 
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = iinstance.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(iinstance.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VkRenderSystem::endSingleTimeCommands(VkCommandBuffer commandBuffer) 
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(iinstance.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(iinstance.graphicsQueue);

    vkFreeCommandBuffers(iinstance.device, iinstance.commandPool, 1, &commandBuffer);
}


void VkRenderSystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    }
    endSingleTimeCommands(commandBuffer);
}

std::vector<VkVertexInputBindingDescription> VkRenderSystem::getBindingDescription(const RSvertexAttribsInfo& attribInfo) 
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    switch (attribInfo.settings) 
    {
        case RSvertexAttributeSettings::vasInterleaved: 
        {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = attribInfo.sizeOfInterleavedAttrib();//sizeof(rsvd::VertexPC);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescriptions.push_back(bindingDescription);
            break;
        }

        case RSvertexAttributeSettings::vasSeparate: 
        {
            for (uint32_t i = 0; i < attribInfo.numVertexAttribs; i++) 
            {
                RSvertexAttribute attrib = attribInfo.attributes[i];
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = i;//attribInfo.getBindingPoint(attrib);
                bindingDescription.stride = attribInfo.sizeOfAttrib(attrib);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                bindingDescriptions.push_back(bindingDescription);
            }
            break;
        }
    }
    
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VkRenderSystem::getAttributeDescriptions(const RSvertexAttribsInfo& attribInfo) 
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    
    attributeDescriptions.resize(attribInfo.numVertexAttribs);
    for (uint32_t i = 0; i < attribInfo.numVertexAttribs; i++) {
        RSvertexAttribute attrib = attribInfo.attributes[i];
        switch(attribInfo.settings)
        {
            case RSvertexAttributeSettings::vasInterleaved:
                attributeDescriptions[i].binding = 0;
                attributeDescriptions[i].offset = getOffset(attribInfo.numVertexAttribs, i);
                break;
                
            case RSvertexAttributeSettings::vasSeparate:
                attributeDescriptions[i].binding = i;//attribInfo.getBindingPoint(attrib);
                attributeDescriptions[i].offset = 0;
                break;
        }
        
        attributeDescriptions[i].location = i;//attribInfo.getBindingPoint(attrib);
        attributeDescriptions[i].format = getVkFormat(attribInfo.attributes[i]);
    }

    return attributeDescriptions;
}

bool VkRenderSystem::geometryDataAvailable(const RSgeometryDataID& geomDataID) 
{
    return geomDataID.isValid() && (igeometryDataMap.find(geomDataID) != igeometryDataMap.end());
}

RSresult VkRenderSystem::geometryDataCreate(RSgeometryDataID& outgdataID, uint32_t numVertices, uint32_t numIndices, const RSvertexAttribsInfo attributesInfo) 
{
    RSuint id;
    bool success = igeomDataIDpool.CreateID(id);
    assert(success && "failed to create a geometry data ID");
    if (success) 
    {
        VkRSgeometryData gdata;
        gdata.numVertices = numVertices;
        gdata.numIndices = numIndices;
        gdata.attributesInfo = attributesInfo;
        //TODO: prefer to use a unique pointer. Perhaps keep a separate struct for internal representation of vertex attributes so its not exposed to client.
        gdata.attributesInfo.attributes = new RSvertexAttribute[attributesInfo.numVertexAttribs];
        std::memcpy(gdata.attributesInfo.attributes, attributesInfo.attributes, attributesInfo.numVertexAttribs * sizeof(RSvertexAttribute));

        //Create buffer for position
        bool foundPosition = false;
        for (uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) 
        {
            if (gdata.attributesInfo.attributes[i] == RSvertexAttribute::vaPosition) 
            {
                foundPosition = true;
                break;
            }
        }
        if (foundPosition == false) 
        {
            return RSresult::FAILURE;
        }

        switch (gdata.attributesInfo.settings) 
        {
            case RSvertexAttributeSettings::vasInterleaved: 
            {
                //create buffer for staging position vertex attribs
                VkDeviceSize vaBufferSize = numVertices * attributesInfo.sizeOfInterleavedAttrib();
                createBuffer(vaBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gdata.interleaved.stagingVABuffer, gdata.interleaved.stagingVAbufferMemory);
                vkMapMemory(iinstance.device, gdata.interleaved.stagingVAbufferMemory, 0, vaBufferSize, 0, &gdata.interleaved.mappedStagingVAPtr);

                //create buffer for final vertex attributes buffer
                createBuffer(vaBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gdata.interleaved.vaBuffer, gdata.interleaved.vaBufferMemory);
                break;
            }

            case RSvertexAttributeSettings::vasSeparate: 
            {
                for (uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) 
                {
                    RSvertexAttribute attrib = gdata.attributesInfo.attributes[i];
                    VkDeviceSize bufferSize = numVertices * attributesInfo.sizeOfAttrib(attrib);
                    
                    //create staging buffer.
                    uint32_t attribidx = static_cast<uint32_t>(attrib);
                    VkRSbuffer& stageBuffer = gdata.separate.stagingBuffers[attribidx];
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stageBuffer.buffer, stageBuffer.memory);
                    vkMapMemory(iinstance.device, stageBuffer.memory, 0, bufferSize, 0, &stageBuffer.mapped);

                    //create the device buffer
                    VkRSbuffer& deviceBuffer = gdata.separate.buffers[attribidx];
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceBuffer.buffer, deviceBuffer.memory);
                }
                break;
            }
        }

        //create buffer for indices
        if (numIndices) 
        {
            VkDeviceSize indexBufferSize = numIndices * sizeof(uint32_t);
            createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, gdata.indices.stagingIndexBuffer, gdata.indices.stagingIndexBufferMemory);
            vkMapMemory(iinstance.device, gdata.indices.stagingIndexBufferMemory, 0, indexBufferSize, 0, &gdata.indices.mappedIndexPtr);
        
            createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gdata.indices.indicesBuffer, gdata.indices.indicesBufferMemory);
        }

        outgdataID.id = id;
        igeometryDataMap[outgdataID] = gdata;
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateInterleavedVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data) 
{
    assert(gdataID.isValid() && "input geometry data ID is invalid");

    if (geometryDataAvailable(gdataID)) 
    {
        VkRSgeometryData gdata = igeometryDataMap[gdataID];
        assert(gdata.attributesInfo.settings == RSvertexAttributeSettings::vasInterleaved && "attributes are not interleaved");
        if (gdata.attributesInfo.settings != RSvertexAttributeSettings::vasInterleaved) 
        {
            return RSresult::FAILURE;
        }

        if (offset != 0) 
        {
            throw std::runtime_error("Unsupported operation!");
        }
        
        //memcpy((char*)gdata.mappedStagingVAPtr + offset, data, sizeinBytes);
        memcpy(gdata.interleaved.mappedStagingVAPtr, data, sizeInBytes);
        //Debugging purposes-
        //rsvd::VertexPC* vertices = static_cast<rsvd::VertexPC*>(gdata.interleaved.mappedStagingVAPtr);

        return RSresult::SUCCESS;
    }
    return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateVertices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, RSvertexAttribute attrib, void* data) 
{
    assert(gdataID.isValid() && "input geometry data ID is invalid");

    if (geometryDataAvailable(gdataID)) 
    {
        VkRSgeometryData gdata = igeometryDataMap[gdataID];

        assert(gdata.attributesInfo.settings == RSvertexAttributeSettings::vasSeparate && "attributes are not separate");
        if (gdata.attributesInfo.settings != RSvertexAttributeSettings::vasSeparate) 
        {
            return RSresult::FAILURE;
        }
        uint32_t attribIdx = static_cast<uint32_t>(attrib);
        memcpy(gdata.separate.stagingBuffers[attribIdx].mapped, data, sizeInBytes);

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataUpdateIndices(const RSgeometryDataID& gdataID, uint32_t offset, uint32_t sizeInBytes, void* data) 
{
    assert(gdataID.isValid() && "input geometry data ID is invalid");

    if (geometryDataAvailable(gdataID)) 
    {
        VkRSgeometryData gdata = igeometryDataMap[gdataID];
        memcpy(gdata.indices.mappedIndexPtr, data, sizeInBytes);
        //For debugging purposes:
        //uint32_t* indices = static_cast<uint32_t*>(gdata.indices.mappedIndexPtr);
        return RSresult::SUCCESS;
    }
    return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataFinalize(const RSgeometryDataID& gdataID) 
{
    assert(gdataID.isValid() && "input geometry data ID is invalid");

    if (geometryDataAvailable(gdataID)) 
    {
        VkRSgeometryData gdata = igeometryDataMap[gdataID];

        //unmap the host pointers
        switch (gdata.attributesInfo.settings) 
        {
            case RSvertexAttributeSettings::vasInterleaved: 
            {
                vkUnmapMemory(iinstance.device, gdata.interleaved.stagingVAbufferMemory);
                gdata.interleaved.mappedStagingVAPtr = nullptr;
                
                //copy to device buffer
                VkDeviceSize vaBufferSize = gdata.numVertices * gdata.attributesInfo.sizeOfInterleavedAttrib();
                copyBuffer(gdata.interleaved.stagingVABuffer, gdata.interleaved.vaBuffer, vaBufferSize);
                
                //destroy the staging buffers
                vkDestroyBuffer(iinstance.device, gdata.interleaved.stagingVABuffer, nullptr);
                vkFreeMemory(iinstance.device, gdata.interleaved.stagingVAbufferMemory, nullptr);
                break;
            }

            case RSvertexAttributeSettings::vasSeparate: 
            {
                for(uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) {
                    RSvertexAttribute attrib = gdata.attributesInfo.attributes[i];
                    uint32_t attribIdx = static_cast<uint32_t>(attrib);
                    VkRSbuffer& stagingBuffer = gdata.separate.stagingBuffers[attribIdx];
                    VkRSbuffer& deviceBuffer = gdata.separate.buffers[attribIdx];
                    stagingBuffer.mapped = nullptr;

                    //copy to device buffer
                    VkDeviceSize bufferSize = gdata.numVertices * gdata.attributesInfo.sizeOfAttrib(attrib);
                    copyBuffer(stagingBuffer.buffer, deviceBuffer.buffer, bufferSize);

                    //destroy the staging buffers
                    vkDestroyBuffer(iinstance.device, stagingBuffer.buffer, nullptr);
                    vkFreeMemory(iinstance.device, stagingBuffer.memory, nullptr);
                }
                break;
            }
        }

        if (gdata.numIndices) 
        {
            vkUnmapMemory(iinstance.device, gdata.indices.stagingIndexBufferMemory);
            gdata.indices.mappedIndexPtr = nullptr;
            //copy to device buffer
            VkDeviceSize indexBufferSize = gdata.numIndices * sizeof(uint32_t);
            copyBuffer(gdata.indices.stagingIndexBuffer, gdata.indices.indicesBuffer, indexBufferSize);
            //destroy the staging buffers
            vkDestroyBuffer(iinstance.device, gdata.indices.stagingIndexBuffer, nullptr);
            vkFreeMemory(iinstance.device, gdata.indices.stagingIndexBufferMemory, nullptr);
        }

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDataDispose(const RSgeometryDataID& gdataID) 
{
    assert(gdataID.isValid() && "input geometry data ID is invalid");
    
    if (geometryDataAvailable(gdataID)) 
    {
        VkRSgeometryData gdata = igeometryDataMap[gdataID];
        vkDestroyBuffer(iinstance.device, gdata.interleaved.vaBuffer, nullptr);
        vkFreeMemory(iinstance.device, gdata.interleaved.vaBufferMemory, nullptr);

        if (gdata.numIndices) 
        {
            vkDestroyBuffer(iinstance.device, gdata.indices.indicesBuffer, nullptr);
            vkFreeMemory(iinstance.device, gdata.indices.indicesBufferMemory, nullptr);
        }
        //delete the vertex attribute enums.
        delete[] gdata.attributesInfo.attributes;
        gdata.attributesInfo.attributes = nullptr;
        igeometryDataMap.erase(gdataID);
        igeomDataIDpool.DestroyID(gdataID.id);

        return RSresult::SUCCESS;
    }
    
    return RSresult::FAILURE;
}

bool VkRenderSystem::geometryAvailable(const RSgeometryID& geomID) 
{
    return geomID.isValid() && igeometryMap.find(geomID) != igeometryMap.end();
}

RSresult VkRenderSystem::geometryCreate(RSgeometryID& outgeomID, const RSgeometryInfo& geomInfo) 
{
    RSuint id;
    bool success = igeometryIDpool.CreateID(id);
    assert(success && "failed to create a view ID");
    if (success) 
    {
        VkRSgeometry vkrsgeom;
        vkrsgeom.geomInfo = geomInfo;

        outgeomID.id = id;
        igeometryMap[outgeomID] = vkrsgeom;
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::geometryDispose(const RSgeometryID& geomID) 
{
    assert(geomID.isValid() && "input geometry ID is invalid");

    if (geometryAvailable(geomID)) 
    {
        igeometryMap.erase(geomID);
        igeometryIDpool.DestroyID(geomID.id);
        
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

bool VkRenderSystem::collectionAvailable(const RScollectionID& colID) 
{
    return colID.isValid() && (icollectionMap.find(colID) != icollectionMap.end());
}

bool VkRenderSystem::collectionInstanceAvailable(const RScollectionID& collID, const RSinstanceID& instanceID) 
{
    assert(collID.isValid() && "input collection ID is not valid");
    assert(instanceID.isValid() && "input collection instance ID is not valid");

    if (collectionAvailable(collID)) 
    {
        const VkRScollection& coll = icollectionMap[collID];
        if (instanceID.isValid() && coll.instanceMap.find(instanceID) != coll.instanceMap.end()) 
        {
            return true;
        }
    }

    return false;
}

RSresult VkRenderSystem::collectionCreate(RScollectionID& colID, const RScollectionInfo& collInfo) 
{
    RSuint id;
    bool success = icollIDpool.CreateID(id);
    assert(success && "failed to create a collection");
    if (success) 
    {
        VkRScollection vkrscol;
        vkrscol.info = collInfo;
        assert(collInfo.maxInstances > 0 && "must specify the maximum number of collection-instances");

        colID.id = id;
        icollectionMap[colID] = vkrscol;
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

void VkRenderSystem::appearanceCreateDescriptorSetLayout(VkRSappearance& vkrsapp)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    VkResult res = VkResult::VK_ERROR_UNKNOWN;
    if(vkrsapp.appInfo.shaderTemplate == RSshaderTemplate::stVolumeSlice)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding uniformLayoutBinding{};
        uniformLayoutBinding.binding = 1;
        uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformLayoutBinding.descriptorCount = 1;
        uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings {samplerLayoutBinding, uniformLayoutBinding};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        res = vkCreateDescriptorSetLayout(iinstance.device, &layoutInfo, nullptr, &vkrsapp.descriptorSetLayout);
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
    else if(vkrsapp.appInfo.shaderTemplate == RSshaderTemplate::stSimpleTextured)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &samplerLayoutBinding;

        res = vkCreateDescriptorSetLayout(iinstance.device, &layoutInfo, nullptr, &vkrsapp.descriptorSetLayout);
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

void VkRenderSystem::appearanceCreateDescriptorSet(VkRSappearance& vkrsapp, const VkDescriptorPool& descriptorPool)
{
    const RStextureID difftexID = vkrsapp.appInfo.diffuseTexture;
    if (textureAvailable(difftexID))
    {
        VkRStexture& vkrstex = itextureMap[difftexID];

        std::vector<VkDescriptorSetLayout> layouts(VkRScontext::MAX_FRAMES_IN_FLIGHT, vkrsapp.descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VkRScontext::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        vkrsapp.descriptorSets.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
        VkResult res = vkAllocateDescriptorSets(iinstance.device, &allocInfo, vkrsapp.descriptorSets.data());
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = vkrstex.textureImageView;
            imageInfo.sampler = vkrstex.textureSampler;

            VkWriteDescriptorSet descriptorWrites{};
            descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites.dstSet = vkrsapp.descriptorSets[i];
            descriptorWrites.dstBinding = 0;
            descriptorWrites.dstArrayElement = 0;
            descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites.descriptorCount = 1;
            descriptorWrites.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(iinstance.device, 1, &descriptorWrites, 0, nullptr);
        }
    }
    
    if(vkrsapp.appInfo.shaderTemplate == RSshaderTemplate::stVolumeSlice)
    {
        for(size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vkrsapp.uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(VkRSvolumeSliceDescriptor);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = vkrsapp.descriptorSets[i];
            descriptorWrite.dstBinding = 1;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;
            descriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(iinstance.device, 1, &descriptorWrite, 0, nullptr);
        }
    }
}

void VkRenderSystem::createVolumeSliceUniformBuffers(VkRSappearance& vkrsapp)
{
    VkDeviceSize buffersize = sizeof(VkRSvolumeSliceDescriptor);
    
    vkrsapp.uniformBuffers.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    vkrsapp.uniformBuffersMemory.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    vkrsapp.uniformBuffersMapped.resize(VkRScontext::MAX_FRAMES_IN_FLIGHT);
    
    for(size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkrsapp.uniformBuffers[i], vkrsapp.uniformBuffersMemory[i]);
        vkMapMemory(iinstance.device, vkrsapp.uniformBuffersMemory[i], 0, buffersize, 0, &vkrsapp.uniformBuffersMapped[i]);
    }
}

void VkRenderSystem::updateVolumeSliceUniformBuffers(VkRSappearance& vkrsapp)
{
    VkRSvolumeSliceDescriptor ubo{};
    ubo.window = vkrsapp.appInfo.volumeSlice.window;
    ubo.rescale = vkrsapp.appInfo.volumeSlice.rescale;
    for(uint32_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++)
    {
        memcpy(vkrsapp.uniformBuffersMapped[i], &ubo, sizeof(VkRSvolumeSliceDescriptor));
    }
}

RSresult VkRenderSystem::collectionInstanceCreate(const RScollectionID& collID, RSinstanceID& outInstID, const RSinstanceInfo& instInfo)
{
    assert(collID.isValid() && "input collection ID is not valid");
    
    if (!geometryDataAvailable(instInfo.gdataID) || !geometryAvailable(instInfo.geomID) || !appearanceAvailable(instInfo.appID)) 
    {
        return RSresult::FAILURE;
    }

    if (collectionAvailable(collID)) 
    {
        uint32_t id;
        iinstanceIDpool.CreateID(id);
        outInstID.id = id;
        VkRScollection& vkrscoll = icollectionMap[collID];

        VkRScollectionInstance inst;
        inst.instInfo = instInfo;
        if(!instInfo.spatialID.isValid())
        {
            inst.instInfo.spatialID = _identitySpatialID;
        }
        vkrscoll.dirty = true;
        vkrscoll.instanceMap[outInstID] = inst;

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

void VkRenderSystem::collectionInstanceHide(const RScollectionID& collID, const RSinstanceID& instID, bool hide)
{
    assert(collID.isValid() && "invalid collection ID");
    assert(instID.isValid() && "invalid instance ID");
    if(collID.isValid() && instID.isValid() && icollectionMap.find(collID) != icollectionMap.end())
    {
        VkRScollection& coll = icollectionMap[collID];
        VkRScollectionInstance& vkrsinst = coll.instanceMap[instID];
        vkrsinst.hide = hide;
    }
}

RSresult VkRenderSystem::collectionInstanceDispose(const RScollectionID& collID, RSinstanceID& instID) 
{
    assert(collID.isValid() && "input collection ID is not valid");
    assert(instID.isValid() && "input collection instance ID is not valid");

    if (collectionInstanceAvailable(collID, instID)) 
    {
        VkRScollection& coll = icollectionMap[collID];
        VkRScollectionInstance& vkrsinst = coll.instanceMap[instID];

        coll.instanceMap.erase(instID);
        iinstanceIDpool.DestroyID(instID.id);
        return RSresult::SUCCESS;
    }
    
    return RSresult::FAILURE;
}

VkPrimitiveTopology VkRenderSystem::getPrimitiveType(const RSprimitiveType& ptype) 
{
    switch (ptype)
    {
        case RSprimitiveType::ptLine:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            
        case RSprimitiveType::ptLineLoop:
        case RSprimitiveType::ptLineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

        case RSprimitiveType::ptPoint:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        case RSprimitiveType::ptTriangle:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        case RSprimitiveType::ptTriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            
        case RSprimitiveType::ptTriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    }

    return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkCompareOp VkRenderSystem::getDepthCompare(const RSdepthFunction depthFunc)
{
    VkCompareOp compareOp = VkCompareOp::VK_COMPARE_OP_LESS;
    switch (depthFunc)
    {
    case RSdepthFunction::dsAlway:
        compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
        break;

    case RSdepthFunction::dsLess:
        compareOp = VkCompareOp::VK_COMPARE_OP_LESS;
        break;

    case RSdepthFunction::dsLessEquals:
        compareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
        break;

    default:
        compareOp = VkCompareOp::VK_COMPARE_OP_LESS;
    }

    return compareOp;
}

RSresult VkRenderSystem::collectionFinalize(const RScollectionID& colID)
{
    assert(colID.isValid() && "input collection ID is not valid");

    if (collectionAvailable(colID))
    {
        VkRScollection& collection = icollectionMap[colID];

        //finalize the collection
        if (collection.dirty) 
        {
            //create drawcommands for each collection instance
            for (auto& iter : collection.instanceMap) 
            {
                const RSinstanceID& instID = iter.first;
                if(collection.drawCommands.find(instID) != collection.drawCommands.end())
                {
                    continue;
                }
                VkRScollectionInstance& collinst = iter.second;
                const RSgeometryDataID& gdataID = collinst.instInfo.gdataID;
                const RSgeometryID& geomID = collinst.instInfo.geomID;
                if (geometryDataAvailable(gdataID) && geometryAvailable(geomID)) 
                {
                    const VkRSgeometryData& gdata = igeometryDataMap[gdataID];
                    const VkRSgeometry& geom = igeometryMap[geomID];
                    VkRSdrawCommand cmd;
                    cmd.indicesBuffer = gdata.indices.indicesBuffer;
                    cmd.isIndexed = gdata.indices.indicesBuffer != VK_NULL_HANDLE;
                    cmd.numIndices = gdata.numIndices;
                    cmd.numVertices = gdata.numVertices;
                    cmd.attribSetting = gdata.attributesInfo.settings;
                    if (gdata.attributesInfo.settings == RSvertexAttributeSettings::vasInterleaved) 
                    {
                        cmd.vertexBuffers = { gdata.interleaved.vaBuffer };
                    } 
                    else
                    {
                        for (uint32_t i = 0; i < gdata.attributesInfo.numVertexAttribs; i++) 
                        {
                            RSvertexAttribute attrib = gdata.attributesInfo.attributes[i];
                            uint32_t attribIdx = static_cast<uint32_t>(attrib);
                            assert(gdata.separate.buffers[attribIdx].buffer != VK_NULL_HANDLE && "specified vk buffer cannot be null");
                            cmd.vertexBuffers.push_back(gdata.separate.buffers[attribIdx].buffer);
                        }
                    }
                    cmd.primTopology = getPrimitiveType(geom.geomInfo.primType);
                    if (stateAvailable(collinst.instInfo.stateID))
                    {
                        VkRSstate &state = istateMap[collinst.instInfo.stateID];
                        cmd.state = state.state;
                    }

                    cmd.appID = collinst.instInfo.appID;
                    
                    cmd.spatialID = collinst.instInfo.spatialID.isValid() ? collinst.instInfo.spatialID : _identitySpatialID;

                    createGraphicsPipeline(collection, collinst, cmd);

                    collection.drawCommands[instID] = cmd;
                }
            }
            collection.dirty = false;
        }
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::collectionDispose(RScollectionID& colID) 
{
    assert(colID.isValid() && "input collection ID is not valid");

    if (colID.isValid() && icollectionMap.find(colID) != icollectionMap.end())
    {
        VkRScollection& collection = icollectionMap[colID];
        //dispose the collection
        disposeCollection(collection);
        icollectionMap.erase(colID);
        
        icollIDpool.DestroyID(colID.id);
        colID.id = INVALID_ID;
        
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

void VkRenderSystem::disposeCollection(VkRScollection& collection) 
{
    const VkDevice& device = iinstance.device;

    //disposes vulkan constructs
    for (size_t i = 0; i < collection.drawCommands.size(); i++) 
    {
        VkRSdrawCommand& drawcmd = collection.drawCommands[i];
        
        //dispose pipeline
        vkDestroyPipeline(device, drawcmd.graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, drawcmd.pipelineLayout, nullptr);
    }
    
    //dispose instances
    for(auto& iter : collection.instanceMap) 
    {
        RSinstanceID instID = iter.first;
        iinstanceIDpool.DestroyID(instID.id);
    }
    
    collection.instanceMap.clear();
    collection.drawCommands.clear();
}

bool VkRenderSystem::appearanceAvailable(const RSappearanceID& appID) const 
{
    return appID.isValid() && iappearanceMap.find(appID) != iappearanceMap.end();
}

RSresult VkRenderSystem::appearanceCreate(RSappearanceID& outAppID, const RSappearanceInfo& appInfo) 
{
    RSuint id;
    bool success = iappearanceIDpool.CreateID(id);
    assert(success && "failed to create a appearance ID");
    if (success)
    {
        VkRSappearance vkrsapp;
        vkrsapp.appInfo = appInfo;
        if(appInfo.shaderTemplate == RSshaderTemplate::stSimpleTextured ||
           appInfo.shaderTemplate == RSshaderTemplate::stVolumeSlice)
        {
            createVolumeSliceUniformBuffers(vkrsapp);
            appearanceCreateDescriptorSetLayout(vkrsapp);
            appearanceCreateDescriptorSet(vkrsapp, iinstance.appearanceDescriptorPool);
            updateVolumeSliceUniformBuffers(vkrsapp);
        }
        outAppID.id = id;
        iappearanceMap[outAppID] = vkrsapp;

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

bool VkRenderSystem::appearanceUpdateVolumeSlice(const RSappearanceID& appID, const RSvolumeSliceAppearance& vsapp)
{
    assert(appID.isValid() && "invalid appearance ID");
    if(appID.isValid())
    {
        VkRSappearance& vkrsapp = iappearanceMap[appID];
        vkrsapp.appInfo.volumeSlice = vsapp;
        updateVolumeSliceUniformBuffers(vkrsapp);

        return true;
    }
    
    return false;
}

RSresult VkRenderSystem::appearanceDispose(const RSappearanceID& appID) 
{
    assert(appID.isValid() && "input appearance ID is invalid");

    if (appearanceAvailable(appID)) 
    {
        const VkRSappearance& vkrsapp = iappearanceMap[appID];
        if(!vkrsapp.uniformBuffers.empty() && vkrsapp.uniformBuffersMemory.empty())
        {
            for (size_t i = 0; i < VkRScontext::MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyBuffer(iinstance.device, vkrsapp.uniformBuffers[i], nullptr);
                vkFreeMemory(iinstance.device, vkrsapp.uniformBuffersMemory[i], nullptr);
            }
        }

        vkDestroyDescriptorSetLayout(iinstance.device, vkrsapp.descriptorSetLayout, nullptr);
        
        iappearanceMap.erase(appID);
        
        iappearanceIDpool.DestroyID(appID.id);
        
        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}


bool VkRenderSystem::spatialAvailable(const RSspatialID& spatialID) 
{
    return spatialID.isValid() && ispatialMap.find(spatialID) != ispatialMap.end();
}

RSresult VkRenderSystem::spatialCreate(RSspatialID& outSplID, const RSspatial& splInfo) 
{
    RSuint id;
    bool success = ispatialIDpool.CreateID(id);
    assert(success && "failed to create a spatial ID");
    if (success) 
    {
        VkRSspatial vkrsspatial;
        vkrsspatial.spatial = splInfo;

        outSplID.id = id;
        ispatialMap[outSplID] = vkrsspatial;

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

bool VkRenderSystem::spatialSetData(RSspatialID& spatialID, const RSspatial& spatial)
{
    if(spatialID.isValid())
    {
        ispatialMap[spatialID].spatial = spatial;
        
        return true;
    }
    
    return false;
}

RSresult VkRenderSystem::spatialDispose(const RSspatialID& spatialID) 
{
    assert(spatialID.isValid() && "input spatial ID is invalid");

    if (spatialAvailable(spatialID)) 
    {
        ispatialMap.erase(spatialID);

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

bool VkRenderSystem::textureAvailable(const RStextureID& texID) 
{
    return texID.isValid() && itextureMap.find(texID) != itextureMap.end();
}

RSresult VkRenderSystem::textureCreate(RStextureID& outTexID, const char* absfilepath) 
{
    RSuint id;
    bool success = itextureIDpool.CreateID(id);
    assert(success && "failed to create a texture ID");
    if (success) 
    {
        VkRStexture vkrstex;
        vkrstex.absPath = absfilepath;
        vkrstex.texinfo = TextureLoader::readTexture(absfilepath);
        createTextureImage(vkrstex);
        createTextureImageView(vkrstex);
        createTextureSampler(vkrstex);
        outTexID.id = id;
        itextureMap[outTexID] = vkrstex;

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::texture3dCreate(RStextureID& outTexID, const RStextureInfo& texInfo)
{
    assert(texInfo.textureType == RStextureType::ttTexture3D && "invalid texture type");
    assert(texInfo.texelFormat != RStextureFormat::vsInvalid && "unset texture format");
    RSuint id;
    bool success = itextureIDpool.CreateID(id);
    assert(success && "failed to create a texture ID");
    if (success) 
    {
        VkRStexture vkrstex;
        vkrstex.texinfo = texInfo;
        createTextureImage(vkrstex);
        createTextureImageView(vkrstex);
        createTextureSampler(vkrstex);
        outTexID.id = id;
        itextureMap[outTexID] = vkrstex;

        return RSresult::SUCCESS;
    }
    
    return RSresult::FAILURE;
}

RSresult VkRenderSystem::textureCreateFromMemory(RStextureID& outTexID, unsigned char* encodedTexData, uint32_t width, uint32_t height) 
{
    RSuint id;
    bool success = itextureIDpool.CreateID(id);
    assert(success && "failed to create a texture ID");
    if (success) 
    {
        VkRStexture vkrstex;
        vkrstex.texinfo = TextureLoader::readFromMemory(encodedTexData, width, height);
        createTextureImage(vkrstex);
        createTextureImageView(vkrstex);
        createTextureSampler(vkrstex);
        outTexID.id = id;
        itextureMap[outTexID] = vkrstex;

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::textureDispose(const RStextureID& texID) 
{
    assert(texID.isValid() && "input texture ID is invalid");
    
    if (textureAvailable(texID)) 
    {
        VkRStexture& vkrstex = itextureMap[texID];
        vkrstex.texinfo.dispose();
        
        vkDestroyImage(iinstance.device, vkrstex.textureImage, nullptr);
        vkFreeMemory(iinstance.device, vkrstex.textureImageMemory, nullptr);
        vkDestroySampler(iinstance.device, vkrstex.textureSampler, nullptr);
        vkDestroyImageView(iinstance.device, vkrstex.textureImageView, nullptr);

        itextureMap.erase(texID);

        return RSresult::SUCCESS;
    }
    
    return RSresult::FAILURE;
}

bool VkRenderSystem::stateAvailable(const RSstateID& stateID) 
{
    return stateID.isValid() && istateMap.find(stateID) != istateMap.end();
}

RSresult VkRenderSystem::stateCreate(RSstateID& outStateID, const RSstate& state) 
{
    RSuint id;
    bool success = istateIDpool.CreateID(id);
    assert(success && "failed to create a state ID");
    if (success) 
    {
        VkRSstate vkrsstate;
        vkrsstate.state= state;

        outStateID.id = id;
        istateMap[outStateID] = vkrsstate;

        return RSresult::SUCCESS;
    }

    return RSresult::FAILURE;
}

RSresult VkRenderSystem::stateDispose(const RSstateID& stateID) 
{
    assert(stateID.isValid() && "input state ID is invalid");

    if (stateAvailable(stateID)) 
    {
        istateMap.erase(stateID);

        return RSresult::SUCCESS;
    }
    
    return RSresult::FAILURE;

}
