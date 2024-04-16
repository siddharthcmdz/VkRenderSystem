
#include "QuadricDrawable.h"
#include <string>
#include "VkRenderSystem.h"
#include "QuadricDataFactory.h"

namespace ss 
{
    void QuadricDrawable::createQuadrics()
    {
        QuadricData qdata = QuadricDataFactory::createSphere(0.5f, 5, 5);
        auto& vkrs = VkRenderSystem::getInstance();
        
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
        RSvertexAttribsInfo attribInfo;
        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
        attribInfo.attributes = attribs.data();
        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
        
        uint32_t numVertices = static_cast<uint32_t>(qdata.positions.size());
        uint32_t numIndices = static_cast<uint32_t>(qdata.indices.size());
        
        vkrs.geometryDataCreate(_geomInfo.geomDataID, numVertices, numIndices, attribInfo);
        uint32_t posSizeInBytes = numVertices * sizeof(qdata.positions[0]);
        vkrs.geometryDataUpdateVertices(_geomInfo.geomDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)qdata.positions.data());

        uint32_t normSizeInBytes = numVertices * sizeof(qdata.normals[0]);
        vkrs.geometryDataUpdateVertices(_geomInfo.geomDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)qdata.normals.data());

        uint32_t colorSizeInBytes = numVertices * sizeof(qdata.colors[0]);
        vkrs.geometryDataUpdateVertices(_geomInfo.geomDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)qdata.colors.data());

        uint32_t texcoordSizeInBytes = numVertices * sizeof(qdata.texcoords[0]);
        vkrs.geometryDataUpdateVertices(_geomInfo.geomDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)qdata.texcoords.data());

        uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
        vkrs.geometryDataUpdateIndices(_geomInfo.geomDataID, 0, indicesSizeInBytes, (void*)qdata.indices.data());
        vkrs.geometryDataFinalize(_geomInfo.geomDataID);

        RSgeometryInfo geomInfo;
        geomInfo.primType = RSprimitiveType::ptTriangle;
        vkrs.geometryCreate(_geomInfo.geomID, geomInfo);
        
        _bbox = qdata.bbox;
    }

    bool QuadricDrawable::initGeometry() 
    {
        createQuadrics();
        
        return true;
    }

    bool QuadricDrawable::init()
    {
        initGeometry();
        initView();
        
        return true;
    }

    bool QuadricDrawable::dispose()
    {
        disposeGeometry();
        disposeView();
        
        return true;
    }

    bool QuadricDrawable::initView()
    {
        //Create the collection and instances
        auto& vkrs = VkRenderSystem::getInstance();
        RScollectionInfo collInfo;
        collInfo.maxInstances = 100;
        vkrs.collectionCreate(_collectionID, collInfo);
        
        RSappearanceInfo appInfo;
        appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
        vkrs.appearanceCreate(_instData.appearanceID, appInfo);
        
        RSinstanceInfo instInfo;
        instInfo.gdataID = _geomInfo.geomDataID;
        instInfo.geomID = _geomInfo.geomID;
        instInfo.appID = _instData.appearanceID;

        vkrs.collectionInstanceCreate(_collectionID, _instData.instanceID, instInfo);
        
        vkrs.collectionFinalize(_collectionID);
        
        _isInited = true;
        return true;
    }

    std::vector<RScollectionID> QuadricDrawable::getCollections() const
    {
        return std::vector<RScollectionID> {_collectionID};
    }

    bool QuadricDrawable::disposeView()
    {
        auto& vkrs = VkRenderSystem::getInstance();
        
        if(_collectionID.isValid())
        {
            vkrs.collectionDispose(_collectionID);
        }
        
        return true;
    }

    bool QuadricDrawable::disposeGeometry() 
    {
        auto& vkrs = VkRenderSystem::getInstance();
        if(_geomInfo.geomID.isValid()) 
        {
            vkrs.geometryDispose(_geomInfo.geomID);
        }
        if(_geomInfo.geomDataID.isValid()) 
        {
            vkrs.geometryDataDispose(_geomInfo.geomDataID);
        }
        return true;
    }

    BoundingBox QuadricDrawable::getBounds() 
    {
        return _bbox;
    }

    std::string QuadricDrawable::getName() const 
    {
        return getDrawableName(DrawableType::dtQuadrics);
    }

    DrawableType QuadricDrawable::getType() const 
    {
        return DrawableType::dtQuadrics;
    }
}
