#include "TextureLoader.h"
#include "stb_image.h"

void RStextureInfo::dispose() {
	stbi_image_free(this->texels);
}

RStextureInfo TextureLoader::readTexture(const char* filepath) {
	RStextureInfo ti;
	int widthi = static_cast<int>(ti.width);
	int heighti = static_cast<int>(ti.height);
	int numchannelsi = static_cast<int>(ti.numChannels);
	ti.texels = stbi_load(filepath, &widthi, &heighti, &numchannelsi, STBI_rgb_alpha);

	return ti;
}

RStextureInfo TextureLoader::readFromMemory(unsigned char* texdata, uint32_t width, uint32_t height) {
	RStextureInfo ti;
	int widthi = static_cast<int>(ti.width);
	int heighti = static_cast<int>(ti.height);
	int numchannelsi = static_cast<int>(ti.numChannels);

	if (height == 0)
	{
		//ti.texels = stbi_load_from_memory(texdata, width, &ti.texWidth, &ti.texHeight, &ti.texChannels, 0);
		ti.texels = stbi_load_from_memory(texdata, width, &widthi, &heighti, &numchannelsi, STBI_rgb_alpha);
	}
	else
	{
		ti.texels = stbi_load_from_memory(texdata, width * height, &widthi, &heighti, &numchannelsi, STBI_rgb_alpha);
	}

	return ti;
}	