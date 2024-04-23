
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include "BoundingBox.h"
#include "rsids.h"
#include "RSdataTypes.h"

namespace ss 
{

/**
 * @brief Stores capabilities of a model file being read and loaded. This is a high level capabilities of the model that is cached around for making decisions later on while adapting the data to rendersystem constructs.
 */
struct ModelCapabilities 
{
    bool hasMesh = false;
    bool hasMaterials = false;
    bool hasTextures = false;
    bool hasAnimations = false;
    bool hasSkeleton = false;
    bool hasLights = false;
    bool hasCameras = false;
};

/**
 * @brief Describes the storage type and size of indices for vertex indices stored in model files. NB: Currently rendersystem only supports 32 bit integers for indices. This enum is only depicting what is stored in  a model file that is read and loaded but is then converted to 32 bit indices for rendersystem.
 */
enum IndicesIntType 
{
    iitUINT32,
    iitUINT16,
    iitUINT8,
    iintInvalid,
};

/**
 * @brief Stores the geometry data for a specific model. This is used when a model is being loaded from storage and for storing rendersystem handles to the vertex attributes.
 */
struct MeshData 
{
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> texcoords;
    std::vector<uint32_t> iindices;
    IndicesIntType indicesType = IndicesIntType::iitUINT32;
    
    RSgeometryDataID geometryDataID;
    RSgeometryID geometryID;
    RSvertexAttribsInfo attribsInfo;
    BoundingBox localBox;
    
    void* getAttribData(const RSvertexAttribute attrib);
    void dispose();
};


/**
 * @brief Stores the appearance data for a specific model that is being read and loaded.
 */
struct Appearance 
{
    glm::vec4 ambient = glm::vec4(1, 1, 1, 1);
    glm::vec4 diffuse = glm::vec4(0, 0, 0, 1);
    glm::vec4 specular = glm::vec4(0, 0, 0, 1);
    glm::vec4 emissive = glm::vec4(0, 0, 0, 1);
    float shininess = 0.0f;
    std::string diffuseTexturePath;
    bool isDiffuseTextureEmbedded = false;
};

/**
 * @brief Stores the visual representation of the data houses the rendersystem constructs for rendering purposes.
 */
struct MeshInstance 
{
    uint32_t meshIdx;
    glm::mat4 modelmat;
    
    RSinstanceID instanceID;
    RSspatialID spatialID;
    RSstateID stateID;
    RScollectionID associatedCollectionID; //do not dispose this
    RSappearanceID appearanceID;
    RStextureID diffuseTextureID;
    uint32_t materialIdx;
    
    void dispose();
};

/**
 * @brief Describes the type of texture being loaded from a model file from storage device.
 */
enum TextureType 
{
    ttDiffuse,
    ttAmbient,
    ttSpecular
};

/**
 * @brief Stores the type and path of the texture being loaded from a storage device.
 */
struct Texture 
{
    std::string texturePath;
    TextureType textureType;
};

/**
 * @brief Stores all of the data that is needed to describe all the visual representations of entities needed to be rendered within a world. 
 */
struct ModelData 
{
    std::string modelName;
    BoundingBox bbox;
    ss::ModelCapabilities imdcaps;
    std::vector<MeshInstance> meshInstances;
    std::unordered_map<uint32_t, Appearance> materials;
    
    RScollectionID collectionID;
};

//a mapping of a integer key(usually an index into a list of meshes) to a vector of scenesystem meshes
using MeshDataMap = std::unordered_map<uint32_t, std::vector<MeshData>>;
}
