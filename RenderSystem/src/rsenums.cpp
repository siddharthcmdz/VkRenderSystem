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
            
        default:
            break;
    }
    
    return shaderStr;
    
}

