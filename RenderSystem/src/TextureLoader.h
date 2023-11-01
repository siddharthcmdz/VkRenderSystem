#pragma once

#include <cstdint>
#include "rsenums.h"

struct RStextureInfo {
	void* texels = nullptr;
	RStextureFormat texelFormat = RStextureFormat::tfRGBA8;
	RStextureType textureType = RStextureType::ttTexture2D;
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t depth = 1;
	uint32_t numChannels = 4;

	void dispose();
};

class TextureLoader {
public:
	static RStextureInfo readTexture(const char* filepath);
	static RStextureInfo readFromMemory(unsigned char* encodedTexData, uint32_t width, uint32_t height);
};