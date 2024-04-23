
#include "RenderableUtils.h"
#include "MathUtils.h"
#include <vector>
#include "VkRenderSystem.h"
#include "GLTFmodelLoader.h"
#include <cstdint>
#include <glm/gtx/string_cast.hpp>


namespace ss
{
    MakeID RenderableUtils::_volumeSliceIDpool = MakeID(UINT32_MAX);
    MakeID RenderableUtils::_gridIDpool = MakeID(UINT32_MAX);
    MakeID RenderableUtils::_triadIDpool = MakeID(UINT32_MAX);
    MakeID RenderableUtils::_boundsIDpool = MakeID(UINT32_MAX);
    MakeID RenderableUtils::_assetModelIDpool = MakeID(UINT32_MAX);

    std::unordered_map<VolumeSliceID, RenderableUtils::RUvolumeSlice, IDHasher<VolumeSliceID>> RenderableUtils::_volSliceMap;
    std::unordered_map<GridID, RenderableUtils::RUgrid, IDHasher<GridID>> RenderableUtils::_gridMap;
    std::unordered_map<TriadID, RenderableUtils::RUtriad, IDHasher<TriadID>> RenderableUtils::_triadMap;
    std::unordered_map<BoundsID, RenderableUtils::RUbounds, IDHasher<BoundsID>> RenderableUtils::_boundsMap;
    std::unordered_map<AssetModelID, RenderableUtils::RUassetModel, IDHasher<AssetModelID>> RenderableUtils::_assetModelMap;

    VolumeSliceID RenderableUtils::volumeSliceCreate(const VolumeSliceInfo& volsliceInfo, const RScollectionID& volumeSliceCollectionID/*, const RScollectionID& trackerCollectionID*/)
    {
        uint32_t id;
        VolumeSliceID outSliceID;
        RUvolumeSlice ruslice;
        bool success = _volumeSliceIDpool.CreateID(id);
        if(success)
        {
            outSliceID.id = id;
            ruslice.vsinfo = volsliceInfo;
        }
        
        ruslice.sliceCorners = MathUtils::getRectCorners(volsliceInfo.width, volsliceInfo.height, volsliceInfo.sliceNormal, false);
        
        glm::vec4 normal = glm::vec4(volsliceInfo.sliceNormal, 0.0f);
        std::array<glm::vec4, 4> normals;
        normals.fill(normal);
        
        std::array<glm::vec4, 4> colors;
        colors.fill(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        
        std::array<glm::vec2, 4> texcoords = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };
        
        std::array<uint32_t, 6> indices = {
            0, 1, 2,
            2, 3, 0
        };
        
        auto& vkrs = VkRenderSystem::getInstance();
        
        RenderableIDs& ids = ruslice.sliceIDs;
        
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
        RSvertexAttribsInfo attribInfo;
        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
        attribInfo.attributes = attribs.data();
        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
        
        uint32_t numVertices = static_cast<uint32_t>(ruslice.sliceCorners.size());
        uint32_t numIndices = static_cast<uint32_t>(indices.size());
        
        vkrs.geometryDataCreate(ids.geomDataID, numVertices, numIndices, attribInfo);
        uint32_t posSizeInBytes = numVertices * sizeof(ruslice.sliceCorners[0]);
        vkrs.geometryDataUpdateVertices(ids.geomDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)ruslice.sliceCorners.data());

        uint32_t normSizeInBytes = numVertices * sizeof(normals[0]);
        vkrs.geometryDataUpdateVertices(ids.geomDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)normals.data());

        uint32_t colorSizeInBytes = numVertices * sizeof(colors[0]);
        vkrs.geometryDataUpdateVertices(ids.geomDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)colors.data());

        uint32_t texcoordSizeInBytes = numVertices * sizeof(texcoords[0]);
        vkrs.geometryDataUpdateVertices(ids.geomDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)texcoords.data());

        uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
        vkrs.geometryDataUpdateIndices(ids.geomDataID, 0, indicesSizeInBytes, (void*)indices.data());
        vkrs.geometryDataFinalize(ids.geomDataID);

        RSgeometryInfo geomInfo;
        geomInfo.primType = RSprimitiveType::ptTriangle;
        vkrs.geometryCreate(ids.geomID, geomInfo);
        
        //Create the collection and instances
        assert(volsliceInfo.volumeModel.textureID.isValid() && "invalid volume texture ID");
        ids.voxelTextureID = volsliceInfo.volumeModel.textureID;
        
        RSappearanceInfo appInfo;
        appInfo.shaderTemplate = RSshaderTemplate::stVolumeSlice;
        appInfo.volumeSlice = volsliceInfo.appearance;
        //TODO: commented below just for testing purposes
//        appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
        appInfo.diffuseTexture = ids.voxelTextureID;
        vkrs.appearanceCreate(ids.appID, appInfo);
        
        RSspatial spatial = volsliceInfo.spatial;
        vkrs.spatialCreate(ids.spatialID, spatial);
        
        RSinstanceInfo instInfo;
        instInfo.gdataID = ids.geomDataID;
        instInfo.geomID = ids.geomID;
        instInfo.appID = ids.appID;
        instInfo.spatialID = ids.spatialID;
        instInfo.name = volsliceInfo.name;
        
        vkrs.collectionInstanceCreate(volumeSliceCollectionID, ids.instanceID, instInfo);
        
        if(success)
        {
            _volSliceMap[outSliceID] = ruslice;
        }
        
        return outSliceID;
    }

    std::array<glm::vec3, 3> RenderableUtils::volumeSliceGetFrame(const VolumeSliceID& volumeSliceID)
    {
        assert(volumeSliceID.isValid() && "invalid volume slice ID");
        assert(_volSliceMap.find(volumeSliceID) != _volSliceMap.end() && "volume slice map inconsistent state");

        std::array<glm::vec3, 3> res;
        if(volumeSliceID.isValid() && _volSliceMap.find(volumeSliceID) != _volSliceMap.end())
        {
            const RUvolumeSlice& ruvs = _volSliceMap.at(volumeSliceID);
//            std::array<glm::vec4, 4> corners = ruvs.sliceCorners;
            std::array<glm::vec3, 2> vecs = MathUtils::getOrthoNormals(ruvs.vsinfo.sliceNormal);
            glm::vec4 xformedXaxis = glm::vec4(vecs[0], 0.0f) * ruvs.vsinfo.spatial.model;
            glm::vec4 xformedYaxis = glm::vec4(vecs[1], 0.0f) * ruvs.vsinfo.spatial.model;
            glm::vec4 xformedZaxis = glm::vec4(ruvs.vsinfo.sliceNormal, 0.0f) * ruvs.vsinfo.spatial.model;
            
            res[0] = glm::normalize(glm::vec3(xformedXaxis));
            res[1] = glm::normalize(glm::vec3(xformedYaxis));
            res[2] = glm::normalize(glm::vec3(xformedZaxis));
        }
        
        return res;
    }

    RSinstanceID RenderableUtils::volumeSliceGetInstance(const VolumeSliceID& volumeSliceID)
    {
        assert(volumeSliceID.isValid() && "invalid volume slice ID");
        RSinstanceID instanceID;
        if(volumeSliceID.isValid() && _volSliceMap.find(volumeSliceID) != _volSliceMap.end())
        {
            RUvolumeSlice& ruslice = _volSliceMap[volumeSliceID];
            instanceID = ruslice.sliceIDs.instanceID;
        }
        
        return instanceID;
    }

    void RenderableUtils::volumeSliceUpdate(const VolumeSliceID& volumeSliceID, const RSspatial& spatial)
    {
        if(volumeSliceID.isValid())
        {
            RUvolumeSlice& ruslice = _volSliceMap[volumeSliceID];
            ruslice.vsinfo.spatial = spatial;
            
            auto& vkrs = VkRenderSystem::getInstance();
            vkrs.spatialSetData(ruslice.sliceIDs.spatialID, spatial);
        }
    }

    void RenderableUtils::volumeSliceUpdate(const VolumeSliceID& volumeSliceID, const RSvolumeSliceAppearance& vsapp)
    {
        if(volumeSliceID.isValid())
        {
            RUvolumeSlice& ruslice = _volSliceMap[volumeSliceID];
            ruslice.vsinfo.appearance = vsapp;

            auto& vkrs = VkRenderSystem::getInstance();
            vkrs.appearanceUpdateVolumeSlice(ruslice.sliceIDs.appID, vsapp);
        }
    }

    BoundingBox RenderableUtils::volumeSliceGetBounds(const VolumeSliceID& volumeSliceID)
    {
        BoundingBox bbox;
        if(volumeSliceID.isValid())
        {
            RUvolumeSlice& ruslice = _volSliceMap[volumeSliceID];
            const int NUM_CORNERS = 4;
            for(uint32_t i = 0; i < NUM_CORNERS; i++)
            {
                const glm::mat4 model = ruslice.vsinfo.spatial.model;
                const glm::vec4 xformed = model * ruslice.sliceCorners[i];
                bbox.expandBy(xformed);
            }
        }
        
        return bbox;
    }

    void RenderableUtils::volumeSliceDispose(const VolumeSliceID& volumeSliceID, const RScollectionID& collectionID)
    {
        if(volumeSliceID.isValid() && collectionID.isValid())
        {
            const RUvolumeSlice& ruvolslice = _volSliceMap[volumeSliceID];
            auto& vkrs = VkRenderSystem::getInstance();
            RenderableIDs ids = ruvolslice.sliceIDs;
            vkrs.appearanceDispose(ids.appID);
            vkrs.spatialDispose(ids.spatialID);
            vkrs.geometryDispose(ids.geomID);
            vkrs.geometryDataDispose(ids.geomDataID);
            vkrs.collectionInstanceDispose(collectionID, ids.instanceID);
        }
    }

    void RenderableUtils::dispose(RSlineIDs& lineIDs, RScollectionID& collectionID)
    {
        auto& vkrs = VkRenderSystem::getInstance();

        if (lineIDs.geomDataID.isValid())
        {
            vkrs.geometryDataDispose(lineIDs.geomDataID);
        }

        if (lineIDs.geomID.isValid())
        {
            vkrs.geometryDispose(lineIDs.geomID);
        }

        if (lineIDs.instanceID.isValid())
        {
            vkrs.collectionInstanceDispose(collectionID, lineIDs.instanceID);
        }

        if (lineIDs.appID.isValid())
        {
            vkrs.appearanceDispose(lineIDs.appID);
        }
    }

    GridID RenderableUtils::GridCreate(const GridInfo& gi, const RScollectionID& collectionID)
    {
        GridID outID;
        RUgrid rugrid;
        uint32_t id;
        bool success = _gridIDpool.CreateID(id);
        if (success)
        {
            outID.id = id;
            rugrid.gi = gi;
        }

        //create the geometry
        float halfsz = gi.size * 0.5f;
        std::vector<glm::vec4> positions =
        {
            { glm::vec4(-halfsz, 0.0f,  halfsz, 1.0f) },
            { glm::vec4( halfsz, 0.0f,  halfsz, 1.0f) },

            { glm::vec4( halfsz, 0.0f,  halfsz, 1.0f) },
            { glm::vec4( halfsz, 0.0f, -halfsz, 1.0f) },

            { glm::vec4( halfsz, 0.0f, -halfsz, 1.0f) },
            { glm::vec4(-halfsz, 0.0f, -halfsz, 1.0f) },

            { glm::vec4(-halfsz, 0.0f, -halfsz, 1.0f) },
            { glm::vec4(-halfsz, 0.0f,  halfsz, 1.0f) }
        };

        float spacing = gi.size / float(gi.resolution-1);
        //vertical lines
        for (uint32_t i = 1; i < gi.resolution; i++)
        {
            positions.push_back( glm::vec4(-halfsz + i * spacing, 0, -halfsz, 1.0f) );
            positions.push_back( glm::vec4(-halfsz + i * spacing, 0,  halfsz, 1.0f) );
        }
        
        //horizontal lines
        for (uint32_t i = 1; i < gi.resolution; i++)
        {
            positions.push_back( glm::vec4(-halfsz, 0,  -halfsz + i * spacing, 1.0f) );
            positions.push_back( glm::vec4( halfsz, 0,  -halfsz + i * spacing, 1.0f) );
        }

        std::vector<glm::vec4> colors(positions.size(), gi.color);

        auto& vkrs = VkRenderSystem::getInstance();
        RSvertexAttribsInfo attribsInfo;
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor };
        attribsInfo.attributes = attribs.data();
        attribsInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
        attribsInfo.settings = RSvertexAttributeSettings::vasSeparate;
        
        uint32_t numVertices = static_cast<uint32_t>(positions.size());
        vkrs.geometryDataCreate(rugrid.lineIDs.geomDataID, numVertices, 0, attribsInfo);
        uint32_t szinbytes = numVertices * sizeof(positions[0]);
        vkrs.geometryDataUpdateVertices(rugrid.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaPosition, positions.data());
        vkrs.geometryDataUpdateVertices(rugrid.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaColor, colors.data());
        vkrs.geometryDataFinalize(rugrid.lineIDs.geomDataID);

        RSgeometryInfo geomInfo;
        geomInfo.primType = RSprimitiveType::ptLine;
        vkrs.geometryCreate(rugrid.lineIDs.geomID, geomInfo);

        //create the instance and add to collection
        RSappearanceInfo appInfo;
        appInfo.shaderTemplate = RSshaderTemplate::stLines;
        vkrs.appearanceCreate(rugrid.lineIDs.appID, appInfo);

        RSinstanceInfo instInfo;
        instInfo.appID = rugrid.lineIDs.appID;
        instInfo.gdataID = rugrid.lineIDs.geomDataID;
        instInfo.geomID = rugrid.lineIDs.geomID;

        vkrs.collectionInstanceCreate(collectionID, rugrid.lineIDs.instanceID, instInfo);

        _gridMap[outID] = rugrid;

        return outID;
    }

    void RenderableUtils::GridDispose(const GridID& gridID, RScollectionID& collectionID)
    {
        if (gridID.isValid())
        {
            auto& vkrs = VkRenderSystem::getInstance();
            RUgrid& rugrid = _gridMap[gridID];
            dispose(rugrid.lineIDs, collectionID);

            _gridMap.erase(gridID);
        }
    }

    TriadID RenderableUtils::triadCreate(const TriadInfo& ti, const RScollectionID collectionID)
    {
        uint32_t id;
        RUtriad rutriad;
        TriadID outID;
        bool success = _triadIDpool.CreateID(id);
        if (success)
        {
            outID.id = id;
            rutriad.tinfo = ti;
        }
        
        glm::vec4 axis10 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 axis11 = glm::vec4(rutriad.tinfo.axis1 * rutriad.tinfo.size, 1.0f);
        
        glm::vec4 axis20 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 axis21 = glm::vec4(rutriad.tinfo.axis2 * rutriad.tinfo.size, 1.0f);
        
        glm::vec4 axis30 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 axis31 = glm::vec4(rutriad.tinfo.axis3 * rutriad.tinfo.size, 1.0f);
        
        
        std::vector<glm::vec4> positions =
        {
            axis10,
            axis11,
            axis20,
            axis21,
            axis30,
            axis31
        };

        std::vector<glm::vec4> colors =
        {
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),

            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),

            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
        };

        auto& vkrs = VkRenderSystem::getInstance();
        uint32_t numVertices = static_cast<uint32_t>(positions.size());
        RSvertexAttribsInfo attribInfo;
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor };
        attribInfo.attributes = attribs.data();
        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());

        vkrs.geometryDataCreate(rutriad.lineIDs.geomDataID, numVertices, 0, attribInfo);
        uint32_t szinbytes = static_cast<uint32_t>(positions.size() * sizeof(positions[0]));
        vkrs.geometryDataUpdateVertices(rutriad.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaPosition, positions.data());
        vkrs.geometryDataUpdateVertices(rutriad.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaColor, colors.data());
        vkrs.geometryDataFinalize(rutriad.lineIDs.geomDataID);

        RSgeometryInfo geomInfo;
        geomInfo.primType = RSprimitiveType::ptLine;
        vkrs.geometryCreate(rutriad.lineIDs.geomID, geomInfo);

        RSappearanceInfo appInfo;
        appInfo.shaderTemplate = RSshaderTemplate::stLines;
        vkrs.appearanceCreate(rutriad.lineIDs.appID, appInfo);

        vkrs.spatialCreate(rutriad.lineIDs.spatialID, ti.spatial);

        RSstate state;
        state.depthState.depthFunc = RSdepthFunction::dsAlway;
        state.lnstate.lineWidth = 3.0f;
        vkrs.stateCreate(rutriad.lineIDs.stateID, state);
        
        RSinstanceInfo instInfo;
        instInfo.appID = rutriad.lineIDs.appID;
        instInfo.gdataID = rutriad.lineIDs.geomDataID;
        instInfo.geomID = rutriad.lineIDs.geomID;
        instInfo.spatialID = rutriad.lineIDs.spatialID;
        instInfo.stateID = rutriad.lineIDs.stateID;

        vkrs.collectionInstanceCreate(collectionID, rutriad.lineIDs.instanceID, instInfo);

        _triadMap[outID] = rutriad;

        return outID;
    }

    void RenderableUtils::triadUpdate(const TriadID& triadID, const RSspatial& spatial)
    {
        assert(triadID.isValid() && "invalid triad ID");
        if(triadID.isValid() && _triadMap.find(triadID) != _triadMap.end())
        {
            
            _triadMap[triadID].tinfo.spatial = spatial;
            auto& vkrs = VkRenderSystem::getInstance();
            vkrs.spatialSetData(_triadMap.at(triadID).lineIDs.spatialID, spatial);
        }
    }

    void RenderableUtils::triadDispose(const TriadID& triadID, RScollectionID& collectionID)
    {
        if (triadID.isValid() && _triadMap.find(triadID) != _triadMap.end())
        {
            RUtriad& rutriad = _triadMap[triadID];
            dispose(rutriad.lineIDs, collectionID);

            _triadMap.erase(triadID);
        }
    }

    BoundsID  RenderableUtils::boundsCreate(const BoundsInfo& bi, const RScollectionID& collectionID)
    {
        uint32_t id;
        RUbounds rubounds;
        BoundsID outID;
        bool success = _boundsIDpool.CreateID(id);
        if (success)
        {
            outID.id = id;
            rubounds.boundsInfo = bi;
        }
        
        glm::vec4 minpt = bi.bbox.getmin();
        glm::vec4 maxpt = bi.bbox.getmax();
        std::vector<glm::vec4> positions =
        {
            minpt,
            glm::vec4(maxpt.x, minpt.y, minpt.z, 1.0f),
            
            minpt,
            glm::vec4(minpt.x, maxpt.y, minpt.z, 1.0f),
            
            minpt,
            glm::vec4(minpt.x, minpt.y, maxpt.z, 1.0f),
            
            glm::vec4(minpt.x, maxpt.y, maxpt.z, 1.0f),
            maxpt,
            
            glm::vec4(maxpt.x, minpt.y, maxpt.z, 1.0f),
            maxpt,
            
            glm::vec4(maxpt.x, maxpt.y, minpt.z, 1.0f),
            maxpt
        };

        std::vector<glm::vec4> colors =
        {
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),

            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),

            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
            
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
        };

        auto& vkrs = VkRenderSystem::getInstance();
        uint32_t numVertices = static_cast<uint32_t>(positions.size());
        RSvertexAttribsInfo attribInfo;
        std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor };
        attribInfo.attributes = attribs.data();
        attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
        attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());

        vkrs.geometryDataCreate(rubounds.lineIDs.geomDataID, numVertices, 0, attribInfo);
        uint32_t szinbytes = static_cast<uint32_t>(positions.size() * sizeof(positions[0]));
        vkrs.geometryDataUpdateVertices(rubounds.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaPosition, positions.data());
        vkrs.geometryDataUpdateVertices(rubounds.lineIDs.geomDataID, 0, szinbytes, RSvertexAttribute::vaColor, colors.data());
        vkrs.geometryDataFinalize(rubounds.lineIDs.geomDataID);

        RSgeometryInfo geomInfo;
        geomInfo.primType = RSprimitiveType::ptLine;
        vkrs.geometryCreate(rubounds.lineIDs.geomID, geomInfo);

        RSappearanceInfo appInfo;
        appInfo.shaderTemplate = RSshaderTemplate::stLines;
        vkrs.appearanceCreate(rubounds.lineIDs.appID, appInfo);

        RSspatial spatial;
        spatial.model = rubounds.boundsInfo.xform;
        spatial.modelInv = glm::inverse(spatial.model);
        vkrs.spatialCreate(rubounds.lineIDs.spatialID, spatial);

        RSstate state;
        state.depthState.depthFunc = RSdepthFunction::dsAlway;
        state.lnstate.lineWidth = 3.0f;
        vkrs.stateCreate(rubounds.lineIDs.stateID, state);
        
        RSinstanceInfo instInfo;
        instInfo.appID = rubounds.lineIDs.appID;
        instInfo.gdataID = rubounds.lineIDs.geomDataID;
        instInfo.geomID = rubounds.lineIDs.geomID;
        instInfo.spatialID = rubounds.lineIDs.spatialID;
        instInfo.stateID = rubounds.lineIDs.stateID;

        vkrs.collectionInstanceCreate(collectionID, rubounds.lineIDs.instanceID, instInfo);

        _boundsMap[outID] = rubounds;

        return outID;
    }

    void  RenderableUtils::boundsDispose(const BoundsID& boundsID, RScollectionID& collectionID)
    {
        if (boundsID.isValid())
        {
            RUbounds& rubounds = _boundsMap[boundsID];
            dispose(rubounds.lineIDs, collectionID);

            _boundsMap.erase(boundsID);
        }
    }

    //AssetModelID RenderableUtils::modelCreate(const unsigned char* inMemoryModel, uint32_t memcount, uint32_t uniqueID, const RScollectionID& collectionID, const std::string& dbgname)
    //{
    //    uint32_t id;
    //    RUassetModel rumodel;
    //    AssetModelID outID;
    //    bool success = _assetModelIDpool.CreateID(id);
    //    if (success)
    //    {
    //        outID.id = id;
    //        RUassetModel ruam;
    //        ruam.meshDataMap = GLTFmodelLoader::loadModelGeometryFromMemory(inMemoryModel, memcount, uniqueID);
    //        ruam.modelData.collectionID = collectionID;
    //        ruam.modelData.modelName = dbgname;
    //        GLTFmodelLoader::loadModelInstance(uniqueID, ruam.meshDataMap, ruam.modelData);
    //        
    //        _assetModelMap[outID] = ruam;
    //    }
    //    
    //    return outID;
    //}

    //BoundingBox RenderableUtils::modelGetBounds(const AssetModelID& modelID)
    //{
    //    BoundingBox bbox;
    //    if(_assetModelMap.find(modelID) != _assetModelMap.end())
    //    {
    //        RUassetModel& ruam = _assetModelMap[modelID];
    //        bbox = ruam.bbox;
    //    }
    //    
    //    return bbox;
    //}

    //std::vector<RSinstanceID> RenderableUtils::modelGetInstances(const AssetModelID& modelID)
    //{
    //    assert(modelID.isValid() && "invalid asset model ID");
    //    std::vector<RSinstanceID> instanceList;
    //    
    //    if(modelID.isValid() && _assetModelMap.find(modelID) != _assetModelMap.end())
    //    {
    //        RUassetModel& ruam = _assetModelMap[modelID];
    //        for(const MeshInstance& mi : ruam.modelData.meshInstances)
    //        {
    //            instanceList.push_back(mi.instanceID);
    //        }
    //    }
    //    
    //    return instanceList;
    //}

    //void RenderableUtils::modelUpdate(const AssetModelID& modelID, const RSspatial& spatial)
    //{
    //    if(_assetModelMap.find(modelID) != _assetModelMap.end())
    //    {
    //        auto& vkrs = VkRenderSystem::getInstance();
    //        RUassetModel& ruam = _assetModelMap[modelID];
    //        for(auto& meshInst : ruam.modelData.meshInstances)
    //        {
    //            vkrs.spatialSetData(meshInst.spatialID, spatial);
    //        }
    //    }
    //}

    //void RenderableUtils::modelDispose(const AssetModelID& modelID, const RScollectionID& collectionID)
    //{
    //    assert(modelID.isValid() && "invalid model ID");
    //    assert(collectionID.isValid() && "invalid collection ID");
    //    if(modelID.isValid() && _assetModelMap.find(modelID) != _assetModelMap.end() && collectionID.isValid())
    //    {
    //        RUassetModel& ruam = _assetModelMap[modelID];
    //        for(auto& iter : ruam.meshDataMap)
    //        {
    //            for(auto& iter1 : iter.second)
    //            {
    //                iter1.dispose();
    //            }
    //        }
    //        ruam.meshDataMap.clear();
    //        
    //        //Dispose mesh instances
    //        uint32_t numMeshInsts = static_cast<uint32_t>(ruam.modelData.meshInstances.size());
    //        for(MeshInstance& mi : ruam.modelData.meshInstances)
    //        {
    //            mi.dispose();
    //        }
    //        
    //        if(ruam.modelData.collectionID.isValid())
    //        {
    //            auto& vkrs = VkRenderSystem::getInstance();
    //            vkrs.collectionDispose(ruam.modelData.collectionID);
    //        }
    //    }
    //}
}
