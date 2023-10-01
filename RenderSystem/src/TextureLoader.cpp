#include "TextureLoader.h"
#include "stb_image.h"

void RStextureInfo::dispose() {
	stbi_image_free(this->texels);
}

RStextureInfo TextureLoader::readTexture(const char* filepath) {
	RStextureInfo ti;
	ti.texels = stbi_load(filepath, &ti.texWidth, &ti.texHeight, &ti.texChannels, STBI_rgb_alpha);

	return ti;
}

RStextureInfo TextureLoader::readFromMemory(unsigned char* texdata, uint32_t width, uint32_t height) {
	RStextureInfo ti;
	if (height == 0)
	{
		//ti.texels = stbi_load_from_memory(texdata, width, &ti.texWidth, &ti.texHeight, &ti.texChannels, 0);
		ti.texels = stbi_load_from_memory(texdata, width, &ti.texWidth, &ti.texHeight, &ti.texChannels, STBI_rgb_alpha);
	}
	else
	{
		ti.texels = stbi_load_from_memory(texdata, width * height, &ti.texWidth, &ti.texHeight, &ti.texChannels, STBI_rgb_alpha);
	}

	return ti;
}	