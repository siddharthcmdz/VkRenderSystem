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
#include <unordered_map>
#include "AbstractWorldDrawable.h"
#include "ModelData.h"
#include "rsids.h"
#include "rsenums.h"
#include "RenderableUtils.h"
#include "ssenums.h"
#include "SSdataTypes.h"

namespace ss 
{

    /**
    * @brief A drawable for rendering a volume slice.
    */
    class VolumeSliceDrawable final : public AbstractWorldDrawable
    {
    private:
        
        BoundingBox _bbox;
        float _volumeSize = 128;
        RScollectionID _collectionID;
        std::unordered_map<VolumeSliceOrientation, VolumeSliceData> _volumeSliceMap;
        
        VolumeSliceData createVolumeSlice(const VolumeSliceOrientation vso, const VolumeModel& volumeModel, const RScollectionID& collectionID);
        
    public:
        VolumeSliceDrawable() = default;
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
    };

}


