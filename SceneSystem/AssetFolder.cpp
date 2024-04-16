
#include "AssetFolder.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBundle.h>

namespace ss {
    
    std::string AssetFolder::getFolder(std::string resourceName, std::string resourceExtension, std::string subDirName) {
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFStringEncoding encoding = kCFStringEncodingUTF8;

        CFStringRef resourceNameCFstringRef;
        resourceNameCFstringRef = CFStringCreateWithCString(kCFAllocatorDefault, resourceName.c_str(), encoding);

        CFStringRef resourceExtensionCFstringRef;
        resourceExtensionCFstringRef = CFStringCreateWithCString(kCFAllocatorDefault, resourceExtension.c_str(), encoding);
        
        CFStringRef subDirNameCFstringRef;
        subDirNameCFstringRef = CFStringCreateWithCString(kCFAllocatorDefault, subDirName.c_str(), encoding);
        
        CFURLRef ur = CFBundleCopyResourceURL(mainBundle, resourceNameCFstringRef, resourceExtensionCFstringRef, subDirNameCFstringRef);
        CFStringRef fragPath = CFURLCopyFileSystemPath(ur, kCFURLPOSIXPathStyle);
        
        //Now convert to cstring
        CFIndex length = CFStringGetLength(fragPath);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, encoding);
        
        std::string resourceFileStr;
        resourceFileStr.resize(maxSize+1);
        
        CFStringGetCString(fragPath, resourceFileStr.data(), maxSize, encoding);
        
        size_t loc = resourceFileStr.find_last_of("/");
        std::string resourcePathStr = resourceFileStr.substr(0, loc+1);
        
        CFRelease(fragPath);
        CFRelease(ur);
        CFRelease(resourceNameCFstringRef);
        CFRelease(resourceExtensionCFstringRef);
        CFRelease(subDirNameCFstringRef);
        
        ur = NULL;
        
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
