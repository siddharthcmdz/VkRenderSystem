//
//#pragma once
//#include "ModelData.h"
//
//namespace tinygltf 
//{
//    class Model;
//    class Node;
//}
//
//namespace ss 
//{
//
//    class GLTFmodelLoader final 
//    {
//    private:
//        static std::unordered_map<std::string, tinygltf::Model*> _modelMap;
//        static const std::string IN_MEMORY_MODEL_STR;
//        
//        static std::string printMode(int mode);
//        static std::string indt(const int indent);
//        
//        static std::vector<glm::vec4> transform3to4(const float* data, size_t numVertices, bool isNormals = false);
//        static RSprimitiveType getPrimitiveMode(int mode);
//        static MeshDataMap populateMeshes(const tinygltf::Model* gltfModel);
//        static void populate(const tinygltf::Node* node, const tinygltf::Model* input, ss::MeshDataMap& meshDataMap,ss::ModelData& modelData);
//
//    public:
//        static MeshDataMap loadModelGeometry(std::string modelpath);
//        static void loadModelInstance(std::string modelpath, MeshDataMap& mdm, ModelData& modelData);
//        static MeshDataMap loadModelGeometryFromMemory(const unsigned char* memory, uint32_t memlen, uint32_t uniqueID);
//        static void loadModelInstance(uint32_t uniqueID, MeshDataMap& mdm, ModelData& modelData);
//        
//    };
//
//}
