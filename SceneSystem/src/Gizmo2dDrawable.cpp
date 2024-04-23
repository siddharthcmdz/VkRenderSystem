//
//#include "Gizmo2dDrawable.h"
//#include "QuadricDataFactory.h"
//#include "AssetFolder.h"
//#include "VkRenderSystem.h"
//
//namespace ss 
//{
//    void Gizmo2dDrawable::createGeometry(const QuadricData& qdata) 
//    {
//        auto& vkrs = VkRenderSystem::getInstance();
//        
//        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
//        RSvertexAttribsInfo attribInfo;
//        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
//        attribInfo.attributes = attribs.data();
//        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
//        
//        uint32_t numVertices = static_cast<uint32_t>(qdata.positions.size());
//        uint32_t numIndices = static_cast<uint32_t>(qdata.indices.size());
//        
//        vkrs.geometryDataCreate(_instData.geomDataID, numVertices, numIndices, attribInfo);
//        uint32_t posSizeInBytes = numVertices * sizeof(qdata.positions[0]);
//        vkrs.geometryDataUpdateVertices(_instData.geomDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)qdata.positions.data());
//
//        uint32_t normSizeInBytes = numVertices * sizeof(qdata.normals[0]);
//        vkrs.geometryDataUpdateVertices(_instData.geomDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)qdata.normals.data());
//
//        uint32_t colorSizeInBytes = numVertices * sizeof(qdata.colors[0]);
//        vkrs.geometryDataUpdateVertices(_instData.geomDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)qdata.colors.data());
//
//        uint32_t texcoordSizeInBytes = numVertices * sizeof(qdata.texcoords[0]);
//        vkrs.geometryDataUpdateVertices(_instData.geomDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)qdata.texcoords.data());
//
//        uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
//        vkrs.geometryDataUpdateIndices(_instData.geomDataID, 0, indicesSizeInBytes, (void*)qdata.indices.data());
//        vkrs.geometryDataFinalize(_instData.geomDataID);
//
//        RSgeometryInfo geomInfo;
//        geomInfo.primType = RSprimitiveType::ptTriangle;
//        vkrs.geometryCreate(_instData.geometryID, geomInfo);
//        
//        _bbox = qdata.bbox;
//    }
//
//    bool Gizmo2dDrawable::initGeometry() 
//    {
//        QuadricData qd = QuadricDataFactory::createQuad();
//        createGeometry(qd);
//        std::string texturePathStub = AssetFolder::getTextureFolder();
//        std::string moveIcon = texturePathStub + "gizmo_button_ move_left.png";
//        std::string rotateCWIcon = texturePathStub + "gizmo_button_rotate_cw.png";
//        std::string rotateCCWIcon = texturePathStub + "gizmo_button_rotate_ccw.png";
//        
//        auto& vkrs = VkRenderSystem::getInstance();
//        RStextureID moveTextureID, rotateCWtextureID, rotateCCWtextureID;
//        vkrs.textureCreate(moveTextureID, moveIcon.c_str());
//        vkrs.textureCreate(rotateCWtextureID, rotateCWIcon.c_str());
//        vkrs.textureCreate(rotateCCWtextureID, rotateCCWIcon.c_str());
//        
//        _textureMap[IconType::itMove] = moveTextureID;
//        _textureMap[IconType::itRotateCW] = rotateCWtextureID;
//        _textureMap[IconType::itRotateCCW] = rotateCCWtextureID;
//        
//        return true;
//    }
//
//    bool Gizmo2dDrawable::initView() 
//    {
//        //Create the collection and instances
//        auto& vkrs = VkRenderSystem::getInstance();
//        RScollectionInfo collInfo;
//        collInfo.maxInstances = 4;
//        vkrs.collectionCreate(_collectionID, collInfo);
//        
//        RSappearanceInfo appInfo;
//        appInfo.diffuseTexture = _textureMap[IconType::itMove];
//        appInfo.shaderTemplate = RSshaderTemplate::stSimpleTextured;
//        vkrs.appearanceCreate(_instData.appearanceID, appInfo);
//        
//        RSspatial spatial;
//        vkrs.spatialCreate(_instData.spatialID, spatial);
//        
//        RSinstanceInfo instInfo;
//        instInfo.appID = _instData.appearanceID;
//        instInfo.spatialID = _instData.spatialID;
//        instInfo.gdataID = _instData.geomDataID;
//        instInfo.geomID = _instData.geometryID;
//        
//        vkrs.collectionInstanceCreate(_collectionID, _instData.instanceID, instInfo);
//        vkrs.collectionFinalize(_collectionID);
//        
//
//        return true;
//    }
//
//    bool Gizmo2dDrawable::init()
//    {
//        initGeometry();
//        initView();
//        
//        return true;
//    }
//
//    bool Gizmo2dDrawable::dispose()
//    {
//        disposeGeometry();
//        disposeView();
//        
//        return true;
//    }
//
//    std::vector<RScollectionID> Gizmo2dDrawable::getCollections() const
//    {
//        return std::vector<RScollectionID> {_collectionID};
//    }
//
//    bool Gizmo2dDrawable::disposeView()
//    {
//        auto& vkrs = VkRenderSystem::getInstance();
//        if(_instData.instanceID.isValid()) 
//        {
//            vkrs.collectionInstanceDispose(_collectionID, _instData.instanceID);
//        }
//        
//        vkrs.collectionDispose(_collectionID);
//        
//        return true;
//    }
//
//    bool Gizmo2dDrawable::disposeGeometry() 
//    {
//        auto& vkrs = VkRenderSystem::getInstance();
//        if(_instData.geometryID.isValid()) 
//        {
//            vkrs.geometryDispose(_instData.geometryID);
//        }
//        if(_instData.geomDataID.isValid()) 
//        {
//            vkrs.geometryDataDispose(_instData.geomDataID);
//        }
//        return true;
//    }
//
//    BoundingBox Gizmo2dDrawable::getBounds() 
//    {
//        return _bbox;
//    }
//
//    std::string Gizmo2dDrawable::getName() const 
//    {
//        return getDrawableName(DrawableType::dtGizmo2d);
//    }
//
//    DrawableType Gizmo2dDrawable::getType() const
//    {
//        return DrawableType::dtGizmo2d;
//    }
//}
//
