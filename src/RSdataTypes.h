#pragma once
#include <cstdint>
#include <glm/vec4.hpp>
#include "RStypes.h"

#define MAX_IDS UINT32_MAX
#define INVALID_ID ~0

struct RSview {
	uint32_t width = 0;
	uint32_t height = 0;
	glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
	CameraType cameraType = CameraType::ORBITAL;
	bool dirty = true;
};

struct RSviewID {
	uint32_t id = ~0;
	bool isValid() const;
};

