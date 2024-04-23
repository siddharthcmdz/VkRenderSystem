//#pragma once
//
//#include "AbstractWorldDrawable.h"
//#include "rsids.h"
//#include "unordered_map"
//#include "ModelData.h"
//#include "QuadricDataFactory.h"
//
//namespace ss 
//{
//    class Gizmo2dDrawable final : public AbstractWorldDrawable
//    {
//    private:
//        enum IconType 
//        {
//            itMove,
//            itRotateCW,
//            itRotateCCW,
//            itInvalid
//        };
//        std::unordered_map<IconType, RStextureID> _textureMap;
//        InstanceData _instData;
//        RScollectionID _collectionID;
//        BoundingBox _bbox;
//        
//        void createGeometry(const QuadricData& qd);
//        bool initGeometry();
//        bool initView();
//        bool disposeView();
//        bool disposeGeometry();
//
//    public:
//        
//        Gizmo2dDrawable() = default;
//        bool init() override;
//        bool dispose() override;
//        std::vector<RScollectionID> getCollections() const override;
//        BoundingBox getBounds() override;
//        std::string getName() const override;
//        DrawableType getType() const override;
//    };
//}
