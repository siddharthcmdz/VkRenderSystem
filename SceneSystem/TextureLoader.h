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

#include <cstdint>
#include "rsenums.h"

/**
 * @brief Stores information related to a texture of any dimension.
 */
struct RStextureInfo
{
    void* texels = nullptr;
    RStextureFormat texelFormat = RStextureFormat::tfRGBA8;
    RStextureType textureType = RStextureType::ttTexture2D;
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    uint32_t numChannels = 4;

    void dispose();
};

/**
 * @brief A utility class for loading textures of different formats and dimensions.
 */
class TextureLoader final
{
    public:
    
    /**
     * @brief Creates a fake 3D texture volume using fractal/perlin noise.
     * @param width the specified width of the volume
     * @param height the specified height of the volume
     * @param depth the specified depth of the volume
     * @param textureFormat the specified format of the voxel which can be 8 or 16 bit
     * @return the texture info including the voxel data.
     */
    static RStextureInfo create3Dvolume(uint32_t width, uint32_t height, uint32_t depth, RStextureFormat textureFormat);
    
    /**
     * @brief Reads a 2D texture from a file on storage.
     * @param filepath the absolute path to the image file on storage
     * @return the texture info including the texel data in RGBA format.
     */
    static RStextureInfo readTexture(const char* filepath);
    
    /**
     * @brief Reads a 2D texture that is still encoded and in memory. When loading GLB files, these textures are embedded into the the glb file and read into memory. We use STBI to read in-memory encoded texel data and return the raw texels.
     * @param encodedTexData the specified pointer to the memory containig encoded texel data.
     * @param width the specified width
     * @param height the specified height
     * @return the texture info including the texel data in RGBA format.
     */
    static RStextureInfo readFromMemory(unsigned char* encodedTexData, uint32_t width, uint32_t height);
};
