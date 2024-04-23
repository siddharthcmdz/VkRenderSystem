
#include "GLTFmodelLoader.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "tiny_gltf.h"
#include "../tinygltf-2.8.21/tiny_gltf.h"
#include <iostream>
#include "ModelData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "VkRenderSystem.h"

namespace ss 
{

    std::unordered_map<std::string, tinygltf::Model*> GLTFmodelLoader::_modelMap;
    const std::string GLTFmodelLoader::IN_MEMORY_MODEL_STR = "_mem_model_id_";

    static std::string indt(const int indent)
    {
        std::string s;
        for (int i = 0; i < indent; i++) 
        {
            s += "  ";
        }
        return s;
    }

    static std::string printMode(int mode) 
    {
        if (mode == TINYGLTF_MODE_POINTS) 
        {
            return "POINTS";
        }
        else if (mode == TINYGLTF_MODE_LINE) 
        {
            return "LINE";
        }
        else if (mode == TINYGLTF_MODE_LINE_LOOP) 
        {
            return "LINE_LOOP";
        }
        else if (mode == TINYGLTF_MODE_TRIANGLES) 
        {
            return "TRIANGLES";
        }
        else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) 
        {
            return "TRIANGLE_FAN";
        }
        else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) 
        {
            return "TRIANGLE_STRIP";
        }
        return "**UNKNOWN**";
    }

    static void DumpStringIntMap(const std::map<std::string, int>& m, int idnt) 
    {
        std::map<std::string, int>::const_iterator it(m.begin());
        std::map<std::string, int>::const_iterator itEnd(m.end());
        for (; it != itEnd; it++) {
            std::cout << indt(idnt) << it->first << ": " << it->second << std::endl;
        }
    }

    static void dumpPrimitive(const tinygltf::Primitive& primitive, int idnt) 
    {
        std::cout << indt(idnt) << "material : " << primitive.material << std::endl;
        std::cout << indt(idnt) << "indices : " << primitive.indices << std::endl;
        std::cout << indt(idnt) << "mode     : " << printMode(primitive.mode) << "(" << primitive.mode << ")" << std::endl;
        std::cout << indt(idnt) << "attributes(items=" << primitive.attributes.size() << ")" << std::endl;
        DumpStringIntMap(primitive.attributes, idnt + 1);
    }

    static void dump(const tinygltf::Model& model) 
    {
        std::cout << "=== Dump glTF ===" << std::endl;
        std::cout << "asset.copyright          : " << model.asset.copyright << std::endl;
        std::cout << "asset.generator          : " << model.asset.generator << std::endl;
        std::cout << "asset.version            : " << model.asset.version << std::endl;
        std::cout << "asset.minVersion         : " << model.asset.minVersion << std::endl;
        std::cout << std::endl;

        std::cout << "=== Dump scene ===" << std::endl;
        std::cout << "defaultScene: " << model.defaultScene << std::endl;

        {
            std::cout << "meshes(item=" << model.meshes.size() << ")" << std::endl;
            for (size_t i = 0; i < model.meshes.size(); i++) {
                std::cout << indt(1) << "name     : " << model.meshes[i].name
                    << std::endl;
                std::cout << indt(1)
                    << "primitives(items=" << model.meshes[i].primitives.size()
                    << "): " << std::endl;

                for (size_t k = 0; k < model.meshes[i].primitives.size(); k++) 
                {
                    dumpPrimitive(model.meshes[i].primitives[k], 2);
                }
            }
        }
    }

    std::vector<glm::vec4> GLTFmodelLoader::transform3to4(const float* data, size_t numVertices, bool isNormals) 
    {
        std::vector<glm::vec4> attribs;
        attribs.resize(numVertices);
        size_t stride = 3;
        for (size_t i = 0; i < numVertices; i++) 
        {
            size_t idx0 = i * stride + 0;
            size_t idx1 = i * stride + 1;
            size_t idx2 = i * stride + 2;

            float x = data[idx0];
            float y = data[idx1];
            float z = data[idx2];
            glm::vec3 tup3(x, y, z);
            float w = 1.0f;
            if(isNormals) 
            {
                tup3 = glm::normalize(tup3);
                w = 0.0f;
            }
            attribs[i] = glm::vec4(tup3, w);
        }

        return attribs;
    }

    RSprimitiveType GLTFmodelLoader::getPrimitiveMode(int mode) 
    {
        if (mode == TINYGLTF_MODE_POINTS) 
        {
            return RSprimitiveType::ptPoint;
        }
        else if (mode == TINYGLTF_MODE_LINE) 
        {
            return RSprimitiveType::ptLine;
        }
        else if (mode == TINYGLTF_MODE_LINE_LOOP) 
        {
            return RSprimitiveType::ptLineLoop;
        }
        else if (mode == TINYGLTF_MODE_TRIANGLES) 
        {
            return RSprimitiveType::ptTriangle;
        }
        else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) 
        {
            return RSprimitiveType::ptTriangleFan;
        }
        else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) 
        {
            return RSprimitiveType::ptTriangleStrip;
        }
        return RSprimitiveType::ptPoint;
    }

    BoundingBox getLocalBox(const std::vector<glm::vec4>& positions) 
    {
        BoundingBox bbox;
        uint32_t numVerts = static_cast<uint32_t>(positions.size());
        for(uint32_t i =0 ; i < numVerts; i++)
        {
            bbox.expandBy(positions[i]);
        }
        return bbox;
    }

    MeshDataMap GLTFmodelLoader::populateMeshes(const tinygltf::Model* gltfModel) 
    {
        MeshDataMap meshDataMap;
        if(gltfModel == nullptr) 
        {
            return meshDataMap;
        }
        
        uint32_t numMeshes = static_cast<uint32_t>(gltfModel->meshes.size());
        for(uint32_t i = 0; i < numMeshes; i++) 
        {
            const tinygltf::Mesh mesh = gltfModel->meshes[i];
            std::cout<<"Mesh name: " <<mesh.name<<", num prims: "<<mesh.primitives.size()<<std::endl;
            // Iterate through all primitives of this node's mesh
            for (size_t j = 0; j < mesh.primitives.size(); j++)
            {
                ss::MeshData meshData;

                const tinygltf::Primitive& gltfPrimitive = mesh.primitives[j];
                //Vertices
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* colorsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                size_t vertexCount = 0;
                // Get buffer data for vertex positions
                if (gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end()) 
                {
                    const tinygltf::Accessor& accessor = gltfModel->accessors[gltfPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = gltfModel->bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(gltfModel->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    
                    vertexCount = accessor.count;
                    if (view.byteStride != 16) 
                    {
                        meshData.positions = transform3to4(positionBuffer, vertexCount);
                    }
                    meshData.localBox = getLocalBox(meshData.positions);
                }
                // Get buffer data for vertex normals
                if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end()) 
                {
                    const tinygltf::Accessor& accessor = gltfModel->accessors[gltfPrimitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = gltfModel->bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(gltfModel->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    
                    if (view.byteStride != 16) 
                    {
                        meshData.normals = transform3to4(normalsBuffer, vertexCount, true);
                    }
                }
                //Get buffer data for color. Color may be optionally found in the glb file but if its not there we need to manufacture it.
                if (gltfPrimitive.attributes.find("COLOR_0") != gltfPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = gltfModel->accessors[gltfPrimitive.attributes.find("COLOR_0")->second];
                    const tinygltf::BufferView& view = gltfModel->bufferViews[accessor.bufferView];
                    colorsBuffer = reinterpret_cast<const float*>(&(gltfModel->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    
                    if (view.byteStride != 16) 
                    {
                        meshData.colors = transform3to4(colorsBuffer, vertexCount);
                    }
                }
                
                // Get buffer data for vertex texture coordinates. glTF supports multiple sets, we only load the first one.
                if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = gltfModel->accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = gltfModel->bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(gltfModel->buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    
                    meshData.texcoords.resize(vertexCount);
                    for (size_t k = 0; k < vertexCount; i++) 
                    {
                        meshData.texcoords[k].x = texCoordsBuffer[k * 2 + 0];
                        meshData.texcoords[k].y = texCoordsBuffer[k * 2 + 1];
                    }
                }
                
                std::vector<RSvertexAttribute> attribs = {RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord};
                meshData.attribsInfo.attributes = attribs.data();
                meshData.attribsInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
                meshData.attribsInfo.settings = RSvertexAttributeSettings::vasSeparate;
                
                //Indices
                uint32_t indexCount = 0;
                const tinygltf::Accessor& accessor = gltfModel->accessors[gltfPrimitive.indices];
                const tinygltf::BufferView& bufferView = gltfModel->bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = gltfModel->buffers[bufferView.buffer];
                
                indexCount += static_cast<uint32_t>(accessor.count);
                switch (accessor.componentType) 
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: 
                    {
                        const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) 
                        {
                            meshData.iindices.push_back(buf[index]);
                        }
                        meshData.indicesType = ss::IndicesIntType::iitUINT32;
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: 
                    {
                        const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) 
                        {
                            meshData.iindices.push_back(buf[index]);
                        }
                        meshData.indicesType = ss::IndicesIntType::iitUINT16;
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: 
                    {
                        const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) 
                        {
                            meshData.iindices.push_back(buf[index]);
                        }
                        meshData.indicesType = ss::IndicesIntType::iitUINT8;
                        break;
                    }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        throw std::runtime_error("Unsupported index type while loading gltf file!");
                }
                
                auto& vkrs = VkRenderSystem::getInstance();
                vkrs.geometryDataCreate(meshData.geometryDataID, uint32_t(vertexCount), static_cast<uint32_t>(meshData.iindices.size()), meshData.attribsInfo);
                uint32_t posSizeInBytes = uint32_t(vertexCount) * sizeof(meshData.positions[0]);
                vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)meshData.positions.data());
                if (normalsBuffer == nullptr) 
                {
                    meshData.normals = std::vector<glm::vec4>(vertexCount, glm::vec4(0, 0, 0, 0));
                }
                uint32_t normSizeInBytes = uint32_t(vertexCount) * sizeof(meshData.normals[0]);
                vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)meshData.normals.data());
                
                if (colorsBuffer == nullptr) 
                {
                    meshData.colors = std::vector<glm::vec4>(vertexCount, glm::vec4(0.8, 0.0f, 0.8, 1.0f));
                }
                uint32_t colorSizeInBytes = uint32_t(vertexCount) * sizeof(meshData.colors[0]);
                vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)meshData.colors.data());
                
                if (texCoordsBuffer == nullptr) 
                {
                    meshData.texcoords = std::vector<glm::vec2>(vertexCount, glm::vec2(0, 0));
                }
                uint32_t texcoordSizeInBytes = uint32_t(vertexCount) * sizeof(meshData.texcoords[0]);
                vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)meshData.texcoords.data());
                
                
                if (meshData.iindices.size()) 
                {
                    uint32_t indicesSizeInBytes = indexCount * sizeof(uint32_t);
                    vkrs.geometryDataUpdateIndices(meshData.geometryDataID, 0, indicesSizeInBytes, (void*)meshData.iindices.data());
                }
                vkrs.geometryDataFinalize(meshData.geometryDataID);
                
                RSgeometryInfo geomInfo;
                geomInfo.primType = getPrimitiveMode(gltfPrimitive.mode);
                vkrs.geometryCreate(meshData.geometryID, geomInfo);
                
                meshDataMap[i].push_back(meshData);
            }
        }
        
        return meshDataMap;
    }

    void GLTFmodelLoader::populate(const tinygltf::Node* node, const tinygltf::Model* input, ss::MeshDataMap& meshDataMap,ss::ModelData& modelData) {
        if(node == nullptr || input == nullptr) 
        {
            return;
        }
        glm::mat4 localToWorldMat(1.0f);
        //The below piece of code to calulate local to world matrices is taken from tinygltf examples.
        if (node->translation.size() == 3)
        {
            localToWorldMat = glm::translate(localToWorldMat, glm::vec3(glm::make_vec3(node->translation.data())));
        }
        if (node->rotation.size() == 3) 
        {
            glm::quat q = glm::make_quat(node->rotation.data());
            localToWorldMat *= glm::mat4(q);
        }
        if (node->scale.size() == 3) 
        {
            localToWorldMat = glm::scale(localToWorldMat, glm::vec3(glm::make_vec3(node->scale.data())));
        }
        if (node->matrix.size() == 16) 
        {
            localToWorldMat = glm::make_mat4x4(node->matrix.data());
        };
        
        //Load node's children
        for (size_t i = 0; i < node->children.size(); i++) 
        {
            const tinygltf::Node& child = input->nodes[node->children[i]];
            populate(&input->nodes[node->children[i]], input, meshDataMap, modelData);
        }

        auto& vkrs = VkRenderSystem::getInstance();
        //If node contains meshdata , load vertices and indices from buffers.
        //In gltf this is done via accessors and buffer views
        if (node->mesh > -1) 
        {
            const tinygltf::Mesh mesh = input->meshes[node->mesh];

            uint32_t numPrimitives = static_cast<uint32_t>(meshDataMap[node->mesh].size());
            assert(numPrimitives == mesh.primitives.size() && "mismatch number of primitives in mesh");
            
            RSspatialID spatialID;
            RSspatial spatial;
//            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
            spatial.model = localToWorldMat /** scaleMat*/;
            spatial.modelInv = glm::inverse(spatial.model);
            vkrs.spatialCreate(spatialID, spatial);

            RSappearanceID appearanceID;
            RSappearanceInfo appInfo;
            appInfo.shaderTemplate = RSshaderTemplate::stSimpleLit;
            vkrs.appearanceCreate(appearanceID, appInfo);

            for(uint32_t i = 0; i < numPrimitives; i++) 
            {
                tinygltf::Primitive prim = mesh.primitives[i];
                ss::MeshInstance meshInst;
                meshInst.spatialID = spatialID;
                meshInst.appearanceID = appearanceID;
                meshInst.materialIdx = prim.material;
                meshInst.modelmat = spatial.model;
                meshInst.stateID.id = INVALID_ID;
                meshInst.meshIdx = node->mesh;
                ss::MeshData& meshData = meshDataMap[node->mesh][i];

                RSinstanceInfo instInfo;
                instInfo.gdataID = meshData.geometryDataID;
                instInfo.geomID = meshData.geometryID;
                instInfo.spatialID = meshInst.spatialID;
                instInfo.appID = meshInst.appearanceID;
                instInfo.name = modelData.modelName;

                vkrs.collectionInstanceCreate(modelData.collectionID, meshInst.instanceID, instInfo);
                glm::vec4 bboxWCmin = localToWorldMat * meshData.localBox.getmin();
                glm::vec4 bboxWCmax = localToWorldMat * meshData.localBox.getmax();
                modelData.bbox.expandBy(bboxWCmin);
                modelData.bbox.expandBy(bboxWCmax);
                
                modelData.meshInstances.push_back(meshInst);
            }
        }
    }

    MeshDataMap GLTFmodelLoader::loadModelGeometryFromMemory(const unsigned char* modelInMemory, uint32_t memlen, uint32_t uniqueID)
    {
        std::string modelpath = IN_MEMORY_MODEL_STR+std::to_string(uniqueID);
        tinygltf::Model model;
        tinygltf::TinyGLTF gltfctx;
        std::string err;
        std::string warn;
        
        bool ret = gltfctx.LoadBinaryFromMemory(&model, &err, &warn, modelInMemory, memlen);
        
        if (!warn.empty()) 
        {
            printf("Warn: %s\n", warn.c_str());
        }

        if (!err.empty()) 
        {
            printf("Err: %s\n", err.c_str());
        }

        if (!ret) 
        {
            printf("Failed to parse glTF\n");
        }

        _modelMap[modelpath] = new tinygltf::Model(model);
        MeshDataMap mdm = populateMeshes(&model);
        return mdm;

    }

    MeshDataMap GLTFmodelLoader::loadModelGeometry(std::string modelpath) 
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF gltfctx;
        std::string err;
        std::string warn;
        
        bool ret = gltfctx.LoadBinaryFromFile(&model, &err, &warn, modelpath.c_str());
        
        if (!warn.empty()) 
        {
            printf("Warn: %s\n", warn.c_str());
        }

        if (!err.empty()) 
        {
            printf("Err: %s\n", err.c_str());
        }

        if (!ret) 
        {
            printf("Failed to parse glTF\n");
        }
        
        _modelMap[modelpath] = new tinygltf::Model(model);
        MeshDataMap mdm = populateMeshes(&model);
        
        return mdm;
    }

    void GLTFmodelLoader::loadModelInstance(uint32_t uniqueID, MeshDataMap& mdm, ModelData& modelData)
    {
        std::string modelname = IN_MEMORY_MODEL_STR + std::to_string(uniqueID);
        loadModelInstance(modelname, mdm, modelData);
    }

    void GLTFmodelLoader::loadModelInstance(std::string modelpath, MeshDataMap& meshDataMap, ModelData& modelData) 
    {
        if(_modelMap.find(modelpath) != _modelMap.end()) 
        {
            const tinygltf::Model* model = _modelMap[modelpath];
            const tinygltf::Scene& scene = model->scenes[0];
            for (size_t i = 0; i < scene.nodes.size(); i++) 
            {
                const tinygltf::Node& node = model->nodes[scene.nodes[i]];
                populate(&node, model, meshDataMap, modelData);
            }
        }
    }
}
