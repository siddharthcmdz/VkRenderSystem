#pragma once
#include <cstdint>
#include <glm/vec4.hpp>
#include <vector>
#include "RStypes.h"

#define MAX_IDS UINT32_MAX
#define INVALID_ID ~0

struct RSview {
	glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
	CameraType cameraType = CameraType::ORBITAL;
	bool dirty = true;
};

struct RSviewID {
	uint32_t id = ~0;
	bool isValid() const;
};

struct RScontextID {
	uint32_t id = ~0;
	bool isValid() const;
};

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