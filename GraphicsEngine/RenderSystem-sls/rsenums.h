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
#include <string>

/**
* @brief Describes what vertex attributes are being used.
* vaPosition should always use glm::vec4 where w is reserved
* vaNormal should always use glm::vec4 where w is reserved
* vaColor should always use glm::vec4
* vaTexCoord should always use glm::vec2
**/
enum RSvertexAttribute 
{
    vaPosition,
    vaNormal,
    vaColor,
    vaTexCoord
};

/**
 * @brief Describes if the vertex attributes are interleaved or kept separate in device buffers.
 */
enum RSvertexAttributeSettings 
{
    vasInterleaved,
    vasSeparate
};

/**
 * @brief Describes the geometry primitive topology
 */
enum RSprimitiveType 
{
    ptTriangle,
    ptTriangleFan,
    ptTriangleStrip,
    ptPoint,
    ptLine,
    ptLineStrip,
    ptLineLoop
};

/**
 * @brief Describes the shader to use for the collection-instance render
 */
enum RSshaderTemplate 
{
    stOneTriangle,
    stPassthrough,
    stSimpleLit,
    stSimpleTextured,
    stVolumeSlice,
    stLines,
    stMax
};

/**
 * Describes the appearance sub type for what is being rendered
 */
enum RScollectionHint
{
    chVolumeSlice,
    chModel3D,
    chLines,
    chInvalid
};

/**
 * @brief Describes the texture format of the texel being uploaded to the device
 */
enum RStextureFormat 
{
    tfUnsignedBytes,
    tfUnsignedShort,
    tfRGBA8,
    vsInvalid
};

/**
 * @brief Describes the type of the texture worked on.
 */
enum RStextureType 
{
    ttTexture1D,
    ttTexture2D,
    ttTexture3D,
    ttInvalid
};

/**
 * @brief Describes the depth function used for rasterization
 */
enum RSdepthFunction
{
    dsAlway,
    dsLess,
    dsLessEquals,
};

/**
 * @brief Describes one or more templates for configured vulkan descriptor set layouts
 */
enum DescriptorLayoutType
{
    dltViewDefault,
    dltInvalid
};

/**
 * @brief Describes one or more templates for configured vulkan renderpasses
 */
enum RenderPassType
{
    rptViewSimple,
    rptInvalid
};


std::string getShaderStr(const RSshaderTemplate shaderTemplate);
