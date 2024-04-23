
#pragma once

#include "AbstractWorldDrawable.h"
#include "BoundingBox.h"
#include <string>
#include <vector>
#include "ModelData.h"

namespace ss 
{

    class MultiQuadricDrawable final : public AbstractWorldDrawable
    {
    private:
        BoundingBox _bbox;
        std::vector<GeometryInfo> _geomInfos;
        std::vector<InstanceData> _instances;
        RScollectionID _collectionID;

        void createQuadrics();
        bool initGeometry();
        bool initView();
        bool disposeView();
        bool disposeGeometry();

    public:
        MultiQuadricDrawable() = default;
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
    };

}

