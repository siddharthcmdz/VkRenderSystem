//
//#include "MainDrawable.h"
//#include "VkRenderSystem.h"
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/string_cast.hpp>
//#include <glm/gtx/matrix_decompose.hpp>
//#include <glm/gtx/quaternion.hpp>
//#include <glm/gtx/euler_angles.hpp>
//#include <iostream>
//
//namespace ss
//{
//
//    bool MainDrawable::init()
//    {
//        return true;
//    }
//
//    void MainDrawable::loadVolume(const ss::VolumeModel& volumeModel, const RSvolumeSliceAppearance& volumeAppearance)
//    {
//        auto& vkrs = VkRenderSystem::getInstance();
//        //TODO: handle disposing previous volume data
//        if(!_volumeSliceCollectionID.isValid())
//        {
//            RScollectionInfo collInfo;
//            collInfo.collectionName = "Volume Slices";
//            collInfo.maxInstances = 3;
//            
//            vkrs.collectionCreate(_volumeSliceCollectionID, collInfo);
//        }
//        
//        _volumeSliceMap[VolumeSliceOrientation::vsoAxial] = createVolumeSlice(VolumeSliceOrientation::vsoAxial, volumeModel, volumeAppearance);
//        _volumeSliceMap[VolumeSliceOrientation::vsoCoronal] = createVolumeSlice(VolumeSliceOrientation::vsoCoronal, volumeModel, volumeAppearance);
//        _volumeSliceMap[VolumeSliceOrientation::vsoSagittal] = createVolumeSlice(VolumeSliceOrientation::vsoSagittal, volumeModel, volumeAppearance);
//        
//        //set initial default location of slizes in the middle of the volume
//        const static float DEFAULT_AXIAL_POSITION = 0.5f;
//        const static float DEFAULT_SAGITTAL_POSITION = 0.5f;
//        const static float DEFAULT_CORONAL_POSITION = 0.5f;
//        float volWidthf = static_cast<float>(volumeModel.info.width);
//        float volHeightf = static_cast<float>(volumeModel.info.height);
//        float volDepthf = static_cast<float>(volumeModel.info.depth);
//        BoundingBox volumeBox(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(volWidthf, volHeightf, volDepthf, 1.0f));
//        _currVolOffset.axial = static_cast<uint32_t>((volumeBox.getmax().z - volumeBox.getmin().z) * DEFAULT_AXIAL_POSITION);
//        _currVolOffset.sagittal = static_cast<uint32_t>((volumeBox.getmax().x - volumeBox.getmin().x) * DEFAULT_SAGITTAL_POSITION);
//        _currVolOffset.coronal = static_cast<uint32_t>((volumeBox.getmax().y - volumeBox.getmin().y) * DEFAULT_CORONAL_POSITION);
//        
//        updateVolumeOffset(_currVolOffset);
//        
//        vkrs.collectionFinalize(_volumeSliceCollectionID);
//        
//        float volwidthf = static_cast<float>(volumeModel.info.width);
//        float volheightf = static_cast<float>(volumeModel.info.height);
//        float voldepthf = static_cast<float>(volumeModel.info.depth);
//        ss::BoundingBox volbox(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(volwidthf, volheightf, voldepthf, 1.0f));
//        _bbox = volbox.xform(_toMetersScaleMat);
//  
//        //Initialize collection for the instruments
//        RScollectionInfo collInfo;
//        collInfo.maxInstances = _maxInstruments;
//        collInfo.collectionName = "Instruments";
//        vkrs.collectionCreate(_instrumentsCollectionID, collInfo);
//    }
//
//    void MainDrawable::enableVolumeSliceTracker(bool onOff)
//    {
//        _volumeSlicesEnableMarker = onOff;
//    }
//
//    VolumeSliceID MainDrawable::getSliceID(const VolumeSliceOrientation& vso) const
//    {
//        assert(vso != VolumeSliceOrientation::vsoInvalid && "invalid volume slice orientation");
//        
//        VolumeSliceID vsid;
//        if(_volumeSliceMap.find(vso) != _volumeSliceMap.end())
//        {
//            vsid = _volumeSliceMap.at(vso).sliceID;
//        }
//        return vsid;
//    }
//
//    void MainDrawable::updateVolumeOffset(const VolumeOffset& voloff)
//    {
//        for(auto& iter : _volumeSliceMap)
//        {
//            VolumeSliceOrientation vso = iter.first;
//            VolumeSliceData& vsd = iter.second;
//            
//            float volWidthf = static_cast<float>(vsd.sliceInfo.volumeModel.info.width);
//            float volHeightf = static_cast<float>(vsd.sliceInfo.volumeModel.info.height);
//            float volDepthf = static_cast<float>(vsd.sliceInfo.volumeModel.info.depth);
//            BoundingBox volumeBox(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(volWidthf, volHeightf, volDepthf, 1.0f));
//
//            switch (vso)
//            {
//                case VolumeSliceOrientation::vsoAxial:
//                {
//                    float zdist = static_cast<float>(voloff.axial);
//                    vsd.sliceTransMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zdist));
//
//                    float znormalized = zdist / (volumeBox.getmax().z - volumeBox.getmin().z);
//                    vsd.voxelTrans = glm::vec3(0.0f, 0.0f, znormalized);
//                    break;
//                }
//                    
//                case VolumeSliceOrientation::vsoSagittal:
//                {
//                    float xdist = static_cast<float>(voloff.sagittal);
//                    vsd.sliceTransMat = glm::translate(glm::mat4(1.0f), glm::vec3(xdist, 0.0f, 0.0f));
//
//                    float xnormalized = xdist / (volumeBox.getmax().x - volumeBox.getmin().x);
//                    vsd.voxelTrans = glm::vec3(xnormalized, 0.0f, 0.0f);
//                    break;
//                }
//                    
//                case VolumeSliceOrientation::vsoCoronal:
//                {
//                    float ydist = static_cast<float>(voloff.coronal);
//                    vsd.sliceTransMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, ydist, 0.0f));
//
//                    float ynormalized = ydist / (volumeBox.getmax().y - volumeBox.getmin().y);
//                    vsd.voxelTrans = glm::vec3(0.0f, ynormalized, 0.0f);
//                    break;
//                }
//                    
//                default:
//                    break;
//            }
//            
//            vsd.sliceInfo.spatial.model = _toMetersScaleMat * vsd.sliceOriginToPrevMat * vsd.sliceTransMat * vsd.sliceInfo.volumeModel.toLPS * vsd.sliceToOriginMat;
//
//            //scale the 3d texture to unity.
//            glm::mat4 voxelScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/volWidthf, 1.0f/volHeightf, 1.0f/volDepthf));
//            glm::vec3 halfVoxelOffset = glm::vec3(0.5f, 0.5f, 0.5f);
//            glm::mat4 voxelTransToOriginMat = glm::translate(glm::mat4(1.0f), -halfVoxelOffset);
//            glm::mat4 voxelOriginToMat = glm::translate(glm::mat4(1.0f), halfVoxelOffset);
//            glm::mat4 voxelTransMat = glm::translate(glm::mat4(1.0f), vsd.voxelTrans);
//            glm::mat4 textureMat = voxelOriginToMat * voxelTransMat * voxelTransToOriginMat * voxelScaleMat;
//            vsd.sliceInfo.spatial.texture = textureMat;
//            
//            RenderableUtils::volumeSliceUpdate(vsd.sliceID, vsd.sliceInfo.spatial);
//        }
//    }
//
//    VolumeSliceData MainDrawable::createVolumeSlice(const VolumeSliceOrientation vso, const VolumeModel& volumeModel, const RSvolumeSliceAppearance& vsapp)
//    {
//        VolumeSliceData vsd;
//        /**
//         * Currently we are rendering 3 orthogonal slices with hardcoded positions to be in the center of axial, sagittal and coronal. The voxel positions are controlled by the texture matrix and the spatial location of the slice itself is controlled by the model matrix. The slices are scaled to 2 units and translated by -1 so that the bounds of the world is from -1 to +1 in all dimensions. Its just easier that way and also doesnt go past beyond the far clip plane. When it comes to oblique cases, this is moot.
//         */
//        vsd.sliceInfo.volumeModel = volumeModel;
//        float volWidthf = static_cast<float>(volumeModel.info.width);
//        float volHeightf = static_cast<float>(volumeModel.info.height);
//        float volDepthf = static_cast<float>(volumeModel.info.depth);
//
//        
//        glm::vec3 scanScale = glm::vec3(vsd.sliceInfo.volumeModel.scanScale);
//        _toMetersScaleVec = glm::vec3(scanScale.x/volWidthf, scanScale.y/volHeightf, scanScale.z/volDepthf);
//        _toMetersScaleMat = glm::scale(glm::mat4(1.0f), _toMetersScaleVec);
//        
//        float maxDim = std::max(std::max(volWidthf, volHeightf), volDepthf);
//        vsd.sliceInfo.width = maxDim*4;
//        vsd.sliceInfo.height = maxDim*4;
//        
//        float halfwidth = vsd.sliceInfo.width * 0.5f;
//        float halfheight = vsd.sliceInfo.height * 0.5f;
//
//        switch (vso)
//        {
//            case VolumeSliceOrientation::vsoAxial:
//            {
//                vsd.sliceInfo.sliceNormal = glm::vec3(0.0f, 0.0f, 1.0f);
//                
//                glm::vec3 sliceToOrigin = glm::vec3(-halfwidth, -halfheight, 0.0f);
//                glm::vec3 sliceOriginToPrev = glm::vec3(halfwidth, halfheight, 0.0f);
//                vsd.sliceToOriginMat = glm::translate(glm::mat4(1.0f), sliceToOrigin);
//                vsd.sliceOriginToPrevMat = glm::translate(glm::mat4(1.0f), sliceOriginToPrev);
//                
//                vsd.sliceInfo.name = "AxialVolumeSlice";
//                break;
//            }
//                
//            case VolumeSliceOrientation::vsoSagittal:
//            {
//                vsd.sliceInfo.sliceNormal = glm::vec3(1.0f, 0.0f, 0.0f);
//                
//                glm::vec3 sliceToOrigin = glm::vec3(0.0f, -halfwidth, -halfheight);
//                glm::vec3 sliceOriginToPrev = glm::vec3(0.0f, halfwidth, halfheight);
//                vsd.sliceToOriginMat = glm::translate(glm::mat4(1.0f), sliceToOrigin);
//                vsd.sliceOriginToPrevMat = glm::translate(glm::mat4(1.0f), sliceOriginToPrev);
//
//                vsd.sliceInfo.name = "SagittalVolumeSlice";
//                break;
//            }
//
//            case VolumeSliceOrientation::vsoCoronal:
//            {
//                vsd.sliceInfo.sliceNormal = glm::vec3(0.0f, 1.0f, 0.0f);
//                
//                glm::vec3 sliceToOrigin = glm::vec3(-halfwidth, 0.0f, -halfheight);
//                glm::vec3 sliceOriginToPrev = glm::vec3(halfwidth, 0.0f, halfheight);
//                vsd.sliceToOriginMat = glm::translate(glm::mat4(1.0f), sliceToOrigin);
//                vsd.sliceOriginToPrevMat = glm::translate(glm::mat4(1.0f), sliceOriginToPrev);
//
//                vsd.sliceInfo.name = "CoronalVolumeSlice";
//                break;
//            }
//                
//            default:
//                break;
//        }
//
//        vsd.sliceInfo.appearance = vsapp;
//
//        vsd.sliceID = RenderableUtils::volumeSliceCreate(vsd.sliceInfo, _volumeSliceCollectionID);
//        
//        vsd.sliceInfo.spatial.model = _toMetersScaleMat * vsd.sliceOriginToPrevMat * vsd.sliceTransMat * vsd.sliceInfo.volumeModel.toLPS * vsd.sliceToOriginMat;
//
//        //scale the 3d texture to unity.
//        glm::mat4 voxelScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/volWidthf, 1.0f/volHeightf, 1.0f/volDepthf));
//        glm::vec3 halfVoxelOffset = glm::vec3(0.5f, 0.5f, 0.5f);
//        glm::mat4 voxelTransToOriginMat = glm::translate(glm::mat4(1.0f), -halfVoxelOffset);
//        glm::mat4 voxelOriginToMat = glm::translate(glm::mat4(1.0f), halfVoxelOffset);
//        glm::mat4 voxelTransMat = glm::translate(glm::mat4(1.0f), vsd.voxelTrans);
//        glm::mat4 textureMat = voxelOriginToMat * voxelTransMat * voxelTransToOriginMat * voxelScaleMat;
//        vsd.sliceInfo.spatial.texture = textureMat;
//        
//        RenderableUtils::volumeSliceUpdate(vsd.sliceID, vsd.sliceInfo.spatial);
//        
//        return vsd;
//    }
//
//    void MainDrawable::resetViews()
//    {
//        //Reset all transforms of the volume slice to be at the center of the volume
//        for(auto& iter : _volumeSliceMap)
//        {
//            VolumeSliceData& vsd = iter.second;
//            vsd.sliceInfo.spatial.model = _toMetersScaleMat * vsd.sliceOriginToPrevMat * vsd.sliceTransMat * vsd.sliceInfo.volumeModel.toLPS * vsd.sliceToOriginMat;
//            float volWidthf = static_cast<float>(vsd.sliceInfo.volumeModel.info.width);
//            float volHeightf = static_cast<float>(vsd.sliceInfo.volumeModel.info.height);
//            float volDepthf = static_cast<float>(vsd.sliceInfo.volumeModel.info.depth);
//            
//            //scale the 3d texture to unity.
//            glm::mat4 voxelScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/volWidthf, 1.0f/volHeightf, 1.0f/volDepthf));
//            glm::vec3 halfVoxelOffset = glm::vec3(0.5f, 0.5f, 0.5f);
//            glm::mat4 voxelTransToOriginMat = glm::translate(glm::mat4(1.0f), -halfVoxelOffset);
//            glm::mat4 voxelOriginToMat = glm::translate(glm::mat4(1.0f), halfVoxelOffset);
//            glm::mat4 voxelTransMat = glm::translate(glm::mat4(1.0f), vsd.voxelTrans);
//            glm::mat4 textureMat = voxelOriginToMat * voxelTransMat * voxelTransToOriginMat * voxelScaleMat;
//            vsd.sliceInfo.spatial.texture = textureMat;
//            
//            VolumeSliceID sliceID = vsd.sliceID;
//            RenderableUtils::volumeSliceUpdate(sliceID, vsd.sliceInfo.spatial);
//        }
//        
//        //Hide the instrument
//        hideAllInstruments();
//    }
//
//    void MainDrawable::updateVolumeSliceAppearances(const RSvolumeSliceAppearance& volumeSliceAppearance)
//    {
//        for(auto& iter : _volumeSliceMap)
//        {
//            const VolumeSliceID& sliceID = iter.second.sliceID;
//            iter.second.sliceInfo.appearance = volumeSliceAppearance;
//            RenderableUtils::volumeSliceUpdate(sliceID, volumeSliceAppearance);
//        }
//    }
//
//    void MainDrawable::hideAllInstruments()
//    {
//        auto& vkrs = VkRenderSystem::getInstance();
//        for(const auto& iter : _assetModelMap)
//        {
//            std::vector<RSinstanceID> instIDList = RenderableUtils::modelGetInstances(iter.second.modelID);
//            for(const RSinstanceID& instID : instIDList)
//            {
//                vkrs.collectionInstanceHide(_instrumentsCollectionID, instID, true);
//            }
//        }
//        
//        _activeInstrumentID = INVALID_ID;
//    }
//
//    void MainDrawable::setActiveInstrument(uint32_t instrumentID)
//    {
//        if(_assetModelMap.find(instrumentID) == _assetModelMap.end())
//        {
//            return;
//        }
//        
//        if(_activeInstrumentID == _assetModelMap[instrumentID].modelID)
//        {
//            return;
//        }
//        
//        if(instrumentID != INVALID_ID) 
//        {
//            _volumeSlicesConstrained = true;
//        }
//        else
//        {
//            //TODO: reset the slices to orthogonal directions respectively.
//            return;
//        }
//        
//        std::cout<<"setActiveInstrument: "<<instrumentID<<std::endl;
//        auto& vkrs = VkRenderSystem::getInstance();
//        //First, hide all instances
//        hideAllInstruments();
//        
//        //Next, only unhide the active instrument.
//        const AssetModelID& modelID = _assetModelMap[instrumentID].modelID;
//        std::vector<RSinstanceID> instanceList = RenderableUtils::modelGetInstances(modelID);
//        for(const RSinstanceID& instID : instanceList)
//        {
//            vkrs.collectionInstanceHide(_instrumentsCollectionID, instID, false);
//        }
//        _activeInstrumentID = _assetModelMap[instrumentID].modelID;
//
//    }
//
//    void MainDrawable::addInstrument(uint32_t instrumentID, const InMemoryModel& imm)
//    {
//        if(_assetModelMap.find(instrumentID) == _assetModelMap.end())
//        {
//            auto& vkrs = VkRenderSystem::getInstance();
//            AssetModelData amd;
//            std::string instrumentStr = "instrument_"+std::to_string(instrumentID);
//            amd.modelID = RenderableUtils::modelCreate(imm.memModel, imm.memcnt, instrumentID, _instrumentsCollectionID, instrumentStr);
//            amd.bboxwc = RenderableUtils::modelGetBounds(amd.modelID);
//
//            _assetModelMap[instrumentID] = amd;
//            
//            vkrs.collectionFinalize(_instrumentsCollectionID);
//        }
//    }
//
//    bool MainDrawable::dispose()
//    {
//        //Dispose all the volume slices.
//        for(auto& iter : _volumeSliceMap)
//        {
//            RenderableUtils::volumeSliceDispose(iter.second.sliceID, _volumeSliceCollectionID);
//        }
//        
//        //Dispose all the instrument models.
//        for(const auto& iter : _assetModelMap)
//        {
//            const AssetModelID& modelID = iter.second.modelID;
//            RenderableUtils::modelDispose(modelID, _instrumentsCollectionID);
//        }
//
//        auto& vkrs = VkRenderSystem::getInstance();
//        if(_volumeSliceCollectionID.isValid())
//        {
//            vkrs.collectionDispose(_volumeSliceCollectionID);
//        }
//        if(_instrumentsCollectionID.isValid())
//        {
//            vkrs.collectionDispose(_instrumentsCollectionID);
//        }
//        
//        return true;
//    }
//
//    std::vector<RScollectionID> MainDrawable::getCollections() const
//    {
//        std::vector<RScollectionID> collectionList;
//        if(_volumeSliceCollectionID.isValid())
//        {
//            collectionList.push_back(_volumeSliceCollectionID);
//        }
//        
//        if(_instrumentsCollectionID.isValid())
//        {
//            collectionList.push_back(_instrumentsCollectionID);
//        }
//        
//        return collectionList;
//    }
//
//    BoundingBox MainDrawable::getBounds()
//    {
//        return _bbox;
//    }
//
//    std::string MainDrawable::getName() const
//    {
//        return getDrawableName(DrawableType::dtMain);
//    }
//
//    DrawableType MainDrawable::getType() const
//    {
//        return DrawableType::dtMain;
//    }
//
//    glm::mat4 MainDrawable::constrainTranslation(glm::mat4 xform, const Axis& axis) const
//    {
//        glm::mat4 constrained = xform;
//        switch(axis)
//        {
//            case ss::Axis::X:
//            {
//                constrained[3][1] = 0.0f;
//                constrained[3][2] = 0.0f;
//            }
//            break;
//            
//            case ss::Axis::Y:
//            {
//                constrained[3][0] = 0.0f;
//                constrained[3][2] = 0.0f;
//            }
//            break;
//            
//            case ss::Axis::Z:
//            {
//                constrained[3][0] = 0.0f;
//                constrained[3][1] = 0.0f;
//            }
//            break;
//                
//            case ss::Axis::Invalid:
//            default:
//                throw std::runtime_error("Invalid axis for constraining");
//        }
//        
//        return constrained;
//    }
//
//    glm::mat4 getTranslation(glm::mat4 xform)
//    {
//        glm::mat4 transmat = glm::mat4(1.0f);
//        
//        glm::vec3 scale;
//        glm::quat rotation;
//        glm::vec3 translation;
//        glm::vec3 skew;
//        glm::vec4 perspective;
//        glm::decompose(xform, scale, rotation, translation, skew, perspective);
//        transmat = glm::translate(glm::mat4(1.0f), translation);
//        
//        return transmat;
//    }
//
//    glm::mat4 getRotation(glm::mat4 xform)
//    {
//        glm::mat4 rotmat = glm::mat4(1.0f);
//        
//        glm::vec3 scale;
//        glm::quat rotation;
//        glm::vec3 translation;
//        glm::vec3 skew;
//        glm::vec4 perspective;
//        glm::decompose(xform, scale, rotation, translation, skew, perspective);
//
//        rotmat = glm::toMat4(rotation);
//        return rotmat;
//    }
//        
//    void MainDrawable::updateInstrumentTransform(uint32_t instrumentID, const glm::mat4& transorig, bool flipInstrumentVertical)
//    {
//        float yaw, pitch, roll;
//        glm::extractEulerAngleXYZ(transorig, pitch, yaw, roll);
//        glm::mat4 xlateorig = getTranslation(transorig);
////        std::cout<<"pitch: "<<glm::degrees(pitch)<<", yaw: "<<glm::degrees(yaw)<<", roll: "<<glm::degrees(roll)<<std::endl;
//        glm::mat4 rotorig = glm::eulerAngleXZ(pitch, roll);
//        glm::mat4 trans = xlateorig * rotorig;
//        
//        glm::mat4 instrumentTrans = trans;
//        if(flipInstrumentVertical)
//        {
//            glm::mat4 defaultYorientationMat =glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//            instrumentTrans = trans * defaultYorientationMat;
//        }
//        
//        std::cout<<"instrument transform: "<<glm::to_string(instrumentTrans)<<std::endl;
//
//        if(_assetModelMap.find(instrumentID) != _assetModelMap.end())
//        {
//            const AssetModelID& amid = _assetModelMap[instrumentID].modelID;
//
//            RSspatial rsspatial;
//            rsspatial.model = instrumentTrans;
//            rsspatial.modelInv = glm::inverse(rsspatial.model);
//            RenderableUtils::modelUpdate(amid, rsspatial);
//        }
//        
//        if(_volumeSlicesConstrained)
//        {
//            VolumeSliceData& axialVSD = _volumeSliceMap[ss::VolumeSliceOrientation::vsoAxial];
//            const VolumeModel& vm = axialVSD.sliceInfo.volumeModel;
//            float volWidthf = static_cast<float>(vm.info.width);
//            float volHeightf = static_cast<float>(vm.info.height);
//            float volDepthf = static_cast<float>(vm.info.depth);
//
//            BoundingBox volumeBox(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(volWidthf, volHeightf, volDepthf, 1.0f));
//
//
//            //update slice rotation for axial.
//            glm::mat4 ztransrot = constrainTranslation(trans, Axis::Z);
//            glm::mat4 metersInverse = glm::inverse(_toMetersScaleMat);
//            
//            axialVSD.sliceInfo.spatial.model = axialVSD.sliceInfo.volumeModel.toLPS * trans * _toMetersScaleMat * axialVSD.sliceToOriginMat;
//            axialVSD.sliceInfo.spatial.modelInv = glm::inverse(axialVSD.sliceInfo.spatial.model);
//                
//            glm::mat4 voxelScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/volWidthf, 1.0f/volHeightf, 1.0f/volDepthf));
//            glm::mat4 axialTextureMat = voxelScaleMat * metersInverse * axialVSD.sliceInfo.spatial.model;
////            glm::mat4 axialTextureMat = voxelScaleMat * volumeTrans * volumeRot;
////            std::cout<<"volumeTrans: "<<glm::to_string(volumeTrans)<<std::endl;
////            std::cout<<"volumeRot: "<<glm::to_string(volumeRot)<<std::endl;
//            axialVSD.sliceInfo.spatial.texture = axialTextureMat;
//            RenderableUtils::volumeSliceUpdate(axialVSD.sliceID, axialVSD.sliceInfo.spatial);
//            
//            //update slice rotation for sagittal
//            VolumeSliceData& sagittalVSD = _volumeSliceMap[ss::VolumeSliceOrientation::vsoSagittal];
////            glm::mat4 xtrans = constrainTranslation(trans, Axis::X);
//            //sagittalVSD.sliceInfo.spatial.model = sagittalVSD.sliceInfo.volumeModel.toLPS * trans * _toMetersScaleMat * sagittalVSD.sliceToOriginMat;
//            sagittalVSD.sliceInfo.spatial.model = sagittalVSD.sliceInfo.volumeModel.toLPS * trans * _toMetersScaleMat * sagittalVSD.sliceToOriginMat;
//            sagittalVSD.sliceInfo.spatial.modelInv = glm::inverse(sagittalVSD.sliceInfo.spatial.model);
//
//            glm::mat4 sagittalTextureMat = voxelScaleMat * metersInverse * sagittalVSD.sliceInfo.spatial.model;
//            sagittalVSD.sliceInfo.spatial.texture = sagittalTextureMat;
//
//            RenderableUtils::volumeSliceUpdate(sagittalVSD.sliceID, sagittalVSD.sliceInfo.spatial);
//
//
//            //update translation for coronal
//            VolumeSliceData& coronalVSD = _volumeSliceMap[ss::VolumeSliceOrientation::vsoCoronal];
//            glm::mat4 ytrans = getTranslation(trans);
//            ytrans = constrainTranslation(ytrans, Axis::Y);
//            
//            coronalVSD.sliceInfo.spatial.model = coronalVSD.sliceInfo.volumeModel.toLPS * ytrans * _toMetersScaleMat * coronalVSD.sliceToOriginMat;
//            coronalVSD.sliceInfo.spatial.modelInv = glm::inverse(coronalVSD.sliceInfo.spatial.model);
//
//            glm::mat4 coronalTextureMat = voxelScaleMat * metersInverse * coronalVSD.sliceInfo.spatial.model;
//            coronalVSD.sliceInfo.spatial.texture = coronalTextureMat;
//
//            RenderableUtils::volumeSliceUpdate(coronalVSD.sliceID, coronalVSD.sliceInfo.spatial);
//        }
//    }
//        
//    RScollectionID MainDrawable::getVolumeSliceCollectionID() const
//    {
//        return _volumeSliceCollectionID;
//    }
//
//    VolumeOffset MainDrawable::getCurrentVolumeOffset() const
//    {
//        return _currVolOffset;
//    }
//
//    uint32_t MainDrawable::getAxialCount() const
//    {
//        return _volumeSliceMap.at(VolumeSliceOrientation::vsoAxial).sliceInfo.volumeModel.info.depth;
//    }
//
//    uint32_t MainDrawable::getSagittalCount() const
//    {
//        return _volumeSliceMap.at(VolumeSliceOrientation::vsoSagittal).sliceInfo.volumeModel.info.width;
//    }
//        
//    uint32_t MainDrawable::getCoronalCount() const
//    {
//        return _volumeSliceMap.at(VolumeSliceOrientation::vsoCoronal).sliceInfo.volumeModel.info.height;
//    }
//    
//    std::array<glm::vec3, 3> MainDrawable::getVolumeSliceFrame(const VolumeSliceOrientation& vso) const
//    {
//        std::array<glm::vec3, 3> frame = RenderableUtils::volumeSliceGetFrame(_volumeSliceMap.at(vso).sliceID);
//        return frame;
//    }
//}
