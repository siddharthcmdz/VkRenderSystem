
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
