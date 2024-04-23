//
//#include "GLTFmodelDrawable.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include "VkRenderSystem.h"
//
//#include <iostream>
//#include "ModelData.h"
//#include "GLTFmodelLoader.h"
//#include "AssetFolder.h"
//
//namespace ss 
//{
//
//    bool GLTFmodelDrawable::initGeometry() 
//    {
//        std::string modelPathStub = AssetFolder::getModelFolder();
//        _modelPath = modelPathStub + "/101-0008.glb";
//        
//        _meshDataMap = GLTFmodelLoader::loadModelGeometry(_modelPath);
//        if(_meshDataMap.empty()) {
//            return false;
//        }
//        
//        return true;
//    }
//
//    bool GLTFmodelDrawable::initView()
//    {
//        auto& vkrs = VkRenderSystem::getInstance();
//        RScollectionInfo collInfo;
//        collInfo.maxInstances = 1;
//        vkrs.collectionCreate(_modelData.collectionID, collInfo);
//        
//        GLTFmodelLoader::loadModelInstance(_modelPath, _meshDataMap, _modelData);
//        
//        vkrs.collectionFinalize(_modelData.collectionID);
//
//        if(_modelData.meshInstances.empty()) 
//        {
//            return false;
//        }
//        
//        return true;
//    }
//    
//    bool GLTFmodelDrawable::init()
//    {
//        initGeometry();
//        initView();
//        
//        return true;
//    }
//
//    bool GLTFmodelDrawable::dispose()
//    {
//        disposeGeometry();
//        disposeView();
//        
//        return true;
//    }
//
//    std::vector<RScollectionID> GLTFmodelDrawable::getCollections() const
//    {
//        return std::vector<RScollectionID> {_modelData.collectionID};
//    }
//
//    bool GLTFmodelDrawable::disposeView()
//    {
//        //You dont have to dispose individual instances, just dispose the entire collection.
//        if(_modelData.collectionID.isValid())
//        {
//            auto& vkrs = VkRenderSystem::getInstance();
//            vkrs.collectionDispose(_modelData.collectionID);
//        }
//        
//        //Dispose mesh instances
//        uint32_t numMeshInsts = static_cast<uint32_t>(_modelData.meshInstances.size());
//        for(MeshInstance& mi : _modelData.meshInstances) 
//        {
//            mi.dispose();
//        }
//        
//        return true;
//    }
//
//    bool GLTFmodelDrawable::disposeGeometry() 
//    {
//        for(auto& iter : _meshDataMap) 
//        {
//            for(auto& iter1 : iter.second) 
//            {
//                iter1.dispose();
//            }
//        }
//        _meshDataMap.clear();
//        
//        return true;
//    }
//
//    BoundingBox GLTFmodelDrawable::getBounds() 
//    {
//        return _bbox;
//    }
//
//    std::string GLTFmodelDrawable::getName() const 
//    {
//        return getDrawableName(DrawableType::dtGLTFmodel);
//    }
//
//    DrawableType GLTFmodelDrawable::getType() const 
//    {
//        return DrawableType::dtGLTFmodel;
//    }
//
//}
//
