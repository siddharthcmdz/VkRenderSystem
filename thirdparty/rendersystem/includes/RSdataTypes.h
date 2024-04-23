#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "RStypes.h"
#include "rsenums.h"
#include "rsids.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

#define MAX_IDS UINT32_MAX
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#if defined(_WIN32)
#include <Windows.h>
#endif

struct RSinitInfo 
{
    bool enableValidation = true;
    bool onScreenCanvas = true;
    char appName[256]; //name of the engine or application
    char shaderPath[256];
#if defined(_WIN32)
    HWND parentHwnd{};
    HINSTANCE parentHinst{};
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    void* parentView;
#endif
};

struct RScontextInfo 
{
    uint32_t initWidth = 800;
    uint32_t initHeight = 600;
    char title[256]{0}; //name of the window displayed as title
#if defined(_WIN32)
    HWND hwnd{};
    HINSTANCE hinst{};
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    void* metallayer;
#endif
};

struct RSview 
{
    glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
    CameraType cameraType = CameraType::ORBITAL;
    glm::mat4 viewmat{};
    glm::mat4 projmat{};
    glm::vec4 lightPos{};
    bool dirty = true;
    std::string name;
};

struct RSvertexAttribsInfo 
{
    uint32_t numVertexAttribs = 0;
    RSvertexAttribute* attributes = nullptr;
    RSvertexAttributeSettings settings;
    uint32_t sizeOfInterleavedAttrib() const;
    uint32_t sizeOfAttrib(RSvertexAttribute attrib) const;
    std::string getName(RSvertexAttribute attrib) const;
    uint32_t getBindingPoint(RSvertexAttribute attrib) const;
};

struct RScollectionInfo 
{
    uint32_t maxInstances = 0;
    std::string collectionName = "Unnamed";
    RScollectionHint hint = RScollectionHint::chInvalid;
};

struct RSinstanceInfo 
{
    RSgeometryDataID gdataID;
    RSgeometryID geomID;
    RSspatialID spatialID;
    RSappearanceID appID;
    RSstateID stateID;
    std::string name;
};

struct RSvolumeSliceAppearance
{
    glm::vec2 window{};
    glm::vec2 rescale{};
};

struct RSappearanceInfo 
{
    RStextureID diffuseTexture;
    RSshaderTemplate shaderTemplate;
    RSvolumeSliceAppearance volumeSlice;
};

struct RSspatial 
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 modelInv = glm::mat4(1.0f);
    glm::mat4 texture = glm::mat4(1.0f);
};

struct RSgeometryInfo 
{
    RSprimitiveType primType = RSprimitiveType::ptTriangleStrip;
    //uint32_t offset = 0;
    //uint32_t numElements = 0;
};

struct RSpointState 
{
    float pointSize = 1.0f;
};

struct RSlineState 
{
    float lineWidth = 1.0f;
};


struct RSdepthState
{
    RSdepthFunction depthFunc = RSdepthFunction::dsLess;
};

struct RSstate 
{
    RSpointState ptstate;
    RSlineState lnstate;
    RSdepthState depthState;
    //TODO: add more states as when needed - blend state, depth state etc
};

/**
 * @brief Stores offset and count of voxels for a sub volume aka volume chunk.
 */
struct RSvolumeChunk
{
    int32_t xoffset = 0, yoffset = 0, zoffset = 0;
    uint32_t xcount = 0, ycount = 0, zcount = 0;
};
