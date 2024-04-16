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
#include "BoundingBox.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "QuadricDataFactory.h"
#include <array>
#include "ModelData.h"

namespace ss 
{
    
    /**
     * @brief A drawable to render and stress test large amount of data across one or more views.
     */
    class BenchmarkDrawable final : public AbstractWorldDrawable
    {
    private:
        enum BenchmarkRenderableType 
        {
            brtSphereQuadric,
            brtCylinderQuadric,
            brtDiskQuadric,
            brtConeQuadric,
            brtQuad,
            brtModel,
            brtMAX
        };
        
        BoundingBox _bbox;
        std::array<MeshData, 6> _meshDataArray;
        std::string _modelPath;
        uint32_t _numMeshPerAxis = 10;
        ModelData _modelData;
        
        void createRenderable(BenchmarkRenderableType brt);
        MeshData createQuadric(const QuadricData& qdata);
        bool initGeometry();
        bool initView();
        bool disposeView();
        bool disposeGeometry();

    public:
        BenchmarkDrawable() = default;
        bool init() override;
        bool dispose() override;
        std::vector<RScollectionID> getCollections() const override;
        BoundingBox getBounds() override;
        std::string getName() const override;
        DrawableType getType() const override;
    };

}
