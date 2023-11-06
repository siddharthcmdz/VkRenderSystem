#include "rsenums.h"
#include <string>

std::string getShaderStr(const RSshaderTemplate shaderTemplate) {
    std::string shaderStr;
    switch (shaderTemplate) {
        case RSshaderTemplate::stTextured:
            shaderStr = "textured";
            break;
            
        case RSshaderTemplate::stPassthrough:
            shaderStr = "passthrough";
            break;

        case RSshaderTemplate::stSimpleLit:
            shaderStr = "simplelit";
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

