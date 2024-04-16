
#pragma once
#include "rsids.h"
#include "RSdataTypes.h"
#include "TextureLoader.h"
#include "BoundingBox.h"
#include "ssids.h"

namespace ss
{

    /**
     * @brief Stores visual representation of entities to be rendered.
     */
    struct InstanceData 
    {
        RSinstanceID instanceID;
        RSspatialID spatialID;
        RSappearanceID appearanceID;
        RSstateID stateID;
        RStextureID diffuseTextureID;
        RSgeometryID geometryID;
        RSgeometryDataID geomDataID;
    };
    
    /**
     * @brief Stores temporarily pointer to the 3D model in memory read over network.
     */
    struct InMemoryModel
    {
        const unsigned char* memModel = nullptr;
        uint32_t memcnt = 0;
    };

    /**
     * @brief Stores all related data for loading a 3D asset model which is usually a tracked surgical instrument.
     */
    struct AssetModelData
    {
        BoundingBox bboxwc;
        AssetModelID modelID;
    };

    /**
     * @brief Stores information related to the 3D vollume
     */
    struct VolumeModel
    {
        RStextureID textureID;
        RStextureInfo info;
        glm::vec3 scanScale{};
        glm::mat4 toLPS{};
    };

    /**
     * @brief Stores view related rs ids like context and its associated view ID.
     */
    struct SceneData
    {
        RScontextID ctxID;
        RSviewID viewID;
    };

    /**
     * @brief Stores large reource entities like geometry data that comprises of vertex attributes.
     */
    struct GeometryInfo {
        RSgeometryDataID geomDataID;
        RSgeometryID geomID;
        ss::BoundingBox localBox;
    };

    /**
     * @brief Stores information related to the 3D volume slice that intersects with the volume data.
     */
    struct VolumeSliceInfo
    {
        VolumeModel volumeModel;
        float width = 0.0f, height = 0.0;
        glm::vec3 sliceNormal = glm::vec3(0.0f, 0.0f, 1.0f); //default is axial.
        RSspatial spatial;
        RSvolumeSliceAppearance appearance;
        std::string name;
    };
    
    /**
     * @brief Stores information related to volume slice info and the volume slice ID.
     */
    struct VolumeSliceData
    {
        VolumeSliceID sliceID;
        VolumeSliceInfo sliceInfo;
        glm::mat4 sliceRotMat = glm::mat4(1.0f);
        glm::mat4 sliceToOriginMat = glm::mat4(1.0f);
        glm::mat4 sliceOriginToPrevMat = glm::mat4(1.0f);
        glm::mat4 sliceTransMat = glm::mat4(1.0f);
        glm::vec3 voxelTrans;
    };

    /**
    * @brief Stores information about the floor grid.
    */
    struct GridInfo
    {
        float size = 10.0f;
        float triadScale = 1.0f;
        uint32_t resolution = 10;
        glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    };

    /**
    * @brief Stores information about the XYZ triad.
    */
    struct TriadInfo
    {
        float size = 1.0f;
        glm::vec3 axis1 = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 axis2 = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 axis3 = glm::vec3(0.0f, 0.0f, 1.0f);
        RSspatial spatial;
    };

    /**
     * @brief Stores information about the bounding box to render.
     */
    struct BoundsInfo
    {
        BoundingBox bbox;
        glm::mat4 xform = glm::mat4(1.0f);
    };

    /**
     * @brief Stores the offsets of individual anatomical planes
     */
    struct VolumeOffset
    {
        uint32_t axial = 0;
        uint32_t sagittal = 0;
        uint32_t coronal = 0;
    };
}
