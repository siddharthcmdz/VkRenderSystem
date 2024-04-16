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

#include "VolumeSliceDrawable.h"
#include <string>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "VkRenderSystem.h"
#include "QuadricDataFactory.h"
#include "RenderableUtils.h"
#include <glm/gtc/matrix_transform.hpp>

namespace ss
{
    VolumeSliceData VolumeSliceDrawable::createVolumeSlice(const VolumeSliceOrientation vso, const VolumeModel& volumeModel, const RScollectionID& collectionID)
    {
        VolumeSliceData vsd;
        
        vsd.sliceInfo.volumeModel = volumeModel;
        glm::mat4 volcenter = glm::mat4(1.0f);
        switch (vso)
        {
            case VolumeSliceOrientation::vsoAxial:
                vsd.sliceInfo.sliceNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                volcenter = glm::translate(glm::mat4(1.0f), glm::vec3(_volumeSize*0.5f, _volumeSize*0.5, 0.0f));
                break;
                
            case VolumeSliceOrientation::vsoCoronal:
                vsd.sliceInfo.sliceNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                volcenter = glm::translate(glm::mat4(1.0f), glm::vec3(_volumeSize*0.5f, 0.0f, _volumeSize*0.5));
                break;
                
            case VolumeSliceOrientation::vsoSagittal:
                vsd.sliceInfo.sliceNormal = glm::vec3(1.0f, 0.0f, 0.0f);
                volcenter = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, _volumeSize*0.5f, _volumeSize*0.5));
                break;
                
            default:
                break;
        }
        
        vsd.sliceInfo.width = _volumeSize;
        vsd.sliceInfo.height = _volumeSize;
        
        //center the volume to the origin
        vsd.sliceInfo.spatial.model = glm::mat4(1.0f);
        glm::mat4 volscale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/volumeModel.info.width, 1.0f/volumeModel.info.height, 1.0f/volumeModel.info.depth));
        glm::mat4 textureMat = volscale * volcenter;
        
        vsd.sliceInfo.spatial.texture = textureMat;
        
        vsd.sliceID = RenderableUtils::volumeSliceCreate(vsd.sliceInfo, _collectionID);
        
        return vsd;
    }

    bool VolumeSliceDrawable::init()
    {
        auto& vkrs = VkRenderSystem::getInstance();

        VolumeModel volumeModel;
        volumeModel.info = TextureLoader::create3Dvolume(_volumeSize, _volumeSize, _volumeSize, RStextureFormat::tfUnsignedShort);
        vkrs.texture3dCreate(volumeModel.textureID, volumeModel.info);

        float halfsz = _volumeSize*0.5f;
        _bbox = BoundingBox(glm::vec4(-halfsz, -halfsz, -halfsz, 1.0f), glm::vec4(halfsz, halfsz, halfsz, 1.0f));
        
        RScollectionInfo collectionInfo;
        collectionInfo.collectionName = "VolumeSlices";
        collectionInfo.maxInstances = 3;
        
        vkrs.collectionCreate(_collectionID, collectionInfo);
        
        _volumeSliceMap[VolumeSliceOrientation::vsoAxial] = createVolumeSlice(VolumeSliceOrientation::vsoAxial, volumeModel, _collectionID);
        _volumeSliceMap[VolumeSliceOrientation::vsoSagittal] = createVolumeSlice(VolumeSliceOrientation::vsoSagittal, volumeModel, _collectionID);
        _volumeSliceMap[VolumeSliceOrientation::vsoCoronal] = createVolumeSlice(VolumeSliceOrientation::vsoCoronal, volumeModel, _collectionID);

                    
        vkrs.collectionFinalize(_collectionID);
        
        return true;
    }

    bool VolumeSliceDrawable::dispose()
    {
        for(const auto& iter : _volumeSliceMap)
        {
            RenderableUtils::volumeSliceDispose(iter.second.sliceID, _collectionID);
        }
        auto& vkrs = VkRenderSystem::getInstance();
        vkrs.collectionDispose(_collectionID);
        return true;
    }

    BoundingBox VolumeSliceDrawable::getBounds() 
    {
        return _bbox;
    }

    std::vector<RScollectionID> VolumeSliceDrawable::getCollections() const
    {
        return std::vector<RScollectionID>{ _collectionID };
    }

    std::string VolumeSliceDrawable::getName() const
    {
        return getDrawableName(DrawableType::dtVolumeSlice);
    }

    DrawableType VolumeSliceDrawable::getType() const 
    {
        return DrawableType::dtVolumeSlice;
    }
}

