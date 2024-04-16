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
#include <string>

namespace ss {

    /**
    * @brief Provides functionality to read folder path of specified resource from ios bundle/
    */
    class AssetFolder final {
    private:
        static std::string getFolder(std::string resourceName, std::string resourceExtension, std::string subDirName);
        
    public:
        /**
         * @brief Gets the absolute path of the folder containing .glb files
         * @return the c-string containing the path of the model asset.
         */
        static std::string getModelFolder();
        
        /**
         * @brief Gets the absolute path of the folder containing texture files.
         * @return the c-string containing the path of the texture asset.
         */
        static std::string getTextureFolder();
        
        /**
         * @brief Gets the absolute path of the folder containing shader files.
         * @return the c-string containing the path of shader assets.
         */
        static std::string getShaderFolder();
    };
}
