
#include "BenchmarkDrawable.h"
#include "VkRenderSystem.h"
#include "GLTFmodelLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "AssetFolder.h"

namespace  ss 
{

    MeshData BenchmarkDrawable::createQuadric(const QuadricData& qdata) 
    {
        MeshData meshData;
        meshData.positions = qdata.positions;
        meshData.colors = qdata.colors;
        meshData.normals = qdata.normals;
        meshData.texcoords = qdata.texcoords;
        meshData.iindices = qdata.indices;
        
        auto& vkrs = VkRenderSystem::getInstance();
        
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
        RSvertexAttribsInfo attribInfo;
        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
        attribInfo.attributes = attribs.data();
        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
        
        uint32_t numVertices = static_cast<uint32_t>(qdata.positions.size());
        uint32_t numIndices = static_cast<uint32_t>(qdata.indices.size());

        vkrs.geometryDataCreate(meshData.geometryDataID, numVertices, numIndices, attribInfo);
        uint32_t posSizeInBytes = numVertices * sizeof(qdata.positions[0]);
        vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)qdata.positions.data());

        uint32_t normSizeInBytes = numVertices * sizeof(qdata.normals[0]);
        vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)qdata.normals.data());

        uint32_t colorSizeInBytes = numVertices * sizeof(qdata.colors[0]);
        vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)qdata.colors.data());

        uint32_t texcoordSizeInBytes = numVertices * sizeof(qdata.texcoords[0]);
        vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)qdata.texcoords.data());

        uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
        vkrs.geometryDataUpdateIndices(meshData.geometryDataID, 0, indicesSizeInBytes, (void*)qdata.indices.data());
        vkrs.geometryDataFinalize(meshData.geometryDataID);
        
        RSgeometryInfo geometry;
        geometry.primType = RSprimitiveType::ptTriangle;
        vkrs.geometryCreate(meshData.geometryID, geometry);

        return meshData;
    }

    void BenchmarkDrawable::createRenderable(BenchmarkRenderableType brt) 
    {
        MeshData meshData;
        if(brt == BenchmarkRenderableType::brtModel) 
        {
            std::string modelPathStub = AssetFolder::getModelFolder();
            _modelPath = modelPathStub + "/101-0008.glb";
            
            MeshDataMap meshDataMap = GLTFmodelLoader::loadModelGeometry(_modelPath);
            //we know the above model has only one primitive and mesh
            meshData = meshDataMap[0][0];
        }
        else
        {
            QuadricData qd;
            switch (brt) 
            {
                case BenchmarkRenderableType::brtSphereQuadric:
                    qd = QuadricDataFactory::createSphere();
                    break;
                    
                case BenchmarkRenderableType::brtCylinderQuadric:
                    qd = QuadricDataFactory::createCylinder();
                    break;
                    
                case BenchmarkRenderableType::brtConeQuadric:
                    qd = QuadricDataFactory::createCone();
                    break;
                    
                case BenchmarkRenderableType::brtDiskQuadric:
                    qd = QuadricDataFactory::createDisk();
                    break;
                    
                case BenchmarkRenderableType::brtQuad:
                    qd = QuadricDataFactory::createQuad();
                    break;
                    
                default:
                    break;
            }
            meshData = createQuadric(qd);
        }
        
        _meshDataArray[brt] = meshData;
    }

    bool BenchmarkDrawable::initGeometry() 
    {
        createRenderable(BenchmarkRenderableType::brtSphereQuadric);
        createRenderable(BenchmarkRenderableType::brtCylinderQuadric);
        createRenderable(BenchmarkRenderableType::brtConeQuadric);
        createRenderable(BenchmarkRenderableType::brtDiskQuadric);
        createRenderable(BenchmarkRenderableType::brtQuad);
        createRenderable(BenchmarkRenderableType::brtModel);
        
        
        return true;
    }

    bool BenchmarkDrawable::initView()
    {
        auto& vkrs = VkRenderSystem::getInstance();
        RScollectionInfo collInfo;
        collInfo.maxInstances = 1000;
        vkrs.collectionCreate(_modelData.collectionID, collInfo);
        
        RSappearanceID quadricAppID;
        RSappearanceInfo quadricAppInfo;
        quadricAppInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
        vkrs.appearanceCreate(quadricAppID, quadricAppInfo);
        
        RSappearanceID meshAppID;
        RSappearanceInfo meshAppInfo;
        meshAppInfo.shaderTemplate = RSshaderTemplate::stSimpleLit;
        vkrs.appearanceCreate(meshAppID, meshAppInfo);

        uint32_t idx = 0;
        for(uint32_t i = 0; i < _numMeshPerAxis; i++) 
        {
            for(uint32_t j = 0; j < _numMeshPerAxis; j++) 
            {
                for(uint32_t k = 0; k < _numMeshPerAxis; k++) 
                {
                    float x = -float(_numMeshPerAxis)*0.5f + float(k);
                    float y = -float(_numMeshPerAxis)*0.5f + float(j);
                    float z = -float(_numMeshPerAxis)*0.5f + float(i);
//                    std::cout<<"translate: "<<x<<", "<<y<<", "<<z<<std::endl;s
                    glm::vec3 trans(x, y, z);
                    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), trans);
                    
                    RSspatial spatial;
                    spatial.model = transMat;
                    spatial.modelInv = glm::inverse(spatial.model);
                    RSspatialID spatialID;
                    vkrs.spatialCreate(spatialID, spatial);
                    
                    MeshData meshData = _meshDataArray[idx];
                    idx = ++idx % 6;
                    
                    RSinstanceInfo instInfo;
                    instInfo.appID = idx % 6 == 0 ? meshAppID : quadricAppID;
                    instInfo.geomID = meshData.geometryID;
                    instInfo.gdataID = meshData.geometryDataID;
                    instInfo.spatialID = spatialID;
                    
                    InstanceData instData;
                    vkrs.collectionInstanceCreate(_modelData.collectionID, instData.instanceID, instInfo);
                    instData.appearanceID = instInfo.appID;
                    instData.spatialID = instInfo.spatialID;
                }
            }
        }
        
        vkrs.collectionFinalize(_modelData.collectionID);
        return true;
    }

    bool BenchmarkDrawable::disposeView()
    {
            auto& vkrs = VkRenderSystem::getInstance();
            vkrs.collectionDispose(_modelData.collectionID);
       
        for(MeshInstance& mi : _modelData.meshInstances) 
        {
            mi.dispose();
        }

        return true;
    }

    std::vector<RScollectionID> BenchmarkDrawable::getCollections() const
    {
        return std::vector<RScollectionID>{ _modelData.collectionID };
    }

    bool BenchmarkDrawable::init()
    {
        initGeometry();
        initView();
        
        return true;
    }

    bool BenchmarkDrawable::dispose()
    {
        disposeGeometry();
        disposeView();
        
        return true;
    }

    bool BenchmarkDrawable::disposeGeometry() 
    {
        for(MeshData& md : _meshDataArray) 
        {
            md.dispose();
        }
        return true;
    }

    BoundingBox BenchmarkDrawable::getBounds() 
    {
        float szf = static_cast<float>(_numMeshPerAxis);
        BoundingBox bbox(glm::vec4(-szf, -szf, -szf, 1.0f), glm::vec4(szf, szf, szf, 1.0f));
        return bbox;
    }

    std::string BenchmarkDrawable::getName() const 
    {
        return getDrawableName(DrawableType::dtBenchmark);
    }

    DrawableType BenchmarkDrawable::getType() const 
    {
        return DrawableType::dtBenchmark;
    }
}
