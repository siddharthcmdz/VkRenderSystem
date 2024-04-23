
#pragma once

#include "rsids.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <array>
#include "TextureLoader.h"
#include "RSdataTypes.h"
#include "SSdataTypes.h"
#include "ssids.h"
#include "ModelData.h"

namespace ss
{
    /**
     * @brief calculates two vectors that is normal to the given normal vector that defines the plane of hte quad
     * @param normal the specified normal vector
     * @return two vectors that are orthogonal to each other and to the normal.
     */
    static std::array<glm::vec3, 2> getOrthoNormals(glm::vec3 normal);

    /**
     * @brief calculates 4 points of a quad whose dimensions are equal to input size and given normal to the plane.
     * @param size the specified size of the quad in width and height
     * @param normal the specified normal of the quad
     * @param isVolumeSlice if volume slice, then the range goes from 0,0,0 to ni, nj, nk, else each dimension ranges from -size/2 to size/2.
     * @return the four points of the quad.
     */
    static std::array<glm::vec4, 4> getQuadCorners(float size, glm::vec3 normal, bool isVolumeSlice);

    /**
     * @brief A collection of rendeable entities behind a facade to manage its lifecycle
     */
    class RenderableUtils final
    {
    private:
        struct RSlineIDs
        {
            RSgeometryDataID geomDataID;
            RSgeometryID geomID;
            RSappearanceID appID;
            RSinstanceID instanceID;
            RSstateID stateID;
            RSspatialID spatialID;
        };

        struct RenderableIDs
        {
            RSgeometryDataID geomDataID;
            RSgeometryID geomID;
            RStextureID voxelTextureID;
            RStextureID lutTextureID;
            RSappearanceID appID;
            RSstateID stateID;
            RSspatialID spatialID;
            RSinstanceID instanceID;
        };

        struct RUvolumeSlice
        {
            VolumeSliceInfo vsinfo;
            std::array<glm::vec4, 4> sliceCorners;

            RenderableIDs sliceIDs;
        };
        
        struct RUassetModel
        {
            BoundingBox bbox;
            ModelData modelData;
            MeshDataMap meshDataMap;
            std::string modelPath;
        };
        
        struct RUgrid
        {
            GridInfo gi;
            RSlineIDs lineIDs;
        };

        struct RUtriad
        {
            TriadInfo tinfo;
            RSlineIDs lineIDs;
        };
        
        struct RUbounds
        {
            BoundsInfo boundsInfo;
            RSlineIDs lineIDs;
        };
        
        
        static std::unordered_map<VolumeSliceID, RUvolumeSlice, IDHasher<VolumeSliceID>> _volSliceMap;
        static std::unordered_map<GridID, RUgrid, IDHasher<GridID>> _gridMap;
        static std::unordered_map<TriadID, RUtriad, IDHasher<TriadID>> _triadMap;
        static std::unordered_map<BoundsID, RUbounds, IDHasher<BoundsID>> _boundsMap;
        static std::unordered_map<AssetModelID, RUassetModel, IDHasher<AssetModelID>> _assetModelMap;

        static MakeID _volumeSliceIDpool;
        static MakeID _gridIDpool;
        static MakeID _triadIDpool;
        static MakeID _boundsIDpool;
        static MakeID _assetModelIDpool;
        
        static void dispose(RSlineIDs& lineIDs, RScollectionID& collectionID);

        public:
        
            /**
             * @brief Creates a volume slice.
             * @param volsliceInfo the specified information needed for contructing a volume slice
             * @param collectionID the specified rs collection to which one or more volume slices can be added to.
             * @return the ID corresponding to a volume slice
             */
            static VolumeSliceID volumeSliceCreate(const VolumeSliceInfo& volsliceInfo, const RScollectionID& collectionID/*, const RScollectionID& trackerCollectionID*/);
        
            /**
             * @brief Updates the transformation matrix for a volume slice.
             * @param volumeSliceID the specified volume slice ID
             * @param spatial the specified model and texture matrices/spatials for the volume slice
             */
            static void volumeSliceUpdate(const VolumeSliceID& volumeSliceID, const RSspatial& spatial);
        
            /**
             * @brief Updates the volume slice appearance info on the volume slice
             * @param volumeSliceID the specified volume slice ID
             * @param appinfo the specified appearance information pertinent to a volume slice
             */
            static void volumeSliceUpdate(const VolumeSliceID& volumeSliceID, const RSvolumeSliceAppearance& appinfo);
        
            /**
             * @brief Gets the bounding box for the volume slice
             * @param volumeSliceID the specified volume slice ID
             * @return the bounding box of the slice
             */
            static BoundingBox volumeSliceGetBounds(const VolumeSliceID& volumeSliceID);
            
            /**
             *
             */
            static std::array<glm::vec3, 3> volumeSliceGetFrame(const VolumeSliceID& volumeSliceID);
        
            /**
             * @brief Gets the render system instanceID for the specified volume slice
             * @param volumeSliceID the specified volume slice ID
             * @return the specified instance ID
             */
            static RSinstanceID volumeSliceGetInstance(const VolumeSliceID& volumeSliceID);
        
            /**
             * @brief Disposes a volume slice and frees all host and device resources
             * @param volumeSliceID the specified volume slice ID
             * @param collectionID the specified collectionID that contains the volume slice
             */
            static void volumeSliceDispose(const VolumeSliceID& volumeSliceID, const RScollectionID& collectionID);
        
            /**
             * @brief Creates a grid made of lines to be used to self orient the scene
             * @param gi the specified grid information that contains data like resolution and size
             * @param collectionID the specified collection to which grid is created.
             * @return the grid ID
             */
            static GridID GridCreate(const GridInfo& gi, const RScollectionID& collectionID);
        
            /**
             * @brief Disposes the grid and all of its resources on host and device
             * @param gridID the specified grid ID to dispose backing resources
             * @param collectionID the specified collection to which remove the grid from.
             */
            static void GridDispose(const GridID& gridID, RScollectionID& collectionID);

            /**
             * @brief Creates a triad that basically renders world orthogonal vectors
             * @param ti the specified triad info that contains data like size
             * @param collectionID the specified collection ID to which the triad is added to.
             */
            static TriadID triadCreate(const TriadInfo& ti, const RScollectionID collectionID);
        
            static void triadUpdate(const TriadID& triadID, const RSspatial& spatial);
        
            /**
             * @brief DIsposes the triad along with its backing resources in device and host.
             * @param triadID the specified triad ID
             * @param collectionID the specified collection ID  which contains the triad
             */
            static void triadDispose(const TriadID& triadID, RScollectionID& collectionID);
        
            /**
             * @brief Creates a bounding box used for visulization and debugging purposes.
             * @param bi the specified bounds info data
             * @param collectionID the specified collection ID
             * @return the bounds ID.
             */
            static BoundsID boundsCreate(const BoundsInfo& bi, const RScollectionID& collectionID);
        
            /**
             * @brief Disposes the bounds renderable
             * @param boundsID the specified unique bounds ID
             * @param collectionID the specified collection ID
             */
            static void boundsDispose(const BoundsID& boundsID, RScollectionID& collectionID);
            
            /**
             * @brief Creates and caches 3d glb models that are created in memory.
             * @param inMemoryModel the specified pointer to the memory location that has the encoded 3d gltf model
             * @param memcount the specifed number of bytes in the memory location
             * @param uniqueID the specified trackable ID that uniquely identifies this model
             * @param collectionID the specified rs collection ID to which this model will be added to
             * @return the unique ID that represent this model
             */
            static AssetModelID modelCreate(const unsigned char* inMemoryModel, uint32_t memcount, uint32_t uniqueID, const RScollectionID& collectionID, const std::string& dbgname="");
            
            /**
             *
             */
            static BoundingBox modelGetBounds(const AssetModelID& modelID);
        
            /**
             *
             */
            static std::vector<RSinstanceID> modelGetInstances(const AssetModelID& modelID);
        
            /**
             * @brief Updates the model's transformation matrix.
             * @param modelID the specified model ID to update its transformation
             * @param spatial the specified transformation matrices
             */
            static void modelUpdate(const AssetModelID& modelID, const RSspatial& spatial);
            
            /**
             * @brief Disposes the model in the cache
             * @param modelID the specified unique model ID
             * @param collectionID the specified collection from which the model should be removed.
             */
            static void modelDispose(const AssetModelID& modelID, const RScollectionID& collectionID);
        
    };

}
