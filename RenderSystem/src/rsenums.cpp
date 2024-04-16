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

