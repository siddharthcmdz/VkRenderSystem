
#include "AssetFolder.h"

namespace ss {
    
    std::string AssetFolder::getFolder(std::string resourceName, std::string resourceExtension, std::string subDirName) {
        std::string resourcePathStr;
        return resourcePathStr;
    }

    std::string AssetFolder::getModelFolder() {
        std::string modelPathStub = getFolder("101-0008", "glb", "models");
        return modelPathStub;
    }

    std::string AssetFolder::getTextureFolder() {
        std::string texturePathStub = getFolder("gizmo_button_ move_left", ".png", "textures");
        return texturePathStub;
    }

    std::string AssetFolder::getShaderFolder() {
        std::string shaderPathStub = getFolder("onetriangle_frag", "spv", "shaders");
        return shaderPathStub;
    }
}
