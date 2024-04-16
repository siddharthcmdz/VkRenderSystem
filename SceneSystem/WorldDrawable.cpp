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

#include "WorldDrawable.h"
#include <string>


namespace ss {
    std::string getDrawableName(const DrawableType dt) {
        std::string drawableStr;
        switch(dt) {
                
            case DrawableType::dtQuadrics:
                drawableStr = "SingleQuadric";
                break;
                
            case DrawableType::dtMultiQuadrics:
                drawableStr = "MultiQuadrics";
                break;
                
            case DrawableType::dtGLTFmodel:
                drawableStr = "GLTFmodel";
                break;
                
            case DrawableType::dtBenchmark:
                drawableStr = "Benchmark";
                break;
                
            case DrawableType::dtGizmo2d:
                drawableStr = "Gizmo2d";
                break;
                
            case DrawableType::dtVolumeSlice:
                drawableStr = "VolumeSlice";
                break;
                
            case DrawableType::dtMain:
                drawableStr = "Main";
                break;
                
            default:
                drawableStr = "UNKNOWN";
        }
        
        return drawableStr;
    }
}
