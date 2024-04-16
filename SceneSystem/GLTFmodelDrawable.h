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
#include "ModelData.h"
#include <unordered_map>
#include <vector>

namespace ss 
{
    /**
     * @brief A world drawable that is capable of loading a gltf model from storage, transforms data to rendersystem construct and renders it..
     */
    class GLTFmodelDrawable final : public AbstractWorldDrawable
    {
    private:
        BoundingBox _bbox;
        ModelData _modelData;
        MeshDataMap _meshDataMap;
        std::string _modelPath;

        bool initGeometry();
        bool initView();
        bool disposeView();
        bool disposeGeometry();

    public:
        GLTFmodelDrawable() = default;
        
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
    };

}

