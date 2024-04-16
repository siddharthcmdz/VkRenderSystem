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

#include "ModelData.h"
#include "VkRenderSystem.h"

namespace ss {
    void* MeshData::getAttribData(const RSvertexAttribute attrib) {
        switch (attrib) {
        case RSvertexAttribute::vaPosition:
            return positions.data();

        case RSvertexAttribute::vaNormal:
            return normals.data();

        case RSvertexAttribute::vaColor:
            return colors.data();

        case RSvertexAttribute::vaTexCoord:
            return texcoords.data();
        }

        return nullptr;
    }

    void MeshData::dispose() {
        auto& vkrs = VkRenderSystem::getInstance();
        vkrs.geometryDataDispose(this->geometryDataID);
        this->geometryDataID.id = INVALID_ID;
        
        vkrs.geometryDispose(this->geometryID);
        this->geometryID.id = INVALID_ID;
        
        localBox = ss::BoundingBox();
        this->positions.clear();
        this->normals.clear();
        this->colors.clear();
        this->texcoords.clear();
    }

    void MeshInstance::dispose() {
        auto& vkrs = VkRenderSystem::getInstance();
        assert(this->instanceID.isValid() && "invalid instance ID");
        if(this->spatialID.isValid()) {
            vkrs.spatialDispose(this->spatialID);
        }
        if(this->stateID.isValid()) {
            //TODO: dispose stateID when API is ready
        }
        if(this->appearanceID.isValid()) {
            vkrs.appearanceDispose(this->appearanceID);
        }
        if(this->diffuseTextureID.isValid()) {
            //TODO: dispose texture when API is ready
        }
    }
}
