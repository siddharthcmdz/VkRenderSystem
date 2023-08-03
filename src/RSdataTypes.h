#pragma once
#include <cstdint>
#include <glm/vec4.hpp>
#include <vector>
#include "RStypes.h"
#include "rsenums.h"

#define MAX_IDS UINT32_MAX



struct RSinitInfo {
	bool enableValidation = true;
	bool onScreenCanvas = true;
	char appName[256]; //name of the engine or application
};

struct RScontextInfo {
	uint32_t width = 800;
	uint32_t height = 600;
	char title[256]{0}; //name of the window displayed as title
};

struct RSview {
	glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
	CameraType cameraType = CameraType::ORBITAL;
	std::vector<uint32_t> collectionList;
	bool dirty = true;
};

struct RScollectionInfo {

};

struct RSvertexAttribsInfo {
	uint32_t numVertexAttribs = 0;
	RSvertexAttribute* attributes = nullptr;

	uint32_t sizeOfAttrib() const;
};