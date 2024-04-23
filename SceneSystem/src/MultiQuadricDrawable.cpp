
#include "MultiQuadricDrawable.h"
#include "QuadricDataFactory.h"
#include "VkRenderSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace ss {

    bool MultiQuadricDrawable::initGeometry() 
    {
        //create geometry data and geometry information
        createQuadrics();
        
        return true;
    }
    
    std::vector<RScollectionID> MultiQuadricDrawable::getCollections() const
    {
        return std::vector<RScollectionID>{_collectionID};
    }

    void MultiQuadricDrawable::createQuadrics()
    {

        std::vector<QuadricData> qdatalist = {
            QuadricDataFactory::createCone(),
            QuadricDataFactory::createCylinder(),
            QuadricDataFactory::createDisk(),
            QuadricDataFactory::createSphere(),
            QuadricDataFactory::createQuad(),
        };
        _geomInfos.resize(qdatalist.size());
        
        auto& vkrs = VkRenderSystem::getInstance();
        
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
        RSvertexAttribsInfo attribInfo;
        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
        attribInfo.attributes = attribs.data();
        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
        
        
        for(size_t i = 0; i < qdatalist.size(); i++) 
        {
            GeometryInfo geomInfo;

            uint32_t numVertices = static_cast<uint32_t>(qdatalist[i].positions.size());
            uint32_t numIndices = static_cast<uint32_t>(qdatalist[i].indices.size());

            vkrs.geometryDataCreate(geomInfo.geomDataID, numVertices, numIndices, attribInfo);
            uint32_t posSizeInBytes = numVertices * sizeof(qdatalist[i].positions[0]);
            vkrs.geometryDataUpdateVertices(geomInfo.geomDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)qdatalist[i].positions.data());

            uint32_t normSizeInBytes = numVertices * sizeof(qdatalist[i].normals[0]);
            vkrs.geometryDataUpdateVertices(geomInfo.geomDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)qdatalist[i].normals.data());

            uint32_t colorSizeInBytes = numVertices * sizeof(qdatalist[i].colors[0]);
            vkrs.geometryDataUpdateVertices(geomInfo.geomDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)qdatalist[i].colors.data());

            uint32_t texcoordSizeInBytes = numVertices * sizeof(qdatalist[i].texcoords[0]);
            vkrs.geometryDataUpdateVertices(geomInfo.geomDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)qdatalist[i].texcoords.data());

            uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
            vkrs.geometryDataUpdateIndices(geomInfo.geomDataID, 0, indicesSizeInBytes, (void*)qdatalist[i].indices.data());
            vkrs.geometryDataFinalize(geomInfo.geomDataID);
            
            RSgeometryInfo geometry;
            geometry.primType = RSprimitiveType::ptTriangle;
            vkrs.geometryCreate(geomInfo.geomID, geometry);
            
            _geomInfos[i] = geomInfo;
            
            glm::vec4 minpt = qdatalist[i].bbox.getmin();
            glm::vec4 maxpt = qdatalist[i].bbox.getmax();
            _bbox.expandBy(minpt);
            _bbox.expandBy(maxpt);
        }
    }

    bool MultiQuadricDrawable::initView() 
    {
        
        auto& vkrs = VkRenderSystem::getInstance();
        RScollectionInfo collInfo;
        collInfo.maxInstances = static_cast<uint32_t>(_geomInfos.size());
        vkrs.collectionCreate(_collectionID, collInfo);
        
        RSappearanceInfo appInfo;
        RSappearanceID appID;
        appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
        vkrs.appearanceCreate(appID, appInfo);
        
        float width = _bbox.getDiagonal();
        float start = -width * 0.5f;
        float quadWidth = float(width)/float(_geomInfos.size());
        
        //create instances;
        for(size_t i = 0; i < _geomInfos.size(); ++i) {
            InstanceData instData;
            instData.appearanceID = appID;
            
            RSspatial spatial;
            std::cout<<"Start pos: "<<start<<std::endl;
            spatial.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, start));
            spatial.modelInv = glm::inverse(spatial.model);
            start += quadWidth;
            vkrs.spatialCreate(instData.spatialID, spatial);

            GeometryInfo geomInfo = _geomInfos[i];
            RSinstanceInfo instInfo;
            instInfo.appID = instData.appearanceID;
            instInfo.geomID = geomInfo.geomID;
            instInfo.gdataID = geomInfo.geomDataID;
            instInfo.spatialID = instData.spatialID;
            
            _instances.push_back(instData);
            
            vkrs.collectionInstanceCreate(_collectionID, instData.instanceID, instInfo);
        }
        vkrs.collectionFinalize(_collectionID);

        return true;
    }

    bool MultiQuadricDrawable::init()
    {
        initGeometry();
        initView();
        
        return true;
    }

    bool MultiQuadricDrawable::dispose()
    {
        disposeGeometry();
        disposeView();
        
        return true;
    }

    bool MultiQuadricDrawable::disposeGeometry() 
    {
        auto& vkrs = VkRenderSystem::getInstance();
        
        for(size_t i = 0; i < _geomInfos.size(); i++) 
        {
            if(_geomInfos[i].geomID.isValid()) 
            {
                vkrs.geometryDispose(_geomInfos[i].geomID);
            }
            if(_geomInfos[i].geomDataID.isValid()) 
            {
                vkrs.geometryDataDispose(_geomInfos[i].geomDataID);
            }
        }
        _geomInfos.clear();
        
        return true;
    }

    bool MultiQuadricDrawable::disposeView()
    {
        auto& vkrs = VkRenderSystem::getInstance();
        if(_collectionID.isValid()) 
        {
            //You dont have to dispose individual instances, just dispose the entire collection.
            vkrs.collectionDispose(_collectionID);
        }
        
        for(auto& instData : _instances)
        {
            if(instData.spatialID.isValid())
            {
                vkrs.spatialDispose(instData.spatialID);
            }
            if(instData.stateID.isValid()) 
            {
                //TODO: dispose state when API is ready
            }
            if(instData.diffuseTextureID.isValid()) 
            {
                //TODO: dispose texture when API is ready
            }
        }
        _instances.clear();
        
        return true;
    }

    BoundingBox MultiQuadricDrawable::getBounds() 
    {
        return _bbox;
    }

    std::string MultiQuadricDrawable::getName() const 
    {
        return getDrawableName(DrawableType::dtMultiQuadrics);
    }

    DrawableType MultiQuadricDrawable::getType() const 
    {
        return DrawableType::dtMultiQuadrics;
        
    }
}
