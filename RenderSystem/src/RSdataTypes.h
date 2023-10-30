#pragma once
#include <cstdint>
#include <glm/vec4.hpp>
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

#include "rsexporter.h"

struct RSinitInfo
{
	bool enableValidation = true;
	bool onScreenCanvas = true;
	char appName[256]; // name of the engine or application
	char shaderPath[256];
	// These two are needed for querying queue families early on - rendersysteminit(). Its always the case, the window is first constructed and then vkrs. The parent window can serve to construct the logical device which can be reused to present to the presentation queue.
#if defined(_WIN32)
	HWND parentHwnd{};
	HINSTANCE parentHinst{};
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	void *parentView;
#endif
};

struct RScontextInfo
{
	uint32_t initWidth = 800;
	uint32_t initHeight = 600;
	char title[256]{0}; // name of the window displayed as title
#if defined(_WIN32)
	HWND hwnd{};
	HINSTANCE hinst{};
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	void *metallayer;
#endif
};

struct RSview
{
	glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
	CameraType cameraType = CameraType::ORBITAL;
	glm::mat4 viewmat{};
	glm::mat4 projmat{};
	glm::vec4 lightPos;
	bool dirty = true;
};

struct RScollectionInfo
{
	uint32_t maxInstances = 0;
};

struct RSvertexAttribsInfo
{
	uint32_t numVertexAttribs = 0;
	RSvertexAttribute *attributes = nullptr;
	RSvertexAttributeSettings settings;

	RS_EXPORT uint32_t sizeOfInterleavedAttrib() const;
	RS_EXPORT uint32_t sizeOfAttrib(RSvertexAttribute attrib) const;
	RS_EXPORT std::string getName(RSvertexAttribute attrib) const;
	RS_EXPORT uint32_t getBindingPoint(RSvertexAttribute attrib) const;
};

struct RSgeometryInfo
{
	RSprimitiveType primType = RSprimitiveType::ptTriangleStrip;
	// uint32_t offset = 0;
	// uint32_t numElements = 0;
};

struct RSinstanceInfo
{
	RSgeometryDataID gdataID;
	RSgeometryID geomID;
	RSspatialID spatialID;
	RSappearanceID appID;
	RSstateID stateID;
};

struct RSappearanceInfo
{
	RStextureID diffuseTexture;
	RSshaderTemplate shaderTemplate;
};

struct RSspatial
{
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 modelInv = glm::mat4(1.0f);
};

struct RSpointState
{
	float pointSize = 1.0f;
};

struct RSlineState
{
	float lineWidth = 1.0f;
};

struct RSstate
{
	RSpointState ptstate;
	RSlineState lnstate;
};