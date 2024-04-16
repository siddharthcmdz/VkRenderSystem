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

#include "VkRSdataTypes.h"

VkFormat getVkFormat(const RSvertexAttribute& va) {
    switch (va) {
        case RSvertexAttribute::vaPosition:
        case RSvertexAttribute::vaColor:
        case RSvertexAttribute::vaNormal:
            return VK_FORMAT_R32G32B32A32_SFLOAT;

        case RSvertexAttribute::vaTexCoord:
            return VK_FORMAT_R32G32_SFLOAT;

        default:
            break;
    }

    return VkFormat::VK_FORMAT_MAX_ENUM;
}

uint32_t getOffset(uint32_t numAttribs, uint32_t idx) {
    uint32_t curroffset = 0;
    for (size_t i = 0; i < numAttribs; i++) {
        if (idx == i) {
            return curroffset;
        }

        curroffset += sizeof(glm::vec4);
    }

    return curroffset;
}
