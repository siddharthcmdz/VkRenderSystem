//
//#pragma once
//#include "AbstractWorldDrawable.h"
//#include "ModelData.h"
//#include <unordered_map>
//#include <vector>
//
//namespace ss 
//{
//    /**
//     * @brief A world drawable that is capable of loading a gltf model from storage, transforms data to rendersystem construct and renders it..
//     */
//    class GLTFmodelDrawable final : public AbstractWorldDrawable
//    {
//    private:
//        BoundingBox _bbox;
//        ModelData _modelData;
//        MeshDataMap _meshDataMap;
//        std::string _modelPath;
//
//        bool initGeometry();
//        bool initView();
//        bool disposeView();
//        bool disposeGeometry();
//
//    public:
//        GLTFmodelDrawable() = default;
//        
//        bool init() override;
//        bool dispose() override;
//        std::vector<RScollectionID> getCollections() const override;
//        BoundingBox getBounds() override;
//        std::string getName() const override;
//        DrawableType getType() const override;
//    };
//
//}
//
