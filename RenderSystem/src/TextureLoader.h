#pragma once

#include <cstdint>

struct RStextureInfo {
	unsigned char* texels = nullptr;
	int texWidth = 0;
	int texHeight = 0;
	int texChannels = 4;

	void dispose();
};

class TextureLoader {
public:
	static RStextureInfo readTexture(const char* filepath);
};