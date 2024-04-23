
#pragma once
#include "AbstractWorldDrawable.h"
#include "ModelData.h"

namespace ss 
{

    class QuadricDrawable final : public AbstractWorldDrawable
    {
    private:
        BoundingBox _bbox;
        GeometryInfo _geomInfo;
        RScollectionID _collectionID;
        InstanceData _instData;

        bool _isInited = false;

        void createQuadrics();
        bool initGeometry();
        bool initView();
        bool disposeView();
        bool disposeGeometry();

    public:
        QuadricDrawable() = default;
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
    };
}
