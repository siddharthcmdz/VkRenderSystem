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

#include "rsenums.h"
#include <string>

std::string getShaderStr(const RSshaderTemplate shaderTemplate) {
    std::string shaderStr;
    switch (shaderTemplate) {
        case RSshaderTemplate::stOneTriangle:
            shaderStr = "onetriangle";
            break;
            
        case RSshaderTemplate::stPassthrough:
            shaderStr = "passthrough";
            break;
            
        case RSshaderTemplate::stSimpleLit:
            shaderStr = "simpleLit";
            break;
            
        case RSshaderTemplate::stSimpleTextured:
            shaderStr = "simpleTextured";
            break;
            
        case RSshaderTemplate::stVolumeSlice:
            shaderStr = "volumeslice";
            break;
            
        case RSshaderTemplate::stLines:
            shaderStr = "lines";
            break;
            
        default:
            break;
    }
    
    return shaderStr;
    
}

