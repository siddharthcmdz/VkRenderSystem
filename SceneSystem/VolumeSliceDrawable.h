
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


