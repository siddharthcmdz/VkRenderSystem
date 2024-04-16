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

#include "AbstractWorldDrawable.h"
#include "rsids.h"
#include "unordered_map"
#include "ModelData.h"
#include "QuadricDataFactory.h"

namespace ss 
{
    class Gizmo2dDrawable final : public AbstractWorldDrawable
    {
    private:
        enum IconType 
        {
            itMove,
            itRotateCW,
            itRotateCCW,
            itInvalid
        };
        std::unordered_map<IconType, RStextureID> _textureMap;
        InstanceData _instData;
        RScollectionID _collectionID;
        BoundingBox _bbox;
        
        void createGeometry(const QuadricData& qd);
        bool initGeometry();
        bool initView();
        bool disposeView();
        bool disposeGeometry();

    public:
        
        Gizmo2dDrawable() = default;
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
    };
}
