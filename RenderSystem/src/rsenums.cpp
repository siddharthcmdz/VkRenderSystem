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
            
        default:
            break;
    }
    
    return shaderStr;
    
}

