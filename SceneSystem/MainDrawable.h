//// =================================================================================
// Copyright (c) 2023, See All Surgical Inc. All rights reserved.
//
// This software is the property of See All Surgical Inc. The software may not be reproduced,
// modified, distributed, or transferred without the express written permission of See All Surgical Inc.
//
// In no event shall See All Surgical Inc. be liable for any claim, damages or other liability,
// whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software
// or the use or other dealings in the software.
// =================================================================================

#pragma once

#include "WorldDrawable.h"
#include "RenderableUtils.h"
#include "ssenums.h"

namespace ss
{
    class MainDrawable final : public WorldDrawable
    {
    private:
        static constexpr uint32_t DEFAULT_MAX_INSTRUMENTS = 50;
        BoundingBox _bbox;
        RScollectionID _volumeSliceCollectionID;
        RScollectionID _instrumentsCollectionID;
        std::unordered_map<VolumeSliceOrientation, VolumeSliceData> _volumeSliceMap;
        //maps the intrumentID (aka trackable ID) to our asset model data.
        std::unordered_map<uint32_t, AssetModelData> _assetModelMap;
        AssetModelID _activeInstrumentID;
        uint32_t _maxInstruments = DEFAULT_MAX_INSTRUMENTS;
        bool _volumeSlicesConstrained = false;
        bool _volumeSlicesEnableMarker = false;
        glm::mat4 _toMetersScaleMat = glm::mat4(1.0f);
        glm::vec3 _toMetersScaleVec;
        VolumeOffset _currVolOffset;


        VolumeSliceData createVolumeSlice(const VolumeSliceOrientation vso, const VolumeModel& volumeModel, const RSvolumeSliceAppearance& vsapp);
        glm::mat4 constrainTranslation(glm::mat4 xform, const Axis& axis) const;
        void hideAllInstruments();
        
    public:
        MainDrawable() = default;
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
        
        void enableVolumeSliceTracker(bool onOff);
        void loadVolume(const ss::VolumeModel& vm, const RSvolumeSliceAppearance& volumeAppearance);
        void updateVolumeSliceAppearances(const RSvolumeSliceAppearance& volumeSliceAppearance);
        void updateInstrumentTransform(uint32_t instrumentID, const glm::mat4& trans, bool flipInstrumentVertical = false);
        VolumeOffset getCurrentVolumeOffset() const;
        uint32_t getAxialCount() const;
        uint32_t getSagittalCount() const;
        uint32_t getCoronalCount() const;
        void updateVolumeOffset(const VolumeOffset& voloff);
        void addInstrument(uint32_t intrumentID, const InMemoryModel& imm);
        VolumeSliceID getSliceID(const VolumeSliceOrientation& vso) const;
        RScollectionID getVolumeSliceCollectionID() const;
        void setActiveInstrument(uint32_t instrumentID);
        void resetViews();
        std::array<glm::vec3, 3> getVolumeSliceFrame(const VolumeSliceOrientation& vso) const;
    };

}
